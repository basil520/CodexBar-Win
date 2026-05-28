[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$TargetDir,

    [ValidateSet("x64", "x86", "arm64")]
    [string]$Architecture = "x64"
)

$ErrorActionPreference = "Stop"

function Add-CandidateBase {
    param(
        [string]$Path
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return
    }

    try {
        $resolvedPath = (Resolve-Path -LiteralPath $Path -ErrorAction Stop).Path
    } catch {
        return
    }

    if ($script:candidateBases -notcontains $resolvedPath) {
        $script:candidateBases += $resolvedPath
    }
}

function Add-VisualStudioRedistBases {
    param(
        [string]$VisualStudioRoot
    )

    if ([string]::IsNullOrWhiteSpace($VisualStudioRoot) -or -not (Test-Path -LiteralPath $VisualStudioRoot -PathType Container)) {
        return
    }

    Get-ChildItem -LiteralPath $VisualStudioRoot -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        $majorVersionDir = $_
        Get-ChildItem -LiteralPath $majorVersionDir.FullName -Directory -ErrorAction SilentlyContinue | ForEach-Object {
            Add-CandidateBase -Path (Join-Path $_.FullName "VC\Redist\MSVC")
        }
    }
}

function Get-VersionFromCrtDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [System.IO.DirectoryInfo]$Directory
    )

    try {
        return [version]$Directory.Parent.Parent.Name
    } catch {
        return [version]"0.0"
    }
}

$resolvedTargetDir = (Resolve-Path -LiteralPath $TargetDir).Path
$script:candidateBases = @()

if (-not [string]::IsNullOrWhiteSpace($env:VCToolsRedistDir)) {
    Add-CandidateBase -Path $env:VCToolsRedistDir
}

if (-not [string]::IsNullOrWhiteSpace($env:VCINSTALLDIR)) {
    Add-CandidateBase -Path (Join-Path $env:VCINSTALLDIR "Redist\MSVC")
}

$vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path -LiteralPath $vswherePath -PathType Leaf) {
    $installationPaths = & $vswherePath -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    foreach ($installationPath in $installationPaths) {
        Add-CandidateBase -Path (Join-Path $installationPath "VC\Redist\MSVC")
    }
}

Add-VisualStudioRedistBases -VisualStudioRoot (Join-Path $env:ProgramFiles "Microsoft Visual Studio")
Add-VisualStudioRedistBases -VisualStudioRoot (Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio")

$crtDirectories = @()
foreach ($candidateBase in $script:candidateBases) {
    $versionDirectories = @()

    if (Test-Path -LiteralPath (Join-Path $candidateBase $Architecture) -PathType Container) {
        $versionDirectories += Get-Item -LiteralPath $candidateBase
    }

    $versionDirectories += Get-ChildItem -LiteralPath $candidateBase -Directory -ErrorAction SilentlyContinue |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName $Architecture) -PathType Container }

    foreach ($versionDirectory in $versionDirectories) {
        $archDir = Join-Path $versionDirectory.FullName $Architecture
        $crtDirectories += Get-ChildItem -LiteralPath $archDir -Directory -Filter "Microsoft.VC*.CRT" -ErrorAction SilentlyContinue
    }
}

$crtDirectory = $crtDirectories |
    Sort-Object @{ Expression = { Get-VersionFromCrtDirectory -Directory $_ }; Descending = $true }, @{ Expression = { $_.Name }; Descending = $true } |
    Select-Object -First 1

if ($null -eq $crtDirectory) {
    throw "Could not find Microsoft VC runtime redist directory for architecture '$Architecture'. Install Visual Studio C++ tools or set VCToolsRedistDir."
}

$runtimeDlls = Get-ChildItem -LiteralPath $crtDirectory.FullName -Filter "*.dll" -File
if ($runtimeDlls.Count -eq 0) {
    throw "Microsoft VC runtime redist directory '$($crtDirectory.FullName)' does not contain DLL files."
}

foreach ($redistInstaller in Get-ChildItem -LiteralPath $resolvedTargetDir -Filter "vc_redist*.exe" -File -ErrorAction SilentlyContinue) {
    Remove-Item -LiteralPath $redistInstaller.FullName -Force
}

foreach ($runtimeDll in $runtimeDlls) {
    Copy-Item -LiteralPath $runtimeDll.FullName -Destination (Join-Path $resolvedTargetDir $runtimeDll.Name) -Force
}

foreach ($requiredRuntimeDll in @("msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")) {
    if (-not (Test-Path -LiteralPath (Join-Path $resolvedTargetDir $requiredRuntimeDll) -PathType Leaf)) {
        throw "Required VC runtime DLL '$requiredRuntimeDll' was not copied to '$resolvedTargetDir'."
    }
}

Write-Host "Copied app-local VC runtime from '$($crtDirectory.FullName)' to '$resolvedTargetDir'."

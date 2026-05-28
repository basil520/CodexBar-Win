[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$TargetDir
)

$ErrorActionPreference = "Stop"

$resolvedTargetDir = (Resolve-Path -LiteralPath $TargetDir).Path

$prunedFiles = @(
    "opengl32sw.dll",
    "D3Dcompiler_47.dll",
    "Qt63DExtras.dll",
    "Qt6Quick3DUtils.dll"
)

$prunedDirectories = @(
    "qmltooling",
    "generic",
    "sceneparsers",
    "geometryloaders",
    "renderplugins"
)

foreach ($fileName in $prunedFiles) {
    $filePath = Join-Path $resolvedTargetDir $fileName
    if (Test-Path -LiteralPath $filePath -PathType Leaf) {
        Remove-Item -LiteralPath $filePath -Force
        Write-Host "Removed release payload file: $fileName"
    }
}

Get-ChildItem -LiteralPath $resolvedTargetDir -Filter "vc_redist*.exe" -File -ErrorAction SilentlyContinue |
    ForEach-Object {
        Remove-Item -LiteralPath $_.FullName -Force
        Write-Host "Removed VC runtime installer: $($_.Name)"
    }

foreach ($directoryName in $prunedDirectories) {
    $directoryPath = Join-Path $resolvedTargetDir $directoryName
    if (Test-Path -LiteralPath $directoryPath -PathType Container) {
        Remove-Item -LiteralPath $directoryPath -Recurse -Force
        Write-Host "Removed release payload directory: $directoryName"
    }
}

$translationsPath = Join-Path $resolvedTargetDir "translations"
if (Test-Path -LiteralPath $translationsPath -PathType Container) {
    Get-ChildItem -LiteralPath $translationsPath -Filter "*.qm" -File |
        Where-Object { $_.Name -notmatch '_(en|zh_CN)\.qm$' } |
        ForEach-Object {
            Remove-Item -LiteralPath $_.FullName -Force
            Write-Host "Removed translation: $($_.Name)"
        }
}

Get-ChildItem -LiteralPath $resolvedTargetDir -Filter "*.pdb" -Recurse -File -ErrorAction SilentlyContinue |
    ForEach-Object {
        Remove-Item -LiteralPath $_.FullName -Force
        Write-Host "Removed debug symbol: $($_.FullName)"
    }

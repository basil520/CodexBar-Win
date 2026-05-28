[CmdletBinding()]
param(
    [string]$ConfigPath,

    [string]$PayloadPath
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ConfigPath)) {
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $ConfigPath = Join-Path $projectRoot "installer/config/config.xml"
}

function Assert-InstallerValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$Actual,

        [Parameter(Mandatory = $true)]
        [string]$Expected
    )

    if ($Actual -ne $Expected) {
        throw "Installer.$Name expected '$Expected' but found '$Actual'."
    }
}

function Get-InstallerValue {
    param(
        [Parameter(Mandatory = $true)]
        [xml]$Xml,

        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $value = [string]$Xml.Installer.$Name
    if ([string]::IsNullOrWhiteSpace($value)) {
        throw "Missing required Installer.$Name node."
    }

    return $value.Trim()
}

function Assert-FileContains {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [string]$Content,

        [Parameter(Mandatory = $true)]
        [string]$Needle,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not $Content.Contains($Needle)) {
        throw "$Name must contain $Description."
    }
}

function Assert-PathPresent {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Installer payload must include $Description."
    }
}

function Assert-PathAbsent {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (Test-Path -LiteralPath $Path) {
        throw "Installer payload must not include $Description."
    }
}

function ConvertFrom-CodePointList {
    param(
        [Parameter(Mandatory = $true)]
        [int[]]$CodePoints
    )

    return -join ($CodePoints | ForEach-Object { [char]$_ })
}

$resolvedConfigPath = Resolve-Path -LiteralPath $ConfigPath
$configDir = Split-Path -Parent $resolvedConfigPath
[xml]$config = Get-Content -LiteralPath $resolvedConfigPath -Raw

$expectedValues = [ordered]@{
    Title                            = "CodexBarX"
    StyleSheet                       = "stylesheet.qss"
    ControlScript                    = "controller.qs"
    Watermark                        = "watermark.png"
    WizardShowPageList               = "false"
    WizardStyle                      = "Classic"
    TitleColor                       = "#ffffff"
    InstallerWindowIcon              = "icon"
    InstallerApplicationIcon         = "icon"
    Logo                             = "logo.png"
    Banner                           = "banner.png"
}

foreach ($entry in $expectedValues.GetEnumerator()) {
    $actual = Get-InstallerValue -Xml $config -Name $entry.Key
    Assert-InstallerValue -Name $entry.Key -Actual $actual -Expected $entry.Value
}

foreach ($resourceNode in @("StyleSheet", "ControlScript", "Watermark", "Logo", "Banner")) {
    $relativePath = Get-InstallerValue -Xml $config -Name $resourceNode
    $resourcePath = Join-Path $configDir $relativePath

    if (-not (Test-Path -LiteralPath $resourcePath -PathType Leaf)) {
        throw "Installer.$resourceNode references missing file '$relativePath'."
    }
}

foreach ($resource in @("icon.ico", "icon.png")) {
    $resourcePath = Join-Path $configDir $resource
    if (-not (Test-Path -LiteralPath $resourcePath -PathType Leaf)) {
        throw "Installer icon resource is missing '$resource'."
    }
}

$unsupportedGlobalNodes = @(
    "DisableLicenseAutoAcceptCheckBox"
)

foreach ($nodeName in $unsupportedGlobalNodes) {
    if ($config.Installer.SelectSingleNode($nodeName)) {
        throw "Installer.$nodeName is not supported by Qt IFW 4.11 config.xml."
    }
}

$controlScriptPath = Join-Path $configDir (Get-InstallerValue -Xml $config -Name "ControlScript")
$controlScriptContent = Get-Content -LiteralPath $controlScriptPath -Raw -Encoding UTF8
if ($controlScriptContent -match "setAutoAcceptLicenses") {
    throw "ControlScript must not call setAutoAcceptLicenses; license acceptance must remain user-controlled."
}

$styleSheetPath = Join-Path $configDir (Get-InstallerValue -Xml $config -Name "StyleSheet")
$styleSheetContent = Get-Content -LiteralPath $styleSheetPath -Raw -Encoding UTF8

$packageScriptPath = Join-Path (Split-Path -Parent $configDir) "packages/com.codexbarx.app/meta/installscript.qs"
if (-not (Test-Path -LiteralPath $packageScriptPath -PathType Leaf)) {
    throw "Package install script is missing 'packages/com.codexbarx.app/meta/installscript.qs'."
}

$packageScriptContent = Get-Content -LiteralPath $packageScriptPath -Raw -Encoding UTF8

Assert-FileContains -Name "StyleSheet" -Content $styleSheetContent -Needle "CodexBarXInstallerGlassTheme" -Description "the installer glass theme marker"
Assert-FileContains -Name "StyleSheet" -Content $styleSheetContent -Needle "#49A3B0" -Description "the CodexBarX teal accent"
Assert-FileContains -Name "StyleSheet" -Content $styleSheetContent -Needle "QPushButton:default" -Description "primary button styling"
Assert-FileContains -Name "StyleSheet" -Content $styleSheetContent -Needle "QProgressBar::chunk" -Description "installer progress styling"
Assert-FileContains -Name "StyleSheet" -Content $styleSheetContent -Needle "QScrollBar::handle:vertical:hover" -Description "low-distraction scrollbar hover styling"

Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle "var installerTheme" -Description "centralized installer theme tokens"
Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle ((ConvertFrom-CodePointList @(0x5B89, 0x88C5)) + " CodexBarX") -Description "Simplified Chinese welcome copy"
Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle (ConvertFrom-CodePointList @(0x8F7B, 0x91CF, 0x6258, 0x76D8)) -Description "tray-focused product copy"
Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle (ConvertFrom-CodePointList @(0x4E0A, 0x4E00, 0x6B65)) -Description "Simplified Chinese Back button text"
Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle (ConvertFrom-CodePointList @(0x4E0B, 0x4E00, 0x6B65)) -Description "Simplified Chinese Next button text"
Assert-FileContains -Name "ControlScript" -Content $controlScriptContent -Needle (ConvertFrom-CodePointList @(0x5B8C, 0x6210)) -Description "Simplified Chinese Finish button text"

if ($packageScriptContent -match 'addElevatedOperation\(\s*"Settings"') {
    throw "Package install script must not use the Qt IFW Settings operation for Windows uninstall registry entries."
}

Assert-FileContains -Name "PackageInstallScript" -Content $packageScriptContent -Needle 'function addRegistrySetOperation' -Description "a registry set helper"
Assert-FileContains -Name "PackageInstallScript" -Content $packageScriptContent -Needle 'reg.exe' -Description "native Windows registry command execution"
Assert-FileContains -Name "PackageInstallScript" -Content $packageScriptContent -Needle 'UNDOEXECUTE' -Description "registry rollback operations"

if (-not [string]::IsNullOrWhiteSpace($PayloadPath)) {
    $resolvedPayloadPath = Resolve-Path -LiteralPath $PayloadPath

    foreach ($fileName in @("opengl32sw.dll", "D3Dcompiler_47.dll", "Qt63DExtras.dll", "Qt6Quick3DUtils.dll")) {
        Assert-PathAbsent -Path (Join-Path $resolvedPayloadPath $fileName) -Description "pruned Qt deployment file '$fileName'"
    }

    foreach ($directoryName in @("qmltooling", "generic", "sceneparsers", "geometryloaders", "renderplugins")) {
        Assert-PathAbsent -Path (Join-Path $resolvedPayloadPath $directoryName) -Description "pruned Qt plugin directory '$directoryName'"
    }

    foreach ($redistInstaller in Get-ChildItem -LiteralPath $resolvedPayloadPath -Filter "vc_redist*.exe" -File) {
        throw "Installer payload must not include bundled VC runtime installer '$($redistInstaller.Name)'. Use app-local VC runtime DLLs instead."
    }

    foreach ($runtimeDll in @("msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")) {
        Assert-PathPresent -Path (Join-Path $resolvedPayloadPath $runtimeDll) -Description "app-local VC runtime DLL '$runtimeDll'"
    }

    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "sqldrivers/qsqlite.dll") -Description "the SQLite driver"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "tls") -Description "TLS plugins"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "imageformats") -Description "image format plugins"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "platforms/qwindows.dll") -Description "the Windows platform plugin"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "qml/QtQuick/Controls/Basic/qmldir") -Description "Qt Quick Controls Basic QML"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "translations/CodexBarX_en.qm") -Description "CodexBarX English translation"
    Assert-PathPresent -Path (Join-Path $resolvedPayloadPath "translations/CodexBarX_zh_CN.qm") -Description "CodexBarX Simplified Chinese translation"

    $translationsPath = Join-Path $resolvedPayloadPath "translations"
    if (Test-Path -LiteralPath $translationsPath -PathType Container) {
        foreach ($translationFile in Get-ChildItem -LiteralPath $translationsPath -Filter "*.qm" -File) {
            if ($translationFile.Name -notmatch '_(en|zh_CN)\.qm$') {
                throw "Installer payload includes unsupported translation '$($translationFile.Name)'. Only en and zh_CN translations should be packaged."
            }
        }
    }
}

Write-Host "Installer config validation passed."

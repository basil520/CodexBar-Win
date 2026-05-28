import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

SettingsGroupBox {
    id: root
    property bool compact: false

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle {
                width: AppTheme.statusDotSize
                height: AppTheme.statusDotSize
                radius: width / 2
                color: BridgeViewModel.connectedClients.length > 0
                    ? AppTheme.statusOk
                    : (BridgeViewModel.extensionInstalled ? AppTheme.statusUnknown : AppTheme.accentColor)
            }

            Label {
                Layout.fillWidth: true
                text: BridgeViewModel.connectedClients.length > 0
                    ? qsTr("Browser Session Bridge connected")
                    : (BridgeViewModel.extensionInstalled
                        ? qsTr("Extension prepared, waiting for browser")
                        : qsTr("Prepare optional browser extension"))
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
            }
        }

        Label {
            Layout.fillWidth: true
            visible: !root.compact
            text: qsTr("Optional enhanced import: load the unpacked extension in Chrome, Edge, or Brave when native cookie import needs a fallback or a provider requires live localStorage.")
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            text: BridgeViewModel.extensionInstallPath
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            elide: Text.ElideMiddle
        }

        Label {
            Layout.fillWidth: true
            visible: BridgeViewModel.lastError.length > 0
            text: BridgeViewModel.lastError
            color: AppTheme.statusOutage
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            SettingsButton {
                text: BridgeViewModel.extensionPreparing ? qsTr("Preparing...") : qsTr("Prepare Optional Extension")
                primary: !BridgeViewModel.extensionInstalled
                enabled: !BridgeViewModel.extensionPreparing
                onClicked: BridgeViewModel.prepareExtension()
            }

            SettingsButton {
                text: qsTr("Copy Path")
                onClicked: BridgeViewModel.copyExtensionPath()
            }

            SettingsButton {
                text: qsTr("Open Folder")
                onClicked: BridgeViewModel.openExtensionFolder()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: !root.compact

            SettingsButton {
                text: qsTr("Open Chrome Extensions")
                onClicked: BridgeViewModel.openChromeExtensionsPage()
            }

            SettingsButton {
                text: qsTr("Open Edge Extensions")
                onClicked: BridgeViewModel.openEdgeExtensionsPage()
            }
        }
    }
}

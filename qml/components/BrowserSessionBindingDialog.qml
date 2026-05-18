import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

Dialog {
    id: root
    modal: true
    title: qsTr("Choose Browser Profile")
    standardButtons: Dialog.Close

    property string providerId: ""
    property string selectedBindingId: ""
    property var options: []

    signal bindingSelected(string bindingId)

    width: Math.min(parent ? parent.width - 48 : 520, 520)

    background: Rectangle {
        radius: AppTheme.radiusMd
        color: AppTheme.surfacePopup
        border.width: 1
        border.color: AppTheme.surfaceBorder
    }

    contentItem: ColumnLayout {
        spacing: 10

        Label {
            Layout.fillWidth: true
            text: qsTr("Select a connected Chrome, Edge, or Brave profile. Auto uses the first connected profile that supports this provider.")
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        SettingsButton {
            Layout.fillWidth: true
            text: qsTr("Auto")
            primary: root.selectedBindingId === ""
            onClicked: {
                root.bindingSelected("")
                root.close()
            }
        }

        Repeater {
            model: root.options
            delegate: SettingsButton {
                Layout.fillWidth: true
                text: {
                    var suffix = modelData.connected ? qsTr(" - connected") : qsTr(" - offline")
                    return (modelData.label || modelData.bindingId) + suffix
                }
                primary: root.selectedBindingId === modelData.bindingId
                enabled: modelData.connected
                onClicked: {
                    root.bindingSelected(modelData.bindingId)
                    root.close()
                }
            }
        }
    }
}

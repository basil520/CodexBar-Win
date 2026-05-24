import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SurfaceCard {
    id: root

    property string providerId: ""
    property string providerName: ""
    property string state: "idle"
    property string summary: ""
    property string actionText: ""

    signal actionRequested()

    implicitHeight: providerId === "" ? 0 : 64
    visible: providerId !== ""
    radius: AppTheme.radiusMd

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: AppTheme.spacingMd

        Components.ProviderIdentityBadge {
            Layout.preferredWidth: AppTheme.avatarSizeDock
            Layout.preferredHeight: AppTheme.avatarSizeDock
            size: AppTheme.avatarSizeDock
            providerId: root.providerId
            displayName: root.providerName
            selected: true
            severity: root.state === "error" ? "error" : (root.state === "warning" ? "warning" : "none")
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: root.providerName !== "" ? root.providerName : root.providerId
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: root.summary
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }
        }

        Components.ActionButton {
            visible: root.actionText !== ""
            text: root.actionText
            compact: true
            onClicked: root.actionRequested()
        }
    }
}

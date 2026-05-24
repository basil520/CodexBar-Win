import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SurfaceCard {
    id: root

    property string title: qsTr("Nothing to show yet")
    property string reason: ""
    property string nextStep: ""
    property string actionText: ""

    signal actionRequested()

    implicitHeight: Math.max(92, content.implicitHeight + 28)

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: AppTheme.spacingLg
        spacing: AppTheme.spacingSm

        Label {
            Layout.fillWidth: true
            text: root.title
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            text: root.reason
            visible: text !== ""
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            text: root.nextStep
            visible: text !== ""
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Components.ActionButton {
            Layout.alignment: Qt.AlignHCenter
            visible: root.actionText !== ""
            text: root.actionText
            variant: "secondary"
            onClicked: root.actionRequested()
        }
    }
}

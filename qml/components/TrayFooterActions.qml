import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

SelectiveRadiusRect {
    id: root

    property bool glassEffectActive: false
    property bool refreshing: false
    property string refreshDuration: ""

    signal refreshRequested()
    signal settingsRequested()
    signal aboutRequested()
    signal quitRequested()

    implicitHeight: 52
    fillColor: glassEffectActive ? "transparent" : AppTheme.surfaceTitleBar
    topLeftRadius: 0
    topRightRadius: 0
    bottomLeftRadius: glassEffectActive ? 0 : 12
    bottomRightRadius: glassEffectActive ? 0 : 12

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: AppTheme.surfaceBorder
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        ActionButton {
            text: root.refreshing
                ? qsTr("Refreshing...") + (root.refreshDuration ? " " + root.refreshDuration : "")
                : qsTr("Refresh")
            compact: true
            enabled: !root.refreshing
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            onClicked: root.refreshRequested()
        }

        ActionButton {
            text: qsTr("Settings")
            compact: true
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            onClicked: root.settingsRequested()
        }

        ActionButton {
            text: qsTr("About")
            compact: true
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            onClicked: root.aboutRequested()
        }

        ActionButton {
            text: qsTr("Quit")
            compact: true
            variant: "danger"
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            onClicked: root.quitRequested()
        }
    }
}

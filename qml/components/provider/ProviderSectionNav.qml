import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."

Rectangle {
    id: root
    objectName: "providerSectionNav"

    property var sections: []
    property string activeSection: "overview"

    signal sectionRequested(string section)

    Layout.fillWidth: true
    implicitHeight: 38
    radius: AppTheme.radiusMd
    color: AppTheme.surfacePane
    border.width: 1
    border.color: AppTheme.borderSubtle
    clip: true

    Flickable {
        id: flicker
        anchors.fill: parent
        anchors.margins: 5
        contentWidth: sectionRow.implicitWidth
        contentHeight: sectionRow.implicitHeight
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        Row {
            id: sectionRow
            spacing: 6
            height: flicker.height

            Repeater {
                model: root.sections

                delegate: Rectangle {
                    id: pill

                    readonly property string sectionId: modelData && modelData.id ? modelData.id : ""
                    readonly property bool selected: sectionId === root.activeSection

                    width: Math.max(64, label.implicitWidth + 22)
                    height: 28
                    anchors.verticalCenter: parent.verticalCenter
                    radius: 14
                    color: selected ? AppTheme.surfaceSelected
                        : (hoverHandler.hovered ? AppTheme.surfaceHover : "transparent")
                    border.width: selected ? 1 : 0
                    border.color: AppTheme.borderFocus
                    activeFocusOnTab: true
                    Accessible.role: Accessible.Button
                    Accessible.name: label.text

                    function activate() {
                        if (sectionId !== "") root.sectionRequested(sectionId)
                    }

                    Keys.onReturnPressed: function(event) { event.accepted = true; activate() }
                    Keys.onEnterPressed: function(event) { event.accepted = true; activate() }
                    Keys.onSpacePressed: function(event) { event.accepted = true; activate() }

                    Behavior on color {
                        ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
                    }

                    Label {
                        id: label
                        anchors.centerIn: parent
                        text: modelData && modelData.label ? modelData.label : pill.sectionId
                        color: pill.selected ? AppTheme.textPrimary : AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        font.bold: pill.selected
                        elide: Text.ElideRight
                    }

                    HoverHandler {
                        id: hoverHandler
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {
                        onTapped: pill.activate()
                    }
                }
            }
        }
    }
}

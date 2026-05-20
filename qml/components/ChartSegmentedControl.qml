import QtQuick 2.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    property var segments: []
    property int selectedIndex: 0
    signal indexChanged(int index)

    height: 32
    color: AppTheme.surfaceTrack || "#1c1c28"
    radius: 6
    border.width: 1
    border.color: AppTheme.surfaceBorder || "#2d2d3d"

    // Sliding background indicator
    Rectangle {
        id: slider
        y: 2
        height: parent.height - 4
        radius: 4
        color: AppTheme.surfaceCard || "#2d2d3e"
        border.width: 1
        border.color: AppTheme.surfaceBorder || "#3e3e52"

        // Dynamic x and width animation based on selected index
        width: segments.length > 0 ? (root.width - 4) / segments.length : 0
        x: 2 + selectedIndex * width

        Behavior on x {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutCubic
            }
        }
    }

    Row {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: root.segments

            delegate: Item {
                width: root.width / root.segments.length
                height: root.height

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: index === root.selectedIndex ? AppTheme.textPrimary : AppTheme.textSecondary
                    font.pixelSize: 11
                    font.bold: index === root.selectedIndex
                    
                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.selectedIndex = index
                        root.indexChanged(index)
                    }
                }
            }
        }
    }
}

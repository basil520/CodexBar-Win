import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

SelectiveRadiusRect {
    id: root

    property int providerCount: 0
    property bool glassEffectActive: false

    signal moveRequested(real deltaX, real deltaY)

    implicitHeight: 48
    fillColor: glassEffectActive ? "transparent" : AppTheme.surfaceTitleBar
    topLeftRadius: glassEffectActive ? 8 : 12
    topRightRadius: glassEffectActive ? 8 : 12
    bottomLeftRadius: 0
    bottomRightRadius: 0

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: AppTheme.surfaceBorder
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 8

        Text {
            text: qsTr("CodexBar")
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
        }

        Item { Layout.fillWidth: true }

        Text {
            text: root.providerCount + " " + qsTr("providers")
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeXs
        }
    }

    MouseArea {
        id: headerDragArea
        anchors.fill: parent
        cursorShape: Qt.SizeAllCursor
        property real pressX: 0
        property real pressY: 0

        onPressed: {
            pressX = mouseX
            pressY = mouseY
        }

        onPositionChanged: root.moveRequested(mouseX - pressX, mouseY - pressY)
    }
}

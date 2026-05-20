import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

Rectangle {
    id: root

    property string symbol: "copy"
    property string accessibleName: ""
    property color iconColor: danger ? AppTheme.statusOutage : AppTheme.textSecondary
    property bool danger: false
    property bool checked: false
    property string tooltip: accessibleName

    signal activated()

    implicitWidth: 28
    implicitHeight: 28
    width: implicitWidth
    height: implicitHeight
    radius: AppTheme.radiusSm
    color: !enabled ? "transparent"
        : mouse.pressed ? AppTheme.surfacePressed
        : mouse.containsMouse || checked ? AppTheme.surfaceHover
        : "transparent"
    border.width: activeFocus ? 1 : 0
    border.color: AppTheme.surfaceAccentBorder
    opacity: enabled ? 1.0 : 0.45
    clip: true

    Behavior on color { ColorAnimation { duration: 90; easing.type: Easing.OutQuad } }
    Behavior on opacity { NumberAnimation { duration: 90; easing.type: Easing.OutQuad } }

    Canvas {
        id: iconCanvas
        anchors.centerIn: parent
        width: 16
        height: 16

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = root.iconColor
            ctx.fillStyle = root.iconColor
            ctx.lineWidth = 1.7
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            if (root.symbol === "check") {
                ctx.beginPath()
                ctx.moveTo(3, 8.5)
                ctx.lineTo(6.5, 12)
                ctx.lineTo(13, 4)
                ctx.stroke()
            } else if (root.symbol === "warning") {
                ctx.beginPath()
                ctx.moveTo(8, 2.5)
                ctx.lineTo(14, 13)
                ctx.lineTo(2, 13)
                ctx.closePath()
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(8, 6)
                ctx.lineTo(8, 9)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(8, 11.5, 0.8, 0, 2 * Math.PI)
                ctx.fill()
            } else if (root.symbol === "info") {
                ctx.beginPath()
                ctx.arc(8, 8, 6, 0, 2 * Math.PI)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(8, 7)
                ctx.lineTo(8, 11)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(8, 4.8, 0.8, 0, 2 * Math.PI)
                ctx.fill()
            } else {
                ctx.strokeRect(5, 3, 8, 10)
                ctx.strokeRect(3, 5, 8, 10)
            }
        }

        Connections {
            target: root
            function onSymbolChanged() { iconCanvas.requestPaint() }
            function onIconColorChanged() { iconCanvas.requestPaint() }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.activated()
    }

    ToolTip.visible: mouse.containsMouse && root.tooltip !== ""
    ToolTip.delay: 450
    ToolTip.text: root.tooltip
}

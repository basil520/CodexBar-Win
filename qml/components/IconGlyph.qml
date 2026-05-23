import QtQuick 2.15
import ".."

Canvas {
    id: root

    property string glyphName: ""
    property color strokeColor: AppTheme.textPrimary
    property color fillColor: strokeColor
    property real strokeWidth: 1.6

    antialiasing: true

    onGlyphNameChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onFillColorChanged: requestPaint()
    onStrokeWidthChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = strokeColor
        ctx.fillStyle = fillColor
        ctx.lineWidth = strokeWidth
        ctx.lineCap = "round"
        ctx.lineJoin = "round"

        var w = width
        var h = height
        var cx = w / 2
        var cy = h / 2

        ctx.beginPath()
        if (glyphName === "minimize") {
            ctx.moveTo(w * 0.22, h * 0.72)
            ctx.lineTo(w * 0.78, h * 0.72)
        } else if (glyphName === "close") {
            ctx.moveTo(w * 0.28, h * 0.28)
            ctx.lineTo(w * 0.72, h * 0.72)
            ctx.moveTo(w * 0.72, h * 0.28)
            ctx.lineTo(w * 0.28, h * 0.72)
        } else if (glyphName === "restore") {
            ctx.strokeRect(w * 0.34, h * 0.20, w * 0.42, h * 0.42)
            ctx.strokeRect(w * 0.20, h * 0.34, w * 0.42, h * 0.42)
        } else if (glyphName === "maximize") {
            ctx.strokeRect(w * 0.26, h * 0.26, w * 0.50, h * 0.50)
        } else if (glyphName === "chevronDown") {
            ctx.moveTo(w * 0.25, h * 0.38)
            ctx.lineTo(cx, h * 0.62)
            ctx.lineTo(w * 0.75, h * 0.38)
        } else if (glyphName === "chevronRight") {
            ctx.moveTo(w * 0.38, h * 0.25)
            ctx.lineTo(w * 0.62, cy)
            ctx.lineTo(w * 0.38, h * 0.75)
        } else if (glyphName === "refresh") {
            ctx.arc(cx, cy, Math.min(w, h) * 0.28, Math.PI * 0.15, Math.PI * 1.55)
            ctx.moveTo(w * 0.72, h * 0.22)
            ctx.lineTo(w * 0.76, h * 0.44)
            ctx.lineTo(w * 0.56, h * 0.38)
        } else if (glyphName === "copy") {
            ctx.strokeRect(w * 0.32, h * 0.22, w * 0.42, h * 0.46)
            ctx.strokeRect(w * 0.22, h * 0.34, w * 0.42, h * 0.46)
        } else if (glyphName === "check") {
            ctx.moveTo(w * 0.22, h * 0.52)
            ctx.lineTo(w * 0.42, h * 0.70)
            ctx.lineTo(w * 0.78, h * 0.30)
        } else if (glyphName === "warning") {
            ctx.moveTo(cx, h * 0.18)
            ctx.lineTo(w * 0.82, h * 0.78)
            ctx.lineTo(w * 0.18, h * 0.78)
            ctx.closePath()
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(cx, h * 0.38)
            ctx.lineTo(cx, h * 0.58)
            ctx.stroke()
            ctx.beginPath()
            ctx.arc(cx, h * 0.68, 0.8, 0, Math.PI * 2)
            ctx.fill()
            return
        } else {
            ctx.arc(cx, cy, Math.min(w, h) * 0.30, 0, Math.PI * 2)
        }
        ctx.stroke()
    }
}

import QtQuick 2.15

Canvas {
    id: root

    property color fillColor: "transparent"
    property int topLeftRadius: 0
    property int topRightRadius: 0
    property int bottomLeftRadius: 0
    property int bottomRightRadius: 0

    onFillColorChanged: requestPaint()
    onTopLeftRadiusChanged: requestPaint()
    onTopRightRadiusChanged: requestPaint()
    onBottomLeftRadiusChanged: requestPaint()
    onBottomRightRadiusChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        var tl = Math.max(0, topLeftRadius)
        var tr = Math.max(0, topRightRadius)
        var bl = Math.max(0, bottomLeftRadius)
        var br = Math.max(0, bottomRightRadius)
        var w = width
        var h = height

        ctx.beginPath()
        ctx.moveTo(tl, 0)
        ctx.lineTo(w - tr, 0)
        if (tr > 0) ctx.arcTo(w, 0, w, tr, tr)
        else ctx.lineTo(w, 0)
        ctx.lineTo(w, h - br)
        if (br > 0) ctx.arcTo(w, h, w - br, h, br)
        else ctx.lineTo(w, h)
        ctx.lineTo(bl, h)
        if (bl > 0) ctx.arcTo(0, h, 0, h - bl, bl)
        else ctx.lineTo(0, h)
        ctx.lineTo(0, tl)
        if (tl > 0) ctx.arcTo(0, 0, tl, 0, tl)
        else ctx.lineTo(0, 0)
        ctx.closePath()

        ctx.fillStyle = fillColor.toString()
        ctx.fill()
    }
}

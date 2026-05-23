import QtQuick 2.15

import CodexBarX 1.0

Canvas {
    id: sparkCanvas
    width: 90
    height: 32
    antialiasing: true

    property var dataPoints: [10, 15, 12, 24, 18, 30, 28] // Default series
    property color strokeColor: AppTheme.statusOk
    property bool hovered: false

    opacity: hovered ? 0.85 : 0.45
    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
    }

    onDataPointsChanged: requestPaint()
    onStrokeColorChanged: requestPaint()
    onHoveredChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d");
        ctx.reset();
        ctx.clearRect(0, 0, width, height);

        if (dataPoints.length < 2) return;

        var maxVal = Math.max.apply(null, dataPoints);
        var minVal = Math.min.apply(null, dataPoints);
        var range = maxVal - minVal || 1.0;

        var stepX = width / (dataPoints.length - 1);

        // Draw transparent gradient background
        ctx.beginPath();
        ctx.moveTo(0, height);
        for (var i = 0; i < dataPoints.length; i++) {
            var normY = (dataPoints[i] - minVal) / range;
            var plotX = i * stepX;
            var plotY = height - 2 - normY * (height - 6);
            ctx.lineTo(plotX, plotY);
        }
        ctx.lineTo(width, height);
        ctx.closePath();

        var fillGrad = ctx.createLinearGradient(0, 0, 0, height);
        fillGrad.addColorStop(0.0, AppTheme.withAlpha(strokeColor, 0.15));
        fillGrad.addColorStop(1.0, "transparent");
        ctx.fillStyle = fillGrad;
        ctx.fill();

        // Draw line
        ctx.beginPath();
        ctx.lineWidth = 1.2;
        ctx.strokeStyle = strokeColor;
        ctx.lineJoin = "round";
        ctx.lineCap = "round";

        for (var i = 0; i < dataPoints.length; i++) {
            var normY = (dataPoints[i] - minVal) / range;
            var plotX = i * stepX;
            var plotY = height - 2 - normY * (height - 6);
            
            if (i === 0) {
                ctx.moveTo(plotX, plotY);
            } else {
                ctx.lineTo(plotX, plotY);
            }
        }
        ctx.stroke();

        // If hovered, draw a micro pulse dot at the last point
        if (hovered && dataPoints.length > 0) {
            var lastNormY = (dataPoints[dataPoints.length - 1] - minVal) / range;
            var lastX = width;
            var lastY = height - 2 - lastNormY * (height - 6);
            
            ctx.beginPath();
            ctx.arc(lastX - 2, lastY, 2.5, 0, 2 * Math.PI);
            ctx.fillStyle = strokeColor;
            ctx.fill();

            ctx.beginPath();
            ctx.arc(lastX - 2, lastY, 4.5, 0, 2 * Math.PI);
            ctx.strokeStyle = AppTheme.withAlpha(strokeColor, 0.4);
            ctx.lineWidth = 1;
            ctx.stroke();
        }
    }
}

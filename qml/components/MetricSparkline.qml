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

        // 1. Calculate all trend coordinate points
        var points = [];
        for (var i = 0; i < dataPoints.length; i++) {
            var normY = (dataPoints[i] - minVal) / range;
            var plotX = i * stepX;
            var plotY = height - 2 - normY * (height - 6);
            points.push({x: plotX, y: plotY});
        }

        // 2. Pre-calculate Catmull-Rom spline tangent vectors for each point
        var tangents = [];
        var tension = 0.5; // Standard cardinal spline tension
        for (var i = 0; i < points.length; i++) {
            var tx = 0;
            var ty = 0;
            if (i === 0) {
                tx = tension * (points[1].x - points[0].x);
                ty = tension * (points[1].y - points[0].y);
            } else if (i === points.length - 1) {
                tx = tension * (points[points.length - 1].x - points[points.length - 2].x);
                ty = tension * (points[points.length - 1].y - points[points.length - 2].y);
            } else {
                tx = tension * (points[i + 1].x - points[i - 1].x);
                ty = tension * (points[i + 1].y - points[i - 1].y);
            }
            tangents.push({x: tx, y: ty});
        }

        // 3. Draw mathematically smooth transparent gradient area fill
        ctx.beginPath();
        ctx.moveTo(0, height);
        ctx.lineTo(points[0].x, points[0].y);
        for (var i = 0; i < points.length - 1; i++) {
            var A = points[i];
            var B = points[i + 1];
            var Ta = tangents[i];
            var Tb = tangents[i + 1];

            var cp1x = A.x + Ta.x / 3;
            var cp1y = A.y + Ta.y / 3;
            var cp2x = B.x - Tb.x / 3;
            var cp2y = B.y - Tb.y / 3;

            ctx.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, B.x, B.y);
        }
        ctx.lineTo(width, height);
        ctx.closePath();

        var fillGrad = ctx.createLinearGradient(0, 0, 0, height);
        fillGrad.addColorStop(0.0, AppTheme.withAlpha(strokeColor, 0.15));
        fillGrad.addColorStop(1.0, "transparent");
        ctx.fillStyle = fillGrad;
        ctx.fill();

        // 4. Draw mathematically smooth high-precision curve trend line
        ctx.beginPath();
        ctx.lineWidth = 1.25;
        ctx.strokeStyle = strokeColor;
        ctx.lineJoin = "round";
        ctx.lineCap = "round";

        ctx.moveTo(points[0].x, points[0].y);
        for (var i = 0; i < points.length - 1; i++) {
            var A = points[i];
            var B = points[i + 1];
            var Ta = tangents[i];
            var Tb = tangents[i + 1];

            var cp1x = A.x + Ta.x / 3;
            var cp1y = A.y + Ta.y / 3;
            var cp2x = B.x - Tb.x / 3;
            var cp2y = B.y - Tb.y / 3;

            ctx.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, B.x, B.y);
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

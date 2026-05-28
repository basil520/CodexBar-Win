import QtQuick 2.15
import CodexBarX 1.0

Item {
    id: root
    anchors.fill: parent
    clip: true

    property string providerId: (typeof TrayViewModel !== "undefined" && TrayViewModel) ? TrayViewModel.selectedProviderID : ""

    readonly property bool glassEffectActive: SettingsStore.glassEffectEnabled
    readonly property bool decorativeMotionEnabled: PerformanceState.decorativeEffectsActive && !AppTheme.reduceMotion
    readonly property real qualityOpacity: SettingsStore.visualEffectsQuality === "high" ? 1.0
        : SettingsStore.visualEffectsQuality === "low" ? 0.45
        : 0.75

    opacity: (glassEffectActive ? 1.0 : 0.4) * qualityOpacity
    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionPanel); easing.type: Easing.InOutQuad }
    }

    // Smooth continuous time clock driven by hardware-accelerated animation
    property real time: 0
    NumberAnimation on time {
        id: timeAnim
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        from: 0
        to: 10000
        duration: 1000000 // Very long duration to avoid wrapping glitches, smooth drift
    }

    // Brand-Aware Color Symphony
    property color targetColor1: (typeof TrayViewModel !== "undefined" && TrayViewModel) ? AppTheme.providerBrandColor(root.providerId) : AppTheme.accentColor
    property color targetColor2: (typeof TrayViewModel !== "undefined" && TrayViewModel) ? getSecondaryColor(root.providerId) : AppTheme.accentColor

    property color color1: targetColor1
    property color color2: targetColor2

    Behavior on color1 {
        ColorAnimation { duration: 1200; easing.type: Easing.InOutQuad }
    }
    Behavior on color2 {
        ColorAnimation { duration: 1200; easing.type: Easing.InOutQuad }
    }

    function getSecondaryColor(pId) {
        if (pId === "claude") return "#CC7C5E"; // Warm amber/orange
        if (pId === "deepseek") return "#8A3FFC"; // Purple
        if (pId === "openaiapi" || pId === "perplexity") return "#10B981"; // Emerald
        if (pId === "gemini") return "#FF7EB6"; // Pink-violet
        if (pId === "cursor") return "#00BFA5"; // Cyan-teal
        if (pId === "codex") return "#00BCD4"; // Cyan
        // Complementary fallback: shift hue or return a harmonized blue/teal
        return AppTheme.accentColor;
    }

    // Mouse Tracking Area - Transparent and non-blocking
    MouseArea {
        id: mouseTracker
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    // Magnetic Spring Tracker coordinates
    property real trackX: mouseTracker.containsMouse ? mouseTracker.mouseX : parent.width / 2
    property real trackY: mouseTracker.containsMouse ? mouseTracker.mouseY : parent.height / 2

    Behavior on trackX {
        id: trackXBehavior
        enabled: root.decorativeMotionEnabled
        SpringAnimation {
            spring: 1.5
            damping: 0.18
            epsilon: 0.25
        }
    }
    Behavior on trackY {
        id: trackYBehavior
        enabled: root.decorativeMotionEnabled
        SpringAnimation {
            spring: 1.5
            damping: 0.18
            epsilon: 0.25
        }
    }

    // Smooth transition weight for entering/leaving the window
    property real pullWeight: mouseTracker.containsMouse ? 1.0 : 0.0
    Behavior on pullWeight {
        NumberAnimation { duration: 600; easing.type: Easing.OutCubic }
    }

    onTimeChanged: {
        if (decorativeMotionEnabled && PerformanceState.anyUiVisible) {
            canvas.requestPaint();
        }
    }
    onTrackXChanged: if (PerformanceState.anyUiVisible) canvas.requestPaint()
    onTrackYChanged: if (PerformanceState.anyUiVisible) canvas.requestPaint()
    onPullWeightChanged: if (PerformanceState.anyUiVisible) canvas.requestPaint()

    Connections {
        target: PerformanceState
        function onAnyUiVisibleChanged() {
            if (PerformanceState.anyUiVisible) {
                canvas.requestPaint()
            }
        }
        function onDecorativeEffectsActiveChanged() {
            if (PerformanceState.anyUiVisible) {
                canvas.requestPaint()
            }
        }
    }

    // 1. Offscreen Noise Canvas (rendered once on startup)
    Canvas {
        id: noisePatternCanvas
        width: 128
        height: 128
        visible: false

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, 128, 128);
            var seed = 2463534242;
            for (var i = 0; i < 420; ++i) {
                seed = (seed * 1664525 + 1013904223) >>> 0;
                var x = seed % 128;
                seed = (seed * 1664525 + 1013904223) >>> 0;
                var y = seed % 128;
                seed = (seed * 1664525 + 1013904223) >>> 0;
                var alpha = 0.035 + ((seed & 31) / 31.0) * 0.085;
                ctx.fillStyle = "rgba(255,255,255," + alpha + ")";
                ctx.fillRect(x, y, 1, 1);
            }
        }

        Component.onCompleted: requestPaint()
    }

    // 2. Main Fluid Mesh Wave Field Canvas
    Canvas {
        id: canvas
        anchors.fill: parent
        visible: PerformanceState.anyUiVisible

        onWidthChanged: if (PerformanceState.anyUiVisible) requestPaint()
        onHeightChanged: if (PerformanceState.anyUiVisible) requestPaint()

        function drawRibbon(ctx, layer, startX, endX, step, rotMX, rotMY, hasMouse) {
            var t = root.time;
            var baseColor = layer === 1 ? root.color1 : (layer === 2 ? root.color2 : Qt.rgba(1, 1, 1, 0.45));
            
            var amp, freq, phase, speed, verticalOffset;
            if (layer === 1) {
                amp = canvas.height * 0.22;
                freq = 0.003;
                speed = 0.015;
                phase = 0;
                verticalOffset = -canvas.height * 0.08;
            } else if (layer === 2) {
                amp = canvas.height * 0.16;
                freq = 0.005;
                speed = -0.024;
                phase = Math.PI * 0.4;
                verticalOffset = canvas.height * 0.06;
            } else {
                amp = canvas.height * 0.08;
                freq = 0.012;
                speed = 0.038;
                phase = Math.PI * 0.85;
                verticalOffset = canvas.height * 0.18;
            }

            function getWaveY(x) {
                var w = Math.sin(x * freq + t * speed + phase) * amp;
                w += Math.cos(x * freq * 0.4 - t * speed * 0.7 + phase * 1.5) * (amp * 0.35);
                w += Math.sin(x * freq * 1.8 + t * speed * 1.4) * (amp * 0.12);

                var distX = x - rotMX;
                var sigma = 140;
                var factor = Math.exp(-(distX * distX) / (2 * sigma * sigma)) * root.pullWeight;
                var ripple = Math.sin(t * 0.08) * 12 * factor;

                return (w + verticalOffset) * (1 - factor) + (rotMY + ripple) * factor;
            }

            var runs = (layer < 3) ? 2 : 1;
            for (var run = 0; run < runs; ++run) {
                ctx.save();
                
                var color;
                var xOffset = 0;
                if (layer < 3) {
                    if (run === 0) {
                        xOffset = -1.8;
                        color = Qt.rgba(baseColor.r, baseColor.g * 0.25, baseColor.b * 1.1, layer === 1 ? 0.075 : 0.065);
                    } else {
                        xOffset = 1.8;
                        color = Qt.rgba(baseColor.r * 0.25, baseColor.g * 1.1, baseColor.b * 1.1, layer === 1 ? 0.075 : 0.065);
                    }
                } else {
                    color = Qt.rgba(1, 1, 1, 0.095);
                }

                ctx.translate(xOffset, 0);

                var grad = ctx.createLinearGradient(0, -canvas.height, 0, canvas.height);
                grad.addColorStop(0, "transparent");
                grad.addColorStop(0.35, color);
                grad.addColorStop(0.65, color);
                grad.addColorStop(1, "transparent");

                ctx.fillStyle = grad;
                ctx.beginPath();
                
                var firstY = getWaveY(startX);
                ctx.moveTo(startX, firstY);

                for (var x = startX + step; x <= endX; x += step) {
                    var y = getWaveY(x);
                    ctx.lineTo(x, y);
                }

                ctx.lineTo(endX, canvas.height);
                ctx.lineTo(startX, canvas.height);
                ctx.closePath();
                ctx.fill();

                ctx.restore();
            }
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            if (width <= 0 || height <= 0 || !PerformanceState.anyUiVisible) return;

            var mX = root.trackX;
            var mY = root.trackY;
            var hasMouse = mouseTracker.containsMouse;

            ctx.save();
            ctx.translate(width / 2, height / 2);
            ctx.rotate(-22 * Math.PI / 180);

            var cx = width / 2;
            var cy = height / 2;
            var theta = -22 * Math.PI / 180;
            var cosT = Math.cos(theta);
            var sinT = Math.sin(theta);
            
            var rotMX = (mX - cx) * cosT + (mY - cy) * sinT;
            var rotMY = -(mX - cx) * sinT + (mY - cy) * cosT;

            var startX = -width;
            var endX = width * 2;
            var step = 8;

            drawRibbon(ctx, 1, startX, endX, step, rotMX, rotMY, hasMouse);
            drawRibbon(ctx, 2, startX, endX, step, rotMX, rotMY, hasMouse);
            drawRibbon(ctx, 3, startX, endX, step, rotMX, rotMY, hasMouse);

            ctx.restore();

            // Tiled noise shimmer overlay
            if (noisePatternCanvas) {
                ctx.save();
                ctx.globalCompositeOperation = "source-over";
                ctx.globalAlpha = 0.035;
                var offsetX = Math.floor(Math.random() * 128);
                var offsetY = Math.floor(Math.random() * 128);
                
                for (var x = 0; x < width; x += 128) {
                    for (var y = 0; y < height; y += 128) {
                        ctx.drawImage(noisePatternCanvas, x - offsetX, y - offsetY);
                    }
                }
                ctx.restore();
            }
        }
    }
}

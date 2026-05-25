import QtQuick 2.15
import CodexBarX 1.0

Item {
    id: root
    anchors.fill: parent
    clip: true

    readonly property bool glassEffectActive: SettingsStore.glassEffectEnabled
    readonly property bool decorativeMotionEnabled: !AppTheme.reduceMotion && SettingsStore.visualEffectsQuality !== "low"
    readonly property real qualityOpacity: SettingsStore.visualEffectsQuality === "high" ? 1.0
        : SettingsStore.visualEffectsQuality === "low" ? 0.45
        : 0.75

    opacity: (glassEffectActive ? 1.0 : 0.4) * qualityOpacity
    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionPanel); easing.type: Easing.InOutQuad }
    }

    // === Spring-Based Physics Mouse Tracking ===
    property real targetX: width / 2
    property real targetY: height / 2

    property real trackX: width / 2
    property real trackY: height / 2

    Behavior on trackX {
        id: springX
        SpringAnimation {
            spring: 1.5
            damping: 0.22
            epsilon: 0.25
        }
    }

    Behavior on trackY {
        id: springY
        SpringAnimation {
            spring: 1.5
            damping: 0.22
            epsilon: 0.25
        }
    }

    // Restore to center when mouse exits
    onWidthChanged: {
        if (!mouseTracker.containsMouse) {
            targetX = width / 2;
        }
    }
    onHeightChanged: {
        if (!mouseTracker.containsMouse) {
            targetY = height / 2;
        }
    }

    MouseArea {
        id: mouseTracker
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true
        acceptedButtons: Qt.NoButton // Absolutely non-blocking for all mouse buttons

        onPositionChanged: {
            root.targetX = mouse.x;
            root.targetY = mouse.y;
        }

        onEntered: {
            root.targetX = mouseTracker.mouseX;
            root.targetY = mouseTracker.mouseY;
        }

        onExited: {
            root.targetX = root.width / 2;
            root.targetY = root.height / 2;
        }
    }

    // Bind tracking target to properties
    onTargetXChanged: trackX = targetX
    onTargetYChanged: trackY = targetY

    // === Declarative Coordinate Drift Loops (Drifting Orbs) ===
    property real blob1X: width * 0.2
    property real blob1Y: height * 0.25
    property real blob2X: width * 0.7
    property real blob2Y: height * 0.65
    property real blob3X: width * 0.45
    property real blob3Y: height * 0.45

    SequentialAnimation on blob1X {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.width * 0.12; to: root.width * 0.38; duration: 24000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.width * 0.38; to: root.width * 0.12; duration: 24000; easing.type: Easing.InOutQuad }
    }
    SequentialAnimation on blob1Y {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.height * 0.15; to: root.height * 0.35; duration: 28000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.height * 0.35; to: root.height * 0.15; duration: 28000; easing.type: Easing.InOutQuad }
    }

    SequentialAnimation on blob2X {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.width * 0.82; to: root.width * 0.58; duration: 32000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.width * 0.58; to: root.width * 0.82; duration: 32000; easing.type: Easing.InOutQuad }
    }
    SequentialAnimation on blob2Y {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.height * 0.75; to: root.height * 0.55; duration: 26000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.height * 0.55; to: root.height * 0.75; duration: 26000; easing.type: Easing.InOutQuad }
    }

    SequentialAnimation on blob3X {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.width * 0.25; to: root.width * 0.75; duration: 38000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.width * 0.75; to: root.width * 0.25; duration: 38000; easing.type: Easing.InOutQuad }
    }
    SequentialAnimation on blob3Y {
        running: root.decorativeMotionEnabled
        loops: Animation.Infinite
        NumberAnimation { from: root.height * 0.32; to: root.height * 0.68; duration: 34000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: root.height * 0.68; to: root.height * 0.32; duration: 34000; easing.type: Easing.InOutQuad }
    }

    // === Unified High-Performance Blending Canvas ===
    Canvas {
        id: paintCanvas
        anchors.fill: parent
        antialiasing: true

        // Parallax Coordinate Computation with Spring Mouse tracking
        readonly property real drawX1: root.blob1X + (root.trackX - root.width / 2) * 0.15
        readonly property real drawY1: root.blob1Y + (root.trackY - root.height / 2) * 0.15
        readonly property real drawX2: root.blob2X + (root.trackX - root.width / 2) * 0.08
        readonly property real drawY2: root.blob2Y + (root.trackY - root.height / 2) * 0.08
        readonly property real drawX3: root.blob3X - (root.trackX - root.width / 2) * 0.05
        readonly property real drawY3: root.blob3Y - (root.trackY - root.height / 2) * 0.05

        onDrawX1Changed: requestPaint()
        onDrawY1Changed: requestPaint()
        onDrawX2Changed: requestPaint()
        onDrawY2Changed: requestPaint()
        onDrawX3Changed: requestPaint()
        onDrawY3Changed: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            ctx.clearRect(0, 0, width, height);

            if (width <= 0 || height <= 0) return;

            // --- Draw Blob 1 (Codex Brand Accent - Deep Indigo Glow) ---
            var r1 = Math.max(180, width * 0.52);
            var g1 = ctx.createRadialGradient(drawX1, drawY1, 0, drawX1, drawY1, r1);
            g1.addColorStop(0.0, AppTheme.accentColor || "#5e5ce6");
            g1.addColorStop(1.0, "transparent");
            ctx.fillStyle = g1;
            ctx.globalAlpha = 0.09;
            ctx.fillRect(0, 0, width, height);

            // --- Enable 'lighter' composite mode for dynamic fluid color blending ---
            ctx.globalCompositeOperation = "lighter";

            // --- Draw Blob 2 (Claude Brand Accent - Deep Amber Glow) ---
            var r2 = Math.max(160, width * 0.46);
            var g2 = ctx.createRadialGradient(drawX2, drawY2, 0, drawX2, drawY2, r2);
            g2.addColorStop(0.0, AppTheme.providerBrandColor("claude") || "#d97706");
            g2.addColorStop(1.0, "transparent");
            ctx.fillStyle = g2;
            ctx.globalAlpha = 0.07;
            ctx.fillRect(0, 0, width, height);

            // --- Draw Blob 3 (DeepSeek Brand Accent - Soft Blue-Cyan Glow) ---
            var r3 = Math.max(170, width * 0.48);
            var g3 = ctx.createRadialGradient(drawX3, drawY3, 0, drawX3, drawY3, r3);
            g3.addColorStop(0.0, AppTheme.providerBrandColor("deepseek") || "#3b82f6");
            g3.addColorStop(1.0, "transparent");
            ctx.fillStyle = g3;
            ctx.globalAlpha = 0.08;
            ctx.fillRect(0, 0, width, height);
        }
    }
}

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

    Canvas {
        id: blob1
        width: Math.max(300, parent.width * 0.9)
        height: width
        opacity: 0.065
        z: -3

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var cx = width / 2;
            var cy = height / 2;
            var radius = width / 2;
            var gradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
            gradient.addColorStop(0.0, AppTheme.accentColor || "#5e5ce6");
            gradient.addColorStop(1.0, "transparent");
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        SequentialAnimation on x {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: -blob1.width * 0.4; to: -blob1.width * 0.1; duration: 18000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: -blob1.width * 0.1; to: -blob1.width * 0.4; duration: 18000; easing.type: Easing.InOutQuad }
        }

        SequentialAnimation on y {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: -blob1.height * 0.3; to: -blob1.height * 0.1; duration: 22000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: -blob1.height * 0.1; to: -blob1.height * 0.3; duration: 22000; easing.type: Easing.InOutQuad }
        }
    }

    Canvas {
        id: blob2
        width: Math.max(260, parent.width * 0.8)
        height: width
        opacity: 0.05
        z: -2

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var cx = width / 2;
            var cy = height / 2;
            var radius = width / 2;
            var gradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
            gradient.addColorStop(0.0, AppTheme.providerBrandColor("claude") || "#d97706");
            gradient.addColorStop(1.0, "transparent");
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        SequentialAnimation on x {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: root.width - blob2.width * 0.7; to: root.width - blob2.width * 0.4; duration: 25000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.width - blob2.width * 0.4; to: root.width - blob2.width * 0.7; duration: 25000; easing.type: Easing.InOutQuad }
        }

        SequentialAnimation on y {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: root.height - blob2.height * 0.7; to: root.height - blob2.height * 0.4; duration: 20000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.height - blob2.height * 0.4; to: root.height - blob2.height * 0.7; duration: 20000; easing.type: Easing.InOutQuad }
        }
    }

    Canvas {
        id: blob3
        width: Math.max(280, parent.width * 0.85)
        height: width
        opacity: 0.055
        z: -1

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var cx = width / 2;
            var cy = height / 2;
            var radius = width / 2;
            var gradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
            gradient.addColorStop(0.0, AppTheme.providerBrandColor("deepseek") || "#3b82f6");
            gradient.addColorStop(1.0, "transparent");
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, width, height);
        }
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        SequentialAnimation on x {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: -blob3.width * 0.2; to: root.width - blob3.width * 0.8; duration: 21000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.width - blob3.width * 0.8; to: -blob3.width * 0.2; duration: 21000; easing.type: Easing.InOutQuad }
        }

        SequentialAnimation on y {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: root.height * 0.3; to: root.height * 0.6; duration: 24000; easing.type: Easing.InOutQuad }
            NumberAnimation { from: root.height * 0.6; to: root.height * 0.3; duration: 24000; easing.type: Easing.InOutQuad }
        }
    }
}

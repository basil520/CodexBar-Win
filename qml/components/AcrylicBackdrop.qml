import QtQuick 2.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    visible: SettingsStore.glassEffectEnabled
    color: "transparent"

    property color tint: AppTheme.bgPrimary
    readonly property real materialOpacity: AppTheme.glassMaterialOpacity
    readonly property bool decorativeMotionEnabled: !AppTheme.reduceMotion && SettingsStore.visualEffectsQuality === "high"
    readonly property color surfaceScrimOverlay: AppTheme.surfaceScrim
    readonly property color tintScrim: Qt.rgba(root.tint.r, root.tint.g, root.tint.b,
                                              Math.min(0.54, Math.max(0.28, root.materialOpacity * 0.62)))

    // Inherit radius from parent so the acrylic layers are clipped to
    // the rounded shape instead of painting into the corner areas.
    radius: parent && parent.hasOwnProperty("radius") ? parent.radius : 0
    clip: true

    Rectangle {
        anchors.fill: parent
        color: AppTheme.surfaceWindow
    }

    Rectangle {
        anchors.fill: parent
        color: root.tintScrim
    }

    Rectangle {
        anchors.fill: parent
        color: root.surfaceScrimOverlay
        opacity: 0.42
    }

    Rectangle {
        id: auroraBlob1
        width: Math.max(200, parent.width * 0.75)
        height: Math.max(200, parent.width * 0.75)
        radius: width / 2
        opacity: 0.045
        z: -2

        gradient: Gradient {
            GradientStop { position: 0.0; color: AppTheme.accentColor }
            GradientStop { position: 1.0; color: "transparent" }
        }

        SequentialAnimation on x {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: -auroraBlob1.width * 0.3; to: -auroraBlob1.width * 0.1; duration: 8000; easing.type: Easing.InOutSine }
            NumberAnimation { from: -auroraBlob1.width * 0.1; to: -auroraBlob1.width * 0.3; duration: 8000; easing.type: Easing.InOutSine }
        }

        SequentialAnimation on y {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: -auroraBlob1.height * 0.2; to: -auroraBlob1.height * 0.4; duration: 9000; easing.type: Easing.InOutSine }
            NumberAnimation { from: -auroraBlob1.height * 0.4; to: -auroraBlob1.height * 0.2; duration: 9000; easing.type: Easing.InOutSine }
        }
    }

    Rectangle {
        id: auroraBlob2
        width: Math.max(200, parent.height * 0.8)
        height: Math.max(200, parent.height * 0.8)
        radius: width / 2
        opacity: 0.035
        z: -1

        gradient: Gradient {
            GradientStop { position: 0.0; color: AppTheme.providerBrandColor("claude") }
            GradientStop { position: 1.0; color: "transparent" }
        }

        SequentialAnimation on x {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: root.width - auroraBlob2.width * 0.6; to: root.width - auroraBlob2.width * 0.8; duration: 11000; easing.type: Easing.InOutSine }
            NumberAnimation { from: root.width - auroraBlob2.width * 0.8; to: root.width - auroraBlob2.width * 0.6; duration: 11000; easing.type: Easing.InOutSine }
        }

        SequentialAnimation on y {
            running: root.decorativeMotionEnabled
            loops: Animation.Infinite
            NumberAnimation { from: root.height - auroraBlob2.height * 0.8; to: root.height - auroraBlob2.height * 0.6; duration: 10000; easing.type: Easing.InOutSine }
            NumberAnimation { from: root.height - auroraBlob2.height * 0.6; to: root.height - auroraBlob2.height * 0.8; duration: 10000; easing.type: Easing.InOutSine }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(root.tint.r, root.tint.g, root.tint.b, 0.18) }
            GradientStop { position: 0.25; color: Qt.rgba(255, 255, 255, 0.035) }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.035) }
        }
    }

    Canvas {
        id: noiseCanvas
        anchors.fill: parent
        opacity: 0.10 + (1.0 - root.materialOpacity) * 0.18

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            if (width <= 0 || height <= 0) return

            var seed = 2463534242
            var count = Math.max(800, Math.floor(width * height / 28))
            for (var i = 0; i < count; ++i) {
                seed = (seed * 1664525 + 1013904223) >>> 0
                var x = seed % width
                seed = (seed * 1664525 + 1013904223) >>> 0
                var y = seed % height
                seed = (seed * 1664525 + 1013904223) >>> 0
                var alpha = 0.035 + ((seed & 31) / 31.0) * 0.085
                ctx.fillStyle = "rgba(255,255,255," + alpha + ")"
                ctx.fillRect(x, y, 1, 1)
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: AppTheme.surfaceBorder
    }
}

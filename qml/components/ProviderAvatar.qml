import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

Item {
    id: root

    property string providerId: ""
    property string displayName: ""
    property string iconSource: ""
    property color brandColor: AppTheme.providerBrandColor(providerId)
    property int size: 24
    property bool selected: false
    property string severity: "none"
    property bool showProgressRing: false
    property real usagePercent: 100

    readonly property var iconPolicy: policy.iconPolicy(providerId)
    readonly property real paddingRatio: iconPolicy.paddingRatio !== undefined ? iconPolicy.paddingRatio : 0.16
    readonly property int iconPadding: Math.max(1, Math.round(size * paddingRatio))
    readonly property int iconSize: Math.max(8, size - iconPadding * 2)
    readonly property bool preserveBackground: iconPolicy.preserveBackground === true
    readonly property string surfaceMode: iconPolicy.surfaceMode || "brandSoft"
    readonly property string effectiveIconSource: iconSource && iconSource !== ""
        ? iconSource
        : (providerId && providerId !== "" ? "qrc:/icons/ProviderIcon-" + providerId + ".svg" : "")
    readonly property string fallbackText: {
        var label = displayName && displayName !== "" ? displayName : providerId
        return label && label.length > 0 ? label.charAt(0).toUpperCase() : "?"
    }
    readonly property color severityColor: {
        if (severity === "error" || severity === "outage") return AppTheme.statusOutage
        if (severity === "warning" || severity === "degraded") return AppTheme.statusDegraded
        if (severity === "ok") return AppTheme.statusOk
        return AppTheme.statusUnknown
    }

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size
    opacity: enabled ? 1.0 : 0.68

    ProviderIconPolicy {
        id: policy
    }

    function surfaceColor() {
        if (surfaceMode === "none") {
            if (selected) return AppTheme.withAlpha(root.brandColor, 0.18)
            return "transparent"
        }
        if (surfaceMode === "light") {
            return Qt.rgba(0.96, 0.98, 1.0, selected ? 0.98 : 0.92)
        }
        if (surfaceMode === "dark") {
            return AppTheme.surfaceControl
        }
        return AppTheme.withAlpha(root.brandColor, selected ? 0.30 : 0.18)
    }

    function borderColor() {
        if (selected) return root.brandColor
        if (surfaceMode === "light") return Qt.rgba(0.10, 0.14, 0.22, 0.22)
        if (surfaceMode === "none") return AppTheme.withAlpha(AppTheme.surfaceBorder, 0.65)
        return AppTheme.withAlpha(root.brandColor, 0.34)
    }

    Rectangle {
        id: surface
        anchors.fill: parent
        anchors.margins: root.showProgressRing ? 2 : 0
        radius: Math.max(4, width * 0.28)
        color: root.surfaceColor()
        border.width: root.surfaceMode === "none" && !root.selected ? 0 : 1
        border.color: root.borderColor()

        Behavior on color { ColorAnimation { duration: 150 } }
    }

    Image {
        id: providerImage
        anchors.centerIn: surface
        width: root.preserveBackground ? surface.width : root.iconSize
        height: root.preserveBackground ? surface.height : root.iconSize
        source: root.effectiveIconSource
        fillMode: Image.PreserveAspectFit
        sourceSize.width: width
        sourceSize.height: height
        smooth: true
        visible: status === Image.Ready
    }

    Rectangle {
        id: fallback
        anchors.fill: surface
        radius: surface.radius
        color: root.brandColor
        visible: providerImage.status !== Image.Ready

        Label {
            anchors.fill: parent
            anchors.margins: 1
            text: root.fallbackText
            color: AppTheme.textOnAccent
            font.pixelSize: Math.max(8, Math.round(root.size * 0.42))
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    Canvas {
        id: progressRing
        anchors.fill: parent
        visible: root.showProgressRing
        property real normalizedValue: Math.max(0, Math.min(1, root.usagePercent / 100.0))

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var cx = width / 2
            var cy = height / 2
            var radius = Math.max(3, Math.min(width, height) / 2 - 1.5)
            ctx.beginPath()
            ctx.strokeStyle = AppTheme.withAlpha(root.brandColor, root.selected ? 0.30 : 0.18)
            ctx.lineWidth = root.selected ? 2 : 1.25
            ctx.arc(cx, cy, radius, 0, 2 * Math.PI)
            ctx.stroke()
            if (normalizedValue > 0) {
                ctx.beginPath()
                ctx.strokeStyle = root.brandColor
                ctx.lineWidth = root.selected ? 2.4 : 1.6
                ctx.arc(cx, cy, radius, -0.5 * Math.PI, normalizedValue * 2 * Math.PI - 0.5 * Math.PI)
                ctx.stroke()
            }
        }

        onNormalizedValueChanged: requestPaint()
        onVisibleChanged: requestPaint()
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        width: Math.max(6, Math.round(root.size * 0.20))
        height: width
        radius: width / 2
        color: root.severityColor
        border.width: 1
        border.color: AppTheme.surfaceWindow
        visible: root.severity !== "" && root.severity !== "none"
    }
}

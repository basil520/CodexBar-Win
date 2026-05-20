import QtQuick 2.15
import ".."

Rectangle {
    id: root

    property string text: ""
    property string variant: "secondary" // primary, secondary, ghost, danger
    property bool busy: false
    property bool compact: false
    property int horizontalPadding: compact ? 10 : 14
    property int minWidth: compact ? 28 : 48
    property alias font: label.font

    signal clicked()

    implicitWidth: Math.max(minWidth, label.implicitWidth + horizontalPadding * 2)
    implicitHeight: compact ? 28 : 34
    radius: AppTheme.radiusMd
    color: backgroundColor()
    border.width: variant === "primary" ? 0 : 1
    border.color: borderColor()
    opacity: enabled ? 1.0 : 0.55

    function backgroundColor() {
        if (!enabled) return AppTheme.withAlpha(AppTheme.surfaceHover, 0.34)
        if (pressHandler.pressed) {
            if (variant === "primary") return AppTheme.accentHover
            if (variant === "danger") return AppTheme.withAlpha(AppTheme.statusOutage, 0.26)
            return AppTheme.surfaceInteractivePressed
        }
        if (hoverHandler.hovered) {
            if (variant === "primary") return AppTheme.accentHover
            if (variant === "danger") return AppTheme.withAlpha(AppTheme.statusOutage, 0.18)
            return AppTheme.surfaceInteractiveHover
        }
        if (variant === "primary") return AppTheme.accentColor
        if (variant === "danger") return AppTheme.surfaceDangerSoft
        if (variant === "ghost") return "transparent"
        return AppTheme.surfaceInteractive
    }

    function borderColor() {
        if (variant === "danger") return AppTheme.withAlpha(AppTheme.statusOutage, 0.58)
        if (variant === "ghost") return AppTheme.withAlpha(AppTheme.borderSubtle, hoverHandler.hovered ? 0.78 : 0.42)
        return AppTheme.borderSubtle
    }

    Behavior on color {
        ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
    }
    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }

    TapHandler {
        id: pressHandler
        enabled: root.enabled && !root.busy
        onTapped: root.clicked()
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.busy ? qsTr("Working...") : root.text
        color: {
            if (!root.enabled) return AppTheme.textDisabled
            if (root.variant === "primary") return AppTheme.textOnAccent
            if (root.variant === "danger") return AppTheme.statusOutage
            return AppTheme.textPrimary
        }
        font.pixelSize: AppTheme.fontSizeSm
        font.bold: root.variant === "primary"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}

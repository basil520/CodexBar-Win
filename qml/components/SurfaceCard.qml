import QtQuick 2.15
import ".."

Rectangle {
    id: root

    property bool interactive: false
    property bool selected: false
    property string tone: "neutral" // neutral, success, warning, danger
    property bool pressed: false
    property bool hovered: hoverHandler.hovered

    radius: AppTheme.radiusLg
    color: surfaceColor()
    border.width: 1
    border.color: borderColor()

    function surfaceColor() {
        if (tone === "success") return AppTheme.surfaceSuccessSoft
        if (tone === "warning") return AppTheme.surfaceWarningSoft
        if (tone === "danger" || tone === "error") return AppTheme.surfaceDangerSoft
        if (pressed) return AppTheme.surfaceInteractivePressed
        if (interactive && hovered) return AppTheme.surfaceInteractiveHover
        if (selected) return AppTheme.surfaceSelected
        return AppTheme.surfaceCard
    }

    function borderColor() {
        if (selected) return AppTheme.borderFocus
        if (tone === "success") return AppTheme.withAlpha(AppTheme.statusOk, 0.56)
        if (tone === "warning") return AppTheme.withAlpha(AppTheme.statusDegraded, 0.56)
        if (tone === "danger" || tone === "error") return AppTheme.withAlpha(AppTheme.statusOutage, 0.56)
        return AppTheme.borderSubtle
    }

    Behavior on color {
        ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
    }
    Behavior on border.color {
        ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
    }

    HoverHandler {
        id: hoverHandler
        enabled: root.interactive
    }
}

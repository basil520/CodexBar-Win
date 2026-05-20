import QtQuick 2.15
import ".."

Rectangle {
    id: root

    property real pulseOpacity: 0.58

    radius: Math.min(height / 2, AppTheme.radiusMd)
    color: AppTheme.withAlpha(AppTheme.textTertiary, 0.16)
    opacity: pulseOpacity

    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionSlow); easing.type: AppTheme.easeStandard }
    }
}

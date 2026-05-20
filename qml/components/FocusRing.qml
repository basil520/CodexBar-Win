import QtQuick 2.15
import ".."

Rectangle {
    id: root

    property bool active: false
    property color ringColor: AppTheme.borderFocus

    color: "transparent"
    radius: AppTheme.radiusMd
    border.width: active ? 2 : 0
    border.color: ringColor
    opacity: active ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
    }
}

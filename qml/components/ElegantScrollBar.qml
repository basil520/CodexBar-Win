import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

ScrollBar {
    id: root

    property var flickable: null

    policy: ScrollBar.AsNeeded
    active: hovered || pressed || (flickable && (flickable.moving === true || flickable.flicking === true))

    background: Rectangle {
        color: "transparent"
    }

    contentItem: Rectangle {
        implicitWidth: 4
        radius: 2
        opacity: root.active ? 1.0 : 0.0
        color: root.hovered
            ? AppTheme.textSecondary
            : AppTheme.withAlpha(AppTheme.textSecondary, 0.35)

        Behavior on color {
            ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
        }
        Behavior on opacity {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
        }
        Behavior on implicitWidth {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
        }
    }

    states: State {
        name: "hoveredState"
        when: root.hovered
        PropertyChanges {
            target: root.contentItem
            implicitWidth: 8
            radius: 4
        }
    }
}

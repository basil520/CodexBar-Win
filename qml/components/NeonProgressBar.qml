import QtQuick 2.15
import ".."

Item {
    id: root
    implicitWidth: 200
    implicitHeight: 6

    property real progress: 0.0 // 0.0 到 1.0
    property color startColor: AppTheme.accentColor
    property color endColor: Qt.lighter(AppTheme.accentColor, 1.25)
    property bool showPaceWarning: false
    property real paceValue: 0.0

    // 轨道背景
    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: AppTheme.surfaceCard
        border.color: AppTheme.borderSubtle
        border.width: 0.5
    }

    // 渐变进度填充
    Rectangle {
        id: filler
        width: Math.max(0, parent.width * Math.min(1.0, root.progress))
        height: parent.height
        radius: height / 2

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.startColor }
            GradientStop { position: 1.0; color: root.endColor }
        }

        Behavior on width {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionSlow); easing.type: AppTheme.easeStandard }
        }

        // 前端光晕耀斑 (Glow Flare)
        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height * 1.6
            height: parent.height * 1.6
            radius: width / 2
            color: "#FFFFFF"
            opacity: root.progress > 0.05 ? 0.9 : 0.0

            // 外部环形虚晕
            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 2.2
                height: parent.height * 2.2
                radius: width / 2
                color: root.endColor
                opacity: 0.4
            }
        }
    }

    // 用量警戒指针
    Rectangle {
        visible: root.showPaceWarning
        x: Math.max(0, Math.min(parent.width - 3, parent.width * root.paceValue - 1.5))
        width: 3
        height: parent.height + 4
        anchors.verticalCenter: parent.verticalCenter
        color: AppTheme.statusOutage
        radius: 1.5
    }
}

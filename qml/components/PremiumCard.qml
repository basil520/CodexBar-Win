import QtQuick 2.15
import ".."

Item {
    id: root
    property alias contentData: container.data
    property color glowColor: AppTheme.accentColor
    property bool isInteractive: true
    property bool isSelected: false
    property real cardRadius: 10

    signal clicked()

    implicitWidth: 320
    implicitHeight: container.implicitHeight + 24

    // 物理微缩反馈
    scale: isInteractive && mouseArea.pressed ? 0.98 : (isInteractive && mouseArea.containsMouse ? 1.01 : 1.0)
    Behavior on scale { NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard } }

    // 底座卡片
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: root.cardRadius
        color: root.isSelected ? AppTheme.surfaceInteractive : AppTheme.surfaceCard
        border.color: root.isSelected ? root.glowColor : (mouseArea.containsMouse ? Qt.rgba(root.glowColor.r, root.glowColor.g, root.glowColor.b, 0.4) : AppTheme.borderSubtle)
        border.width: 1

        Behavior on color { ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal) } }
        Behavior on border.color { ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal) } }

        // 玻璃高光内描边 (Inner Highlight)
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: bg.radius - 1
            color: "transparent"
            border.color: Qt.rgba(1, 1, 1, 0.05)
            border.width: 1
        }
    }

    // 呼吸发光晕圈 (Selected / Hover Glow)
    Rectangle {
        anchors.fill: parent
        radius: root.cardRadius
        color: "transparent"
        border.color: root.glowColor
        border.width: 2
        opacity: root.isSelected ? 0.4 : (mouseArea.containsMouse ? 0.15 : 0.0)
        visible: isInteractive
        
        Behavior on opacity { NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal) } }
    }

    // 鼠标点击捕获层
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.isInteractive
        enabled: root.isInteractive
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    // 内容锚定区
    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 12
    }
}

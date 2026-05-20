import QtQuick 2.15
import ".."

Rectangle {
    id: root
    property bool checked: false
    signal toggled(bool checked)

    implicitWidth: 34
    implicitHeight: 20
    radius: height / 2
    opacity: enabled ? 1.0 : 0.45
    color: checked
        ? Qt.rgba(AppTheme.accentColor.r, AppTheme.accentColor.g, AppTheme.accentColor.b, 0.95)
        : AppTheme.surfaceHover
    border.width: 1
    border.color: checked ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder

    // Premium sweep gradient background
    gradient: checked ? checkedGradient : null

    Gradient {
        id: checkedGradient
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: AppTheme.accentColor }
        GradientStop { position: 1.0; color: Qt.lighter(AppTheme.accentColor, 1.25) }
    }

    Behavior on color {
        ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
    }

    Rectangle {
        id: knob
        // Haptic physical stretch width
        width: mouseArea.containsPress ? 16 : (mouseArea.containsMouse ? 15 : 14)
        height: 14
        radius: 7
        x: root.checked ? root.width - width - 3 : 3
        y: 3
        color: root.checked ? AppTheme.textOnAccent : AppTheme.textSecondary

        Behavior on x {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeEmphasized }
        }
        Behavior on width {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.toggled(!root.checked)
        }
    }
}

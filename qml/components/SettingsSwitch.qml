import QtQuick 2.15
import ".."

Item {
    id: root

    property bool checked: false
    property string accessibleName: qsTr("Switch")

    signal toggled(bool checked)

    implicitWidth: 42
    implicitHeight: 32
    opacity: enabled ? 1.0 : 0.45
    activeFocusOnTab: enabled
    Accessible.role: Accessible.CheckBox
    Accessible.name: accessibleName
    Accessible.description: checked ? qsTr("On") : qsTr("Off")

    function activate() {
        if (!root.enabled) return
        root.toggled(!root.checked)
    }

    Keys.onReturnPressed: function(event) {
        event.accepted = true
        root.activate()
    }
    Keys.onEnterPressed: function(event) {
        event.accepted = true
        root.activate()
    }
    Keys.onSpacePressed: function(event) {
        event.accepted = true
        root.activate()
    }

    Rectangle {
        id: track

        width: 34
        height: 20
        anchors.centerIn: parent
        radius: height / 2
        color: root.checked
            ? Qt.rgba(AppTheme.accentColor.r, AppTheme.accentColor.g, AppTheme.accentColor.b, 0.95)
            : AppTheme.surfaceHover
        border.width: 1
        border.color: root.checked ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
        gradient: root.checked ? checkedGradient : null

        Behavior on color {
            ColorAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
        }

        Rectangle {
            id: knob

            width: mouseArea.containsPress ? 16 : (mouseArea.containsMouse ? 15 : 14)
            height: 14
            radius: 7
            x: root.checked ? track.width - width - 3 : 3
            y: 3
            color: root.checked ? AppTheme.textOnAccent : AppTheme.textSecondary

            Behavior on x {
                NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeEmphasized }
            }
            Behavior on width {
                NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
            }
        }
    }

    Gradient {
        id: checkedGradient
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: AppTheme.accentColor }
        GradientStop { position: 1.0; color: Qt.lighter(AppTheme.accentColor, 1.25) }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.activate()
    }

    FocusRing {
        anchors.fill: parent
        radius: AppTheme.radiusMd
        active: root.activeFocus
    }
}

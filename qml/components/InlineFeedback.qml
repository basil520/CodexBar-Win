import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

RowLayout {
    id: root

    property string status: "info" // info, busy, success, warning, error
    property string message: ""
    property bool compact: false

    spacing: AppTheme.spacingSm
    visible: message.length > 0 || status === "busy"

    function toneColor() {
        if (status === "success") return AppTheme.statusOk
        if (status === "warning") return AppTheme.statusDegraded
        if (status === "error") return AppTheme.statusOutage
        return AppTheme.accentColor
    }

    Rectangle {
        Layout.preferredWidth: compact ? 7 : 9
        Layout.preferredHeight: width
        radius: width / 2
        color: root.toneColor()
        opacity: root.status === "busy" ? 0.72 : 1.0

        Behavior on opacity {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionSlow); easing.type: AppTheme.easeStandard }
        }
    }

    Text {
        Layout.fillWidth: true
        text: root.status === "busy" && root.message.length === 0 ? qsTr("Working...") : root.message
        color: root.status === "error" ? AppTheme.statusOutage : AppTheme.textSecondary
        font.pixelSize: root.compact ? AppTheme.fontSizeXs : AppTheme.fontSizeSm
        wrapMode: Text.WordWrap
        elide: root.compact ? Text.ElideRight : Text.ElideNone
        maximumLineCount: root.compact ? 1 : 3
    }
}

import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

RowLayout {
    id: root

    property string status: "info" // info, busy, success, warning, error
    property string message: ""
    property bool compact: false
    property bool copyable: false
    property string copyPayload: message

    signal copyRequested(string text)

    spacing: AppTheme.spacingSm
    visible: message.length > 0 || status === "busy"

    function toneColor() {
        if (status === "success") return AppTheme.statusOk
        if (status === "warning") return AppTheme.statusDegraded
        if (status === "error") return AppTheme.statusOutage
        return AppTheme.accentColor
    }

    StatusDot {
        Layout.preferredWidth: compact ? 7 : 9
        Layout.preferredHeight: compact ? 7 : 9
        size: compact ? 7 : 9
        state: root.status
        toneColor: root.toneColor()
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

    IconButton {
        visible: root.copyable
        enabled: root.copyPayload !== ""
        width: 24
        height: 24
        implicitWidth: 24
        implicitHeight: 24
        symbol: "copy"
        accessibleName: qsTr("Copy message")
        tooltip: enabled ? qsTr("Copy message") : qsTr("Nothing to copy")
        iconColor: root.toneColor()
        onActivated: root.copyRequested(root.copyPayload)
    }
}

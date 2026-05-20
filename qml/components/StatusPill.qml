import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: root

    property string text: ""
    property string state: "unknown"
    property color toneColor: {
        if (state === "ok" || state === "succeeded") return AppTheme.statusOk
        if (state === "degraded" || state === "testing" || state === "refreshing") return AppTheme.statusDegraded
        if (state === "outage" || state === "failed" || state === "error") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    readonly property int desiredWidth: Math.max(58, pillLabel.implicitWidth + 20)

    Layout.preferredWidth: desiredWidth
    Layout.minimumWidth: desiredWidth
    Layout.maximumWidth: desiredWidth
    Layout.preferredHeight: 24
    implicitWidth: desiredWidth
    implicitHeight: 24
    radius: 12
    color: AppTheme.withAlpha(toneColor, AppTheme.glassActive ? 0.16 : 0.12)
    border.width: 1
    border.color: AppTheme.withAlpha(toneColor, AppTheme.glassActive ? 0.46 : 0.36)
    opacity: enabled ? 1.0 : 0.58
    clip: true

    Label {
        id: pillLabel
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        text: root.text !== "" ? root.text : root.state
        color: root.toneColor
        font.pixelSize: AppTheme.fontSizeSm
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}

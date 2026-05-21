import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

SettingsGroupBox {
    id: root
    property string errorTitle: "Error"
    property string errorMessage: ""
    property bool expanded: false

    color: Qt.rgba(AppTheme.statusOutage.r, AppTheme.statusOutage.g, AppTheme.statusOutage.b, 0.10)
    border.color: Qt.rgba(AppTheme.statusOutage.r, AppTheme.statusOutage.g, AppTheme.statusOutage.b, 0.35)
    border.width: 1

    ErrorNotice {
        Layout.fillWidth: true
        title: root.errorTitle
        message: root.errorMessage
        detail: root.errorMessage
        density: "diagnostic"
        severity: "error"
        expanded: root.expanded
        onExpandedChanged: root.expanded = expanded
        onCopyRequested: function(text) {
            AppController.copyWithFeedback(text)
        }
    }
}

import QtQuick 2.15
import "../.."
import ".." as Components

Components.ActionButton {
    id: root

    property bool canRun: true
    property string disabledReason: ""
    property string busyLabel: qsTr("Working...")
    property string lastResult: ""
    property string lastError: ""
    property double lastFinishedAt: 0

    enabled: canRun && disabledReason === ""

    Accessible.description: busy ? busyLabel : disabledReason
}

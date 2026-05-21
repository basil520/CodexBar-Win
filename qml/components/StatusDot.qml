import QtQuick 2.15
import ".."

Rectangle {
    id: root

    property string state: "unknown"
    property string severity: state
    property int size: 8
    property color toneColor: {
        if (severity === "ok" || severity === "success" || severity === "succeeded") return AppTheme.statusOk
        if (severity === "warning" || severity === "degraded" || severity === "testing" || severity === "refreshing") return AppTheme.statusDegraded
        if (severity === "error" || severity === "outage" || severity === "failed") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size
    radius: size / 2
    color: toneColor
}

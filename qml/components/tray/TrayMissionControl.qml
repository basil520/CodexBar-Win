import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../.."

ColumnLayout {
    id: root

    property string selectedProviderId: ""
    property bool glassEffectActive: false
    property var activityEvents: []

    spacing: 0
    Accessible.name: qsTr("Mission Control")
}

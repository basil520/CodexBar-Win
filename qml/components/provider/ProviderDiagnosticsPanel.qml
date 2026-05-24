import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../.."
import "../state" as StateComponents

StateComponents.StateTimeline {
    id: root

    property string providerId: ""

    emptyTitle: qsTr("No diagnostics for this provider")
    maxEvents: 5
}

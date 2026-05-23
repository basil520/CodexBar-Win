import QtQuick 2.15
import ".."

Item {
    id: root

    property Item target
    property string providerId: ""
    property bool active: target ? target.visible : false
    property string stateSummary: active ? qsTr("Provider detail") : qsTr("Hidden")

    visible: false
}

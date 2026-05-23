import QtQuick 2.15
import ".."

Item {
    id: root

    property Item target
    property bool active: target ? target.visible : false
    property string stateSummary: active ? qsTr("Overview") : qsTr("Hidden")

    visible: false
}

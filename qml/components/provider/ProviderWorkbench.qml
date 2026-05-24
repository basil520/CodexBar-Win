import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../.."

ColumnLayout {
    id: root

    property string providerId: ""
    property string state: "idle"
    property string recommendedAction: ""

    spacing: AppTheme.spacingMd
    Accessible.name: providerId === "" ? qsTr("Provider Workbench") : qsTr("%1 Workbench").arg(providerId)
}

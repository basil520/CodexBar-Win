import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../.."

ColumnLayout {
    id: root

    property string title: qsTr("Usage Observatory")
    property string subtitle: qsTr("Trends, contribution, and quota risk")

    spacing: AppTheme.spacingMd
    Accessible.name: title
}

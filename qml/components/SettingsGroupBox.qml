import QtQuick 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    color: AppTheme.surfaceCard
    radius: AppTheme.radiusLg
    border.width: 1
    border.color: AppTheme.surfaceBorder
    Layout.fillWidth: true
    implicitHeight: groupLayout.implicitHeight + 32
    default property alias content: groupLayout.data

    ColumnLayout {
        id: groupLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
    }
}

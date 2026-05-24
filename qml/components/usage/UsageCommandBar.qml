import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SurfaceCard {
    id: root

    property string rangeLabel: qsTr("Last 30 days")
    property string freshnessLabel: ""
    property bool refreshing: false

    signal refreshRequested()

    implicitHeight: 54
    radius: AppTheme.radiusLg

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: AppTheme.spacingMd

        Components.StatusDot {
            Layout.preferredWidth: AppTheme.statusDotSize
            Layout.preferredHeight: AppTheme.statusDotSize
            color: root.refreshing ? AppTheme.statusDegraded : AppTheme.statusOk
        }

        Label {
            Layout.fillWidth: true
            text: root.rangeLabel
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            elide: Text.ElideRight
        }

        Label {
            text: root.freshnessLabel
            visible: text !== ""
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            elide: Text.ElideRight
        }

        Components.ActionButton {
            text: root.refreshing ? qsTr("Refreshing") : qsTr("Refresh")
            compact: true
            busy: root.refreshing
            enabled: !root.refreshing
            onClicked: root.refreshRequested()
        }
    }
}

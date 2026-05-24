import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SurfaceCard {
    id: root

    property int providerCount: 0
    property string globalState: "ready"
    property bool refreshing: false
    property string freshnessLabel: ""

    implicitHeight: 40
    radius: AppTheme.radiusMd
    color: AppTheme.surfacePane

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: AppTheme.spacingSm

        Components.StatusDot {
            Layout.preferredWidth: AppTheme.statusDotSize
            Layout.preferredHeight: AppTheme.statusDotSize
            color: root.refreshing ? AppTheme.statusDegraded
                : (root.globalState === "error" ? AppTheme.statusOutage : AppTheme.statusOk)
        }

        Label {
            Layout.fillWidth: true
            text: root.refreshing ? qsTr("Refreshing providers")
                : qsTr("Mission Control ready")
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeSm
            font.bold: true
            elide: Text.ElideRight
        }

        Label {
            text: qsTr("%1 providers").arg(root.providerCount)
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeXs
        }

        Label {
            visible: root.freshnessLabel !== ""
            text: root.freshnessLabel
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeXs
        }
    }
}

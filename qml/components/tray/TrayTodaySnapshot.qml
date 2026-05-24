import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SurfaceCard {
    id: root

    property string todayCost: "$0.00"
    property var providerCount: 0
    property string activeProviderName: ""
    property string statusText: qsTr("Overview")

    implicitHeight: 58
    radius: AppTheme.radiusMd

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: AppTheme.spacingMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: root.statusText
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeXs
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: root.todayCost
                color: AppTheme.accentColor
                font.pixelSize: AppTheme.fontSizeLg
                font.bold: true
                elide: Text.ElideRight
            }
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            color: AppTheme.surfaceBorder
        }

        ColumnLayout {
            Layout.preferredWidth: 96
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: qsTr("Focus")
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeXs
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: root.activeProviderName !== "" ? root.activeProviderName : qsTr("All providers")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeSm
                font.bold: true
                elide: Text.ElideRight
            }
        }
    }
}

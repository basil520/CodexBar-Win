import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property string providerId: ""
    property string providerName: providerId
    property color accentColor: AppTheme.providerBrandColor(providerId)
    property string todayText: ""
    property string periodText: ""
    property string tokenText: ""
    property bool expanded: false
    property var trendValues: []

    implicitHeight: expanded ? 142 : 86
    radius: AppTheme.radiusLg
    interactive: true
    selected: expanded

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 3
        radius: 1.5
        color: root.accentColor
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: AppTheme.spacingLg
        spacing: AppTheme.spacingLg

        ProviderAvatar {
            Layout.preferredWidth: AppTheme.avatarSizeList
            Layout.preferredHeight: AppTheme.avatarSizeList
            size: AppTheme.avatarSizeList
            providerId: root.providerId
            displayName: root.providerName
            brandColor: root.accentColor
            selected: root.expanded
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingXs

            Text {
                text: root.providerName
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeLg
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Text {
                text: root.tokenText
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            spacing: AppTheme.spacingXs

            Text {
                text: qsTr("Today")
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeXs
            }
            Text {
                text: root.todayText
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }
        }

        ColumnLayout {
            spacing: AppTheme.spacingXs

            Text {
                text: qsTr("30 days")
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeXs
            }
            Text {
                text: root.periodText
                color: root.accentColor
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }
        }
    }
}

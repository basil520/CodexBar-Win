import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property string title: ""
    property string subtitle: ""
    property bool loading: false
    property bool empty: false
    property string emptyText: qsTr("No chart data")
    default property alias content: chartSlot.data

    color: AppTheme.surfaceChart
    border.color: AppTheme.surfaceBorder
    radius: AppTheme.radiusLg
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: AppTheme.spacingSm

        RowLayout {
            Layout.fillWidth: true
            visible: root.title !== "" || root.subtitle !== ""
            spacing: AppTheme.spacingSm

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    visible: root.title !== ""
                    text: root.title
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: root.subtitle !== ""
                    text: root.subtitle
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeXs
                    elide: Text.ElideRight
                }
            }
        }

        Item {
            id: chartSlot
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !root.loading && !root.empty
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.loading || root.empty
            text: root.loading ? qsTr("Loading chart") : root.emptyText
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}

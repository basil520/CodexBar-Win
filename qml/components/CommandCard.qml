import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property string title: ""
    property string subtitle: ""
    property string statusText: ""
    property color accentColor: AppTheme.accentColor
    default property alias content: body.data

    implicitHeight: shell.implicitHeight + 28
    radius: AppTheme.radiusLg

    ColumnLayout {
        id: shell
        anchors.fill: parent
        anchors.margins: 14
        spacing: AppTheme.spacingMd

        RowLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd
            visible: root.title !== "" || root.statusText !== ""

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: root.title
                    visible: text !== ""
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeLg
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.subtitle
                    visible: text !== ""
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                }
            }

            StatusPill {
                visible: root.statusText !== ""
                text: root.statusText
                toneColor: root.accentColor
            }
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd
        }
    }
}

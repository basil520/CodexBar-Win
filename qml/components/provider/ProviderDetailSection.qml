import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."

Rectangle {
    id: root
    objectName: "providerDetailSection"

    property string title: ""
    property string subtitle: ""
    default property alias content: contentLayout.data

    Layout.fillWidth: true
    implicitHeight: outerLayout.implicitHeight + 28
    radius: AppTheme.radiusLg
    color: AppTheme.surfaceCard
    border.width: 1
    border.color: AppTheme.borderSubtle

    ColumnLayout {
        id: outerLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3
            visible: root.title !== "" || root.subtitle !== ""

            Label {
                Layout.fillWidth: true
                text: root.title
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
                visible: text !== ""
            }

            Label {
                Layout.fillWidth: true
                text: root.subtitle
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
                visible: text !== ""
            }
        }

        ColumnLayout {
            id: contentLayout
            Layout.fillWidth: true
            spacing: 10
        }
    }
}

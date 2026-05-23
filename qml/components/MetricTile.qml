import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property string label: ""
    property string value: ""
    property string detail: ""
    property color accentColor: AppTheme.accentColor

    implicitHeight: 84
    radius: AppTheme.radiusMd

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 3

        Label {
            Layout.fillWidth: true
            text: root.label
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            text: root.value
            color: root.accentColor
            font.pixelSize: 22
            minimumPixelSize: 13
            fontSizeMode: Text.HorizontalFit
            font.bold: true
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            text: root.detail
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeSm
            elide: Text.ElideRight
        }
    }
}

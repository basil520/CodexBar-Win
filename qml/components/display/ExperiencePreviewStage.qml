import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    id: root

    ColumnLayout {
        Layout.fillWidth: true
        spacing: AppTheme.spacingMd

        Label {
            Layout.fillWidth: true
            text: qsTr("Experience Preview Stage")
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("This preview uses the same provider, state, and chart primitives as the production UI.")
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        DisplayPreviewCard {
            Layout.fillWidth: true
        }
    }
}

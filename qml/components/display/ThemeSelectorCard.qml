import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    RowLayout {
        Layout.fillWidth: true
        spacing: AppTheme.spacingMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: qsTr("Theme")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Choose the overall color style.")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
            }
        }

        Components.SettingsComboBox {
            Layout.preferredWidth: 190
            model: [
                { value: 0, label: qsTr("Dark") },
                { value: 1, label: qsTr("Midnight Blue") },
                { value: 2, label: qsTr("Amethyst") }
            ]
            selectedValue: SettingsStore.theme
            onValueActivated: function(value) {
                SettingsStore.theme = value
            }
        }
    }
}

import QtQuick 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import ".."
import "../components"

SettingsPage {
    title: qsTr("Display")
    subtitle: qsTr("Tune how usage and tray state are presented.")

    SettingsGroupBox {
        RowLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: qsTr("Theme")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                }

                Text {
                    text: qsTr("Choose the overall color style.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                }
            }

            SettingsComboBox {
                Layout.preferredWidth: 180
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

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Merge Icons")
            subtitle: qsTr("Show a single combined tray icon for enabled providers.")
            checked: SettingsStore.mergeIcons
            onToggled: function(checked) {
                SettingsStore.mergeIcons = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Show Usage Amount Used")
            subtitle: qsTr("Use consumed percentage instead of remaining percentage.")
            checked: SettingsStore.usageBarsShowUsed
            onToggled: function(checked) {
                SettingsStore.usageBarsShowUsed = checked
            }
        }
    }

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Show Absolute Reset Times")
            subtitle: qsTr("Display exact reset times instead of relative wording.")
            checked: SettingsStore.resetTimesShowAbsolute
            onToggled: function(checked) {
                SettingsStore.resetTimesShowAbsolute = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Optional Credits and Extra Usage")
            subtitle: qsTr("Show additional provider-specific credit and usage fields.")
            checked: SettingsStore.showOptionalCreditsAndExtraUsage
            onToggled: function(checked) {
                SettingsStore.showOptionalCreditsAndExtraUsage = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Claude Peak Hours")
            subtitle: qsTr("Show peak hours indicator for Claude usage pricing.")
            checked: SettingsStore.claudePeakHoursEnabled
            onToggled: function(checked) {
                SettingsStore.claudePeakHoursEnabled = checked
            }
        }
    }
}

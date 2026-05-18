import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
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

        SettingsToggleRow {
            title: qsTr("Glass Effect")
            subtitle: qsTr("Use native Windows acrylic blur behind app windows.")
            checked: SettingsStore.glassEffectEnabled
            onToggled: function(checked) {
                SettingsStore.glassEffectEnabled = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            enabled: SettingsStore.glassEffectEnabled
            opacity: enabled ? 1.0 : 0.45
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingXs

                Label {
                    text: qsTr("Glass Opacity")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Label {
                    text: qsTr("Lower values make the glass more transparent.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Slider {
                Layout.preferredWidth: 220
                from: 10
                to: 85
                stepSize: 5
                snapMode: Slider.SnapAlways
                live: true
                value: SettingsStore.glassEffectOpacity
                onMoved: SettingsStore.glassEffectOpacity = Math.round(value)
            }

            Label {
                Layout.preferredWidth: 42
                text: SettingsStore.glassEffectOpacity + "%"
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignRight
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

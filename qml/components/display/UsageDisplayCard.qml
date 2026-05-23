import QtQuick 2.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    Components.SettingsToggleRow {
        title: qsTr("Show Absolute Reset Times")
        subtitle: qsTr("Display exact reset times instead of relative wording.")
        checked: SettingsStore.resetTimesShowAbsolute
        onToggled: function(checked) {
            SettingsStore.resetTimesShowAbsolute = checked
        }
    }

    Components.SettingsToggleRow {
        title: qsTr("Optional Credits and Extra Usage")
        subtitle: qsTr("Show additional provider-specific credit and usage fields.")
        checked: SettingsStore.showOptionalCreditsAndExtraUsage
        onToggled: function(checked) {
            SettingsStore.showOptionalCreditsAndExtraUsage = checked
        }
    }

    Components.SettingsToggleRow {
        title: qsTr("Claude Peak Hours")
        subtitle: qsTr("Show peak hours indicator for Claude usage pricing.")
        checked: SettingsStore.claudePeakHoursEnabled
        onToggled: function(checked) {
            SettingsStore.claudePeakHoursEnabled = checked
        }
    }
}

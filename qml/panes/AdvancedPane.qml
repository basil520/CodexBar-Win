import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."
import "../components"

SettingsPage {
    title: qsTr("Advanced")
    subtitle: qsTr("Refresh cadence, status polling, and diagnostic controls.")

    SettingsGroupBox {
        SettingsComboRow {
            title: qsTr("Refresh Frequency")
            subtitle: qsTr("How often enabled providers refresh automatically.")
            model: [
                { value: 1, label: qsTr("Every minute") },
                { value: 5, label: qsTr("Every 5 minutes") },
                { value: 15, label: qsTr("Every 15 minutes") },
                { value: 30, label: qsTr("Every 30 minutes") },
                { value: 0, label: qsTr("Manual only") }
            ]
            selectedValue: SettingsStore.refreshFrequency
            onValueActivated: function(value) {
                SettingsStore.refreshFrequency = value
            }
        }

        SettingsToggleRow {
            title: qsTr("Check Provider Status")
            subtitle: qsTr("Poll statuspage-style endpoints for provider health.")
            checked: SettingsStore.statusChecksEnabled
            onToggled: function(checked) {
                SettingsStore.statusChecksEnabled = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Cookies Import")
            subtitle: qsTr("Enable Browser Session Bridge import UI and extension connections.")
            checked: SettingsStore.browserSessionBridgeEnabled
            onToggled: function(checked) {
                SettingsStore.browserSessionBridgeEnabled = checked
            }
        }
    }

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Debug Mode")
            subtitle: qsTr("Show debug menus and keep more diagnostic information visible.")
            checked: SettingsStore.debugMenuEnabled
            onToggled: function(checked) {
                SettingsStore.debugMenuEnabled = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Codex Verbose Logging")
            subtitle: qsTr("Log detailed strategy-level diagnostics during Codex fetches.")
            checked: SettingsStore.codexVerboseLogging
            onToggled: function(checked) {
                SettingsStore.codexVerboseLogging = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Web Dashboard Debug Dump")
            subtitle: qsTr("Save raw HTML from web dashboard fetches for troubleshooting.")
            checked: SettingsStore.codexWebDebugDumpHTML
            onToggled: function(checked) {
                SettingsStore.codexWebDebugDumpHTML = checked
            }
        }
    }

    Loader {
        Layout.fillWidth: true
        active: SettingsStore.browserSessionBridgeEnabled
        visible: active
        sourceComponent: BrowserSessionInstallGuide {
            compact: false
        }
    }
}

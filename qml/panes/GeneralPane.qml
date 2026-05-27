import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."
import "../components"

SettingsPage {
    id: root
    title: qsTr("General")
    subtitle: qsTr("Startup, updates, language, and notification behavior.")

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Launch at Login")
            subtitle: qsTr("Start CodexBar automatically when Windows starts.")
            checked: SettingsStore.launchAtLogin
            onToggled: function(checked) {
                SettingsStore.launchAtLogin = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Check for Updates")
            subtitle: qsTr("Look for new app versions in the background.")
            checked: SettingsStore.checkForUpdates
            onToggled: function(checked) {
                SettingsStore.checkForUpdates = checked
            }
        }
    }

    SettingsGroupBox {
        SettingsComboRow {
            title: qsTr("Language")
            subtitle: qsTr("Apply interface text immediately where supported.")
            model: [
                { value: "en", label: qsTr("English") },
                { value: "zh_CN", label: qsTr("Chinese") }
            ]
            selectedValue: SettingsStore.language
            onValueActivated: function(value) {
                SettingsStore.language = value
            }
        }

        SettingsToggleRow {
            title: qsTr("Session Quota Notifications")
            subtitle: qsTr("Warn when provider session quota becomes constrained.")
            checked: SettingsStore.sessionQuotaNotificationsEnabled
            onToggled: function(checked) {
                SettingsStore.sessionQuotaNotificationsEnabled = checked
            }
        }
    }
}

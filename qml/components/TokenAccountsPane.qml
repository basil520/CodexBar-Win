import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ColumnLayout {
    id: root
    property string providerId: ""
    property var descriptor: ({})
    property var accounts: []
    property string defaultAccountId: ""
    property bool busy: false
    readonly property bool compactLayout: width > 0 && width < 430

    signal addAccount(string displayName, int sourceMode, string apiKey)
    signal removeAccount(string accountId)
    signal setDefaultAccount(string accountId)
    signal setSourceMode(string accountId, int sourceMode)
    signal setVisibility(string accountId, int visibility)

    spacing: 8

    function tokenAccountConfig() {
        return descriptor && descriptor.tokenAccount ? descriptor.tokenAccount : ({})
    }

    function requiresApiKey() {
        var creds = tokenAccountConfig().requiredCredentialTypes || []
        for (var i = 0; i < creds.length; i++) {
            if (creds[i] === "apiKey") return true
        }
        return false
    }

    function modeValue(mode) {
        if (mode === "web") return 1
        if (mode === "cli") return 2
        if (mode === "oauth") return 3
        if (mode === "api") return 4
        return 0
    }

    function modeName(value) {
        if (value === 1) return "web"
        if (value === 2) return "cli"
        if (value === 3) return "oauth"
        if (value === 4) return "api"
        return "auto"
    }

    function visibilityValue(visibility) {
        if (visibility === "hidden") return 1
        if (visibility === "archived") return 2
        return 0
    }

    function visibilityOptions() {
        return [
            { value: 0, label: qsTr("Visible") },
            { value: 1, label: qsTr("Hidden") }
        ]
    }

    function sourceModeOptions() {
        var modes = descriptor && descriptor.sourceModes ? descriptor.sourceModes : []
        if (modes.length === 0) modes = ["auto"]
        var result = []
        for (var i = 0; i < modes.length; i++) {
            var mode = modes[i]
            result.push({ value: modeValue(mode), label: mode.toUpperCase() })
        }
        return result
    }

    function selectedModeFor(account) {
        return modeValue(account && account.sourceMode ? account.sourceMode : "auto")
    }

    function resetAddFields() {
        accountNameField.text = ""
        accountApiKeyField.text = ""
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: accountNameField
                objectName: "accountNameField"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                placeholderText: qsTr("Account name")
                placeholderTextColor: AppTheme.textTertiary
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeSm
                enabled: !root.busy
                background: Rectangle {
                    radius: 6
                    color: AppTheme.surfaceControl
                    border.width: 1
                    border.color: parent.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                }
            }

            SettingsComboBox {
                id: addSourceMode
                Layout.preferredWidth: 112
                Layout.maximumWidth: 128
                model: root.sourceModeOptions()
                selectedValue: model.length > 0 ? model[0].value : 0
                enabled: !root.busy
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: accountApiKeyField
                objectName: "accountApiKeyField"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                visible: root.requiresApiKey()
                placeholderText: qsTr("API key")
                placeholderTextColor: AppTheme.textTertiary
                echoMode: TextInput.Password
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeSm
                enabled: !root.busy
                background: Rectangle {
                    radius: 6
                    color: AppTheme.surfaceControl
                    border.width: 1
                    border.color: parent.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                }
            }

            Item {
                Layout.fillWidth: true
                visible: !root.requiresApiKey()
            }

            SettingsButton {
                objectName: "addAccountButton"
                compact: root.compactLayout
                text: qsTr("Add Account")
                primary: true
                enabled: !root.busy
                    && accountNameField.text.trim() !== ""
                    && (!root.requiresApiKey() || accountApiKeyField.text.trim() !== "")
                onClicked: {
                    root.addAccount(accountNameField.text.trim(),
                                    addSourceMode.selectedValue,
                                    accountApiKeyField.text.trim())
                    root.resetAddFields()
                }
            }
        }
    }

    FeedbackBanner {
        Layout.fillWidth: true
        visible: !root.accounts || root.accounts.length === 0
        status: "info"
        title: qsTr("No accounts configured.")
        message: qsTr("Add an account when this provider needs separate token or API-key tracking.")
        compact: true
    }

    Repeater {
        model: root.accounts || []

        DisclosureRow {
            Layout.fillWidth: true
            title: modelData.displayName || modelData.accountId
            subtitle: modelData.visibility === "hidden"
                ? qsTr("Hidden")
                : (modelData.sourceMode || "auto").toUpperCase()
            expanded: rowExpanded
            canExpand: true
            property bool rowExpanded: modelData.accountId === root.defaultAccountId || root.accounts.length === 1
            onToggled: rowExpanded = !rowExpanded

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    visible: modelData.accountId === root.defaultAccountId
                    text: qsTr("Default")
                    color: AppTheme.accentColor
                    font.pixelSize: 10
                    font.bold: true
                }

                Item { Layout.fillWidth: true }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.compactLayout ? 2 : 4
                columnSpacing: 8
                rowSpacing: 8

                SettingsComboBox {
                    Layout.fillWidth: root.compactLayout
                    Layout.preferredWidth: 104
                    Layout.minimumWidth: 0
                    model: root.sourceModeOptions()
                    selectedValue: root.selectedModeFor(modelData)
                    enabled: !root.busy
                    onValueActivated: function(value) {
                        root.setSourceMode(modelData.accountId, value)
                    }
                }

                SettingsComboBox {
                    Layout.fillWidth: root.compactLayout
                    Layout.preferredWidth: 104
                    Layout.minimumWidth: 0
                    model: root.visibilityOptions()
                    selectedValue: root.visibilityValue(modelData.visibility)
                    enabled: !root.busy
                    onValueActivated: function(value) {
                        root.setVisibility(modelData.accountId, value)
                    }
                }

                SettingsButton {
                    compact: root.compactLayout
                    text: qsTr("Use")
                    enabled: !root.busy && modelData.accountId !== root.defaultAccountId
                    onClicked: root.setDefaultAccount(modelData.accountId)
                }

                SettingsButton {
                    compact: root.compactLayout
                    text: qsTr("Remove")
                    danger: true
                    enabled: !root.busy
                    onClicked: root.removeAccount(modelData.accountId)
                }
            }
        }
    }
}

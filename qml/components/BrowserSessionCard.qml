import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

SettingsGroupBox {
    id: root
    property string providerId: ""
    property int refreshKey: 0

    function binding() {
        refreshKey
        return BridgeViewModel.bindingForProvider(providerId) || ({})
    }

    function options() {
        refreshKey
        return BridgeViewModel.bindingOptions(providerId)
    }

    function selectedBindingId() {
        var b = binding()
        return b && b.preferredBindingId ? b.preferredBindingId : ""
    }

    function selectedBindingLabel() {
        var id = selectedBindingId()
        if (id === "") return qsTr("Auto")
        var opts = options()
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].bindingId === id) return opts[i].label
        }
        return id
    }

    function canImport() {
        refreshKey
        return BridgeViewModel.serverRunning
            && connectedOptionCount() > 0
            && !BridgeViewModel.importBusy(providerId)
    }

    function connectedOptionCount() {
        refreshKey
        var opts = options()
        var count = 0
        for (var i = 0; i < opts.length; i++) {
            if (opts[i].connected) count++
        }
        return count
    }

    function unavailableReason() {
        refreshKey
        if (BridgeViewModel.importBusy(root.providerId) || root.importError() !== "")
            return ""
        if (!BridgeViewModel.serverRunning)
            return qsTr("Bridge server is not running yet.")
        if (BridgeViewModel.connectedClients.length === 0)
            return ""
        if (options().length === 0 && root.providerId === "codex")
            return qsTr("Connected extension is outdated for Codex import. Click Prepare Extension, then reload the unpacked extension in Edge/Chrome.")
        if (options().length === 0)
            return qsTr("No compatible browser profile supports this provider yet. If the browser is connected, click Prepare Extension and reload the unpacked extension.")
        if (connectedOptionCount() === 0)
            return qsTr("The selected provider has no connected compatible browser profile.")
        return ""
    }

    function importError() {
        refreshKey
        return BridgeViewModel.importError(root.providerId) || ""
    }

    Connections {
        target: BridgeViewModel
        function onConnectedClientsChanged() { root.refreshKey++ }
        function onServerRunningChanged() { root.refreshKey++ }
        function onExtensionInstalledChanged() { root.refreshKey++ }
        function onProviderBindingChanged(providerId) {
            if (providerId === root.providerId) root.refreshKey++
        }
        function onImportBusyChanged(providerId) {
            if (providerId === root.providerId) root.refreshKey++
        }
        function onImportFeedbackChanged(providerId) {
            if (providerId === root.providerId) root.refreshKey++
        }
    }

    BrowserSessionBindingDialog {
        id: bindingDialog
        providerId: root.providerId
        selectedBindingId: root.selectedBindingId()
        options: root.options()
        onBindingSelected: function(bindingId) {
            BridgeViewModel.setBindingForProvider(root.providerId, bindingId)
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Browser Session Bridge")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }

            Rectangle {
                width: AppTheme.statusDotSize
                height: AppTheme.statusDotSize
                radius: width / 2
                color: root.canImport() ? AppTheme.statusOk : AppTheme.statusUnknown
            }
        }

        BrowserSessionInstallGuide {
            Layout.fillWidth: true
            compact: true
            visible: !BridgeViewModel.extensionInstalled || BridgeViewModel.connectedClients.length === 0
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Source Profile")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeSm
                Layout.preferredWidth: 120
            }

            SettingsButton {
                Layout.fillWidth: true
                text: root.selectedBindingLabel()
                enabled: root.options().length > 0
                onClicked: bindingDialog.open()
            }
        }

        SettingsToggleRow {
            title: qsTr("Refresh after import")
            subtitle: qsTr("After a browser import succeeds, refresh this provider automatically.")
            checked: BridgeViewModel.autoSync(root.providerId)
            onToggled: function(checked) {
                BridgeViewModel.setAutoSync(root.providerId, checked)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: qsTr("Last Import")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeSm
                Layout.preferredWidth: 120
            }

            Label {
                text: root.refreshKey >= 0 ? (BridgeViewModel.lastImportTime(root.providerId) || qsTr("Never")) : qsTr("Never")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                Layout.fillWidth: true
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.refreshKey >= 0 && BridgeViewModel.importBusy(root.providerId)
            text: root.refreshKey >= 0 ? qsTr("Importing browser session from the selected profile...") : ""
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            visible: root.refreshKey >= 0 && !BridgeViewModel.importBusy(root.providerId) && root.importError() !== ""
            text: root.refreshKey >= 0 ? root.importError() : ""
            color: AppTheme.statusOutage
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            visible: root.unavailableReason() !== ""
            text: root.unavailableReason()
            color: AppTheme.statusOutage
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        SettingsButton {
            text: root.refreshKey >= 0 && BridgeViewModel.importBusy(root.providerId) ? qsTr("Importing...") : qsTr("Import Now")
            primary: true
            enabled: root.canImport()
            onClicked: BridgeViewModel.requestImport(root.providerId)
        }
    }
}

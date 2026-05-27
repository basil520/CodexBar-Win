import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."
import "provider" as ProviderPanels

ScrollView {
    id: root
    property string providerId: ""
    property var descriptor: null
    property string detailState: "idle"
    property var connectionTest: ({"state": "idle"})
    property var providerStatus: ({"state": "unknown"})
    property var providerError: ""
    property var usageSnapshot: null
    property var codexAccountState: ({})
    property var codexProjection: ({})
    property var tokenAccounts: []
    property string defaultTokenAccountId: ""
    property var tokenAccountOperationState: ({})
    property bool detailsExpanded: false

    property color brandColor: descriptor && descriptor.brandColor ? descriptor.brandColor : AppTheme.accentColor
    property string connectionState: connectionTest && connectionTest.state ? connectionTest.state : "idle"
    property string connectionMessage: connectionTest && connectionTest.message ? connectionTest.message : ""

    signal testConnectionRequested()
    signal refreshRequested()
    signal toggleEnabled(bool enabled)
    signal settingChanged(string key, var value)
    signal secretSaveRequested(string key, string value)
    signal secretClearRequested(string key)
    signal addTokenAccountRequested(string displayName, int sourceMode, string apiKey)
    signal removeTokenAccountRequested(string accountId)
    signal setDefaultTokenAccountRequested(string accountId)
    signal setTokenAccountSourceModeRequested(string accountId, int sourceMode)
    signal setTokenAccountVisibilityRequested(string accountId, int visibility)
    signal setCodexActiveAccountRequested(string accountId)
    signal addCodexAccountRequested()
    signal cancelCodexAuthenticationRequested()
    signal removeCodexAccountRequested(string accountId)
    signal reauthenticateCodexAccountRequested(string accountId)
    signal promoteCodexAccountRequested(string accountId)

    clip: true
    contentWidth: availableWidth
    contentHeight: body.implicitHeight + 48

    ScrollBar.vertical: ElegantScrollBar {
        flickable: root.contentItem
    }
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    onProviderIdChanged: {
        detailsExpanded = false
    }
    property bool tokenAccountBusy: {
        var byProvider = tokenAccountOperationState && tokenAccountOperationState.pendingByProvider
            ? tokenAccountOperationState.pendingByProvider : ({})
        return byProvider[root.providerId] === true
    }

    function statusText(state) {
        if (state === "ok") return qsTr("Operational")
        if (state === "degraded") return qsTr("Degraded")
        if (state === "outage") return qsTr("Outage")
        return qsTr("Unknown")
    }

    function statusColor(state) {
        if (state === "ok" || state === "succeeded") return AppTheme.statusOk
        if (state === "degraded" || state === "testing") return AppTheme.statusDegraded
        if (state === "outage" || state === "failed") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    function statusOpenURL() {
        if (!root.descriptor) return ""
        return root.descriptor.statusURL || ""
    }

    function connectionTitle() {
        if (connectionState === "testing") return qsTr("Testing connection")
        if (connectionState === "succeeded") return qsTr("Connection OK")
        if (connectionState === "failed") return qsTr("Connection failed")
        return qsTr("Not tested")
    }

    function hasUsage() {
        return usageSnapshot !== null
            && (usageSnapshot.primary !== undefined
                || usageSnapshot.secondary !== undefined
                || usageSnapshot.tertiary !== undefined)
    }

    function hasUsageSummary() {
        return usageSnapshot !== null
            && usageSnapshot.loginMethod !== undefined
            && usageSnapshot.loginMethod !== ""
    }

    function tokenAccountConfig() {
        return descriptor && descriptor.tokenAccount ? descriptor.tokenAccount : ({})
    }

    function supportsTokenAccounts() {
        return providerId !== "codex"
            && (tokenAccountConfig().supportsMultipleAccounts === true
                || (tokenAccounts && tokenAccounts.length > 0))
    }

    property bool isDetailProvider: root.providerId === "deepseek"
        || root.providerId === "warp"
        || root.providerId === "kilo"
        || root.providerId === "abacus"
        || root.providerId === "codebuff"

    function durationLabel(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return ""
        if (value < 1000) return Math.round(value) + " ms"
        return (value / 1000.0).toFixed(1) + " s"
    }

    function timeLabel(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return ""
        return Qt.formatDateTime(new Date(value), "yyyy-MM-dd hh:mm:ss")
    }

    Item {
        width: root.availableWidth
        height: root.contentHeight

        ProviderPanels.ProviderWorkbench {
            id: body
            x: 24
            y: 22
            width: Math.max(0, Math.min(root.availableWidth - 48, 760))
            providerId: root.providerId
            state: root.connectionState
            spacing: 12

            ProviderDetailHero {
                providerId: root.providerId
                descriptor: root.descriptor
                providerStatus: root.providerStatus
                providerError: root.providerError
                brandColor: root.brandColor
                onDashboardRequested: function(url) {
                    AppController.openExternalUrl(url)
                }
                onStatusRequested: function(url) {
                    AppController.openExternalUrl(url)
                }
                onRefreshRequested: root.refreshRequested()
                onEnabledToggled: function(enabled) {
                    root.toggleEnabled(enabled)
                }
            }

            ProviderPanels.ProviderStatusNarrative {
                Layout.fillWidth: true
                visible: root.providerError !== "" || root.connectionState !== "idle"
                state: root.providerError !== "" ? "error"
                    : root.connectionState === "failed" ? "error"
                    : root.connectionState === "testing" ? "busy"
                    : root.connectionState === "succeeded" ? "success"
                    : "idle"
                title: root.providerError !== "" ? qsTr("Provider needs attention") : root.connectionTitle()
                reason: root.providerError !== "" ? root.providerError : root.connectionMessage
                nextStep: state === "error"
                    ? qsTr("Open diagnostics, retry the connection, or refresh the configured session source.")
                    : ""
                actionText: state === "error" ? qsTr("Retry") : ""
                onActionRequested: root.testConnectionRequested()
            }

            SettingsGroupBox {
                visible: root.hasUsage() || root.hasUsageSummary()

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SectionTitle { text: qsTr("Usage") }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        visible: root.hasUsageSummary()

                        Label {
                            Layout.preferredWidth: 116
                            text: qsTr("Account")
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fontSizeSm
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: root.usageSnapshot ? root.usageSnapshot.loginMethod : ""
                            color: AppTheme.textPrimary
                            font.pixelSize: AppTheme.fontSizeSm
                            elide: Text.ElideRight
                        }
                    }

                    UsageMetricRow {
                        label: root.isDetailProvider ? qsTr("Balance") : (root.descriptor ? root.descriptor.sessionLabel : qsTr("Session"))
                        metric: root.usageSnapshot ? root.usageSnapshot.primary : null
                        tintColor: root.brandColor
                    }

                    UsageMetricRow {
                        visible: !root.isDetailProvider
                        label: root.descriptor ? root.descriptor.weeklyLabel : qsTr("Weekly")
                        metric: root.usageSnapshot ? root.usageSnapshot.secondary : null
                        tintColor: root.brandColor
                    }

                    UsageMetricRow {
                        visible: !root.isDetailProvider
                            && root.usageSnapshot
                            && root.usageSnapshot.hasTertiary === true
                        label: root.descriptor ? root.descriptor.opusLabel || qsTr("Monthly") : qsTr("Monthly")
                        metric: root.usageSnapshot ? root.usageSnapshot.tertiary : null
                        tintColor: root.brandColor
                    }

                    // Detail text line for detail-only providers (balance/credit info)
                    Label {
                        visible: root.isDetailProvider
                            && root.usageSnapshot
                            && root.usageSnapshot.detail !== undefined
                        Layout.fillWidth: true
                        text: root.usageSnapshot ? root.usageSnapshot.detail : ""
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        wrapMode: Text.WordWrap
                    }
                }
            }

            ProviderPanels.ProviderConnectionPanel {
                connectionTest: root.connectionTest
                connectionState: root.connectionState
                connectionMessage: root.connectionMessage
                onTestConnectionRequested: root.testConnectionRequested()
            }

            SettingsGroupBox {
                visible: root.descriptor
                    && root.descriptor.settingsFields
                    && root.descriptor.settingsFields.length > 0

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SectionTitle { text: qsTr("Settings") }

                    Repeater {
                        model: root.descriptor ? root.descriptor.settingsFields : []

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Label {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    text: modelData.label
                                    color: AppTheme.textPrimary
                                    font.pixelSize: AppTheme.fontSizeSm
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                StatusPill {
                                    visible: modelData.sensitive === true
                                    text: qsTr("Secret")
                                    toneColor: AppTheme.accentColor
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modelData.helpText || ""
                                color: AppTheme.textTertiary
                                font.pixelSize: AppTheme.fontSizeSm
                                wrapMode: Text.WordWrap
                                visible: text !== ""
                            }

                            Loader {
                                Layout.fillWidth: true
                                sourceComponent: {
                                    if (modelData.type === "secret") return secretFieldComponent
                                    if (modelData.type === "picker") return pickerComponent
                                    if (modelData.type === "bool" || modelData.type === "boolean") return boolFieldComponent
                                    if (modelData.multiline === true) return textAreaComponent
                                    return textFieldComponent
                                }

                                Component {
                                    id: textFieldComponent
                                    TextField {
                                        Layout.fillWidth: true
                                        color: AppTheme.textPrimary
                                        font.pixelSize: AppTheme.fontSizeMd
                                        text: modelData.value || ""
                                        property string committedText: modelData.value || ""
                                        placeholderText: modelData.placeholder || ""
                                        placeholderTextColor: AppTheme.textTertiary

                                        function commitIfChanged() {
                                            if (text === committedText) return
                                            committedText = text
                                            root.settingChanged(modelData.key, text)
                                        }

                                        background: Rectangle {
                                            radius: 6
                                            color: AppTheme.surfaceControl
                                            border.color: parent.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                                            border.width: 1
                                        }

                                        onAccepted: commitIfChanged()
                                        onEditingFinished: commitIfChanged()
                                    }
                                }

                                Component {
                                    id: textAreaComponent
                                    TextArea {
                                        Layout.fillWidth: true
                                        implicitHeight: 88
                                        color: AppTheme.textPrimary
                                        font.pixelSize: AppTheme.fontSizeMd
                                        text: modelData.value || ""
                                        placeholderText: modelData.placeholder || ""
                                        placeholderTextColor: AppTheme.textTertiary
                                        wrapMode: TextEdit.Wrap

                                        background: Rectangle {
                                            radius: 6
                                            color: AppTheme.surfaceControl
                                            border.color: parent.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                                            border.width: 1
                                        }

                                        onActiveFocusChanged: {
                                            if (!activeFocus) root.settingChanged(modelData.key, text)
                                        }
                                    }
                                }

                                Component {
                                    id: secretFieldComponent
                                    SecretInput {
                                        Layout.fillWidth: true
                                        placeholder: modelData.placeholder || ""
                                        providerId: root.providerId
                                        fieldKey: modelData.key
                                        secretStatus: modelData.secretStatus || ({"configured": false, "source": "none"})
                                        onSaveRequested: function(value) {
                                            root.secretSaveRequested(modelData.key, value)
                                        }
                                        onClearRequested: function() {
                                            root.secretClearRequested(modelData.key)
                                        }
                                    }
                                }

                                Component {
                                    id: pickerComponent
                                    SettingsComboBox {
                                        Layout.fillWidth: true
                                        model: modelData.options || []
                                        selectedValue: modelData.value !== undefined ? modelData.value : modelData.defaultValue
                                        onValueActivated: function(value) {
                                            root.settingChanged(modelData.key, value)
                                        }
                                    }
                                }

                                Component {
                                    id: boolFieldComponent
                                    RowLayout {
                                        Item { Layout.fillWidth: true }
                                        SettingsSwitch {
                                            accessibleName: modelData.label || modelData.key || qsTr("Setting")
                                            checked: modelData.value || false
                                            onToggled: function(checked) {
                                                root.settingChanged(modelData.key, checked)
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                                color: AppTheme.surfaceBorder
                                opacity: 0.55
                                visible: index < (root.descriptor.settingsFields.length - 1)
                            }
                        }
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                active: false
                visible: false
            }

            ProviderPanels.ProviderBrowserSessionPanel {
                Layout.fillWidth: true
                providerId: root.providerId
            }

            SettingsGroupBox {
                visible: root.supportsTokenAccounts()

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SectionTitle { text: qsTr("Token Accounts") }

                    TokenAccountsPane {
                        Layout.fillWidth: true
                        providerId: root.providerId
                        descriptor: root.descriptor || ({})
                        accounts: root.tokenAccounts || []
                        defaultAccountId: root.defaultTokenAccountId
                        busy: root.tokenAccountBusy

                        onAddAccount: function(displayName, sourceMode, apiKey) {
                            root.addTokenAccountRequested(displayName, sourceMode, apiKey)
                        }
                        onRemoveAccount: function(accountId) {
                            root.removeTokenAccountRequested(accountId)
                        }
                        onSetDefaultAccount: function(accountId) {
                            root.setDefaultTokenAccountRequested(accountId)
                        }
                        onSetSourceMode: function(accountId, sourceMode) {
                            root.setTokenAccountSourceModeRequested(accountId, sourceMode)
                        }
                        onSetVisibility: function(accountId, visibility) {
                            root.setTokenAccountVisibilityRequested(accountId, visibility)
                        }
                    }
                }
            }

            SettingsGroupBox {
                visible: root.providerId === "codex"

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SectionTitle { text: qsTr("Account Management") }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Manage multiple Codex accounts. Switch between accounts to track usage separately.")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        wrapMode: Text.WordWrap
                    }

                    CodexAccountsPane {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 200
                        Layout.preferredHeight: 280
                        visible: root.providerId === "codex"

                        accounts: root.codexAccountState.accounts || []
                        activeAccountID: root.codexAccountState.activeAccountID || ""
                        isAuthenticating: root.codexAccountState.isAuthenticating || false
                        isRemoving: root.codexAccountState.isRemoving || false
                        authenticatingAccountID: root.codexAccountState.authenticatingAccountID || ""
                        removingAccountID: root.codexAccountState.removingAccountID || ""
                        hasUnreadableStore: root.codexAccountState.hasUnreadableStore || false
                        authState: root.codexAccountState.authState || "idle"
                        authMessage: root.codexAccountState.authMessage || ""
                        authError: root.codexAccountState.authError || ""
                        verificationUri: root.codexAccountState.verificationUri || ""
                        userCode: root.codexAccountState.userCode || ""

                        onSetActiveAccount: function(accountID) {
                            root.setCodexActiveAccountRequested(accountID)
                        }
                        onAddAccount: function() {
                            root.addCodexAccountRequested()
                        }
                        onCancelAuthentication: function() {
                            root.cancelCodexAuthenticationRequested()
                        }
                        onOpenVerificationUrl: function(url) {
                            AppController.openExternalUrl(url)
                        }
                        onCopyText: function(text) {
                            AppController.copyText(text)
                        }
                        onRemoveAccount: function(accountID) {
                            root.removeCodexAccountRequested(accountID)
                        }
                        onReauthenticateAccount: function(accountID) {
                            root.reauthenticateCodexAccountRequested(accountID)
                        }
                        onPromoteAccount: function(accountID) {
                            root.promoteCodexAccountRequested(accountID)
                        }
                    }
                }
            }

            SettingsGroupBox {
                visible: root.providerId === "codex"

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    SectionTitle { text: qsTr("Usage Projection") }

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Projected rate lanes and credits from the current active account.")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        wrapMode: Text.WordWrap
                    }

                    // Plan utilization lanes (session / weekly rates)
                    Repeater {
                        model: root.codexProjection && root.codexProjection.planUtilizationLanes
                            ? root.codexProjection.planUtilizationLanes
                            : []

                        Rectangle {
                            Layout.fillWidth: true
                            height: 36
                            color: root.glassEffectActive
                                ? root.colorWithAlpha(AppTheme.bgTertiary, Math.max(0.35, root.glassOpacity * 0.65))
                                : AppTheme.bgTertiary
                            radius: 6

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Label {
                                    Layout.preferredWidth: 80
                                    text: model.modelData && model.modelData.role === 0
                                        ? qsTr("Session")
                                        : qsTr("Weekly")
                                    color: AppTheme.textSecondary
                                    font.pixelSize: AppTheme.fontSizeSm
                                }

                                UsageProgressBar {
                                    Layout.fillWidth: true
                                    value: model.modelData
                                        ? (model.modelData.remainingPercent || 0)
                                        : 0
                                    tintColor: {
                                        var r = model.modelData ? (model.modelData.remainingPercent || 100) : 100
                                        return r < 20 ? AppTheme.statusOutage : AppTheme.accentColor
                                    }
                                }

                                Label {
                                    Layout.preferredWidth: 44
                                    text: {
                                        var p = model.modelData ? (model.modelData.remainingPercent || 0) : 0
                                        return Math.round(p) + "%"
                                    }
                                    color: AppTheme.textPrimary
                                    font.pixelSize: AppTheme.fontSizeSm
                                    horizontalAlignment: Text.AlignRight
                                }
                            }
                        }
                    }

                    // Credits balance
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        visible: root.codexProjection && root.codexProjection.hasCredits

                        Label {
                            Layout.preferredWidth: 80
                            text: qsTr("Credits")
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fontSizeSm
                        }

                        Item { Layout.fillWidth: true }

                        Label {
                            text: "$" + Number(root.codexProjection.creditsRemaining || 0).toFixed(2)
                            color: AppTheme.textPrimary
                            font.pixelSize: AppTheme.fontSizeLg
                            font.bold: true
                        }
                    }

                    // Buy Credits button (when rate lane exhausted)
                    ActionButton {
                        Layout.fillWidth: true
                        visible: root.codexProjection
                            && root.codexProjection.canShowBuyCredits
                            && root.codexProjection.hasExhaustedRateLane
                        text: qsTr("Buy More Credits")
                        variant: "primary"
                        onClicked: root.descriptor && root.descriptor.dashboardURL
                            ? AppController.openExternalUrl(root.descriptor.dashboardURL)
                            : {}
                    }

                    // Supplemental metrics
                    ColumnLayout {
                        visible: root.codexProjection
                            && root.codexProjection.supplementalMetrics
                            && root.codexProjection.supplementalMetrics.length > 0
                        spacing: 4

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: AppTheme.surfaceBorder
                        }

                        Label {
                            text: qsTr("Supplemental Metrics")
                            color: AppTheme.textTertiary
                            font.pixelSize: AppTheme.fontSizeSm
                            font.bold: true
                        }

                        Label {
                            Layout.fillWidth: true
                            text: {
                                var m = root.codexProjection.supplementalMetrics || []
                                var names = []
                                for (var i = 0; i < m.length; i++) {
                                    if (m[i] === 0) names.push(qsTr("Code Review"))
                                    else names.push(qsTr("Metric #") + m[i])
                                }
                                return names.join(", ")
                            }
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fontSizeSm
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            ProviderPanels.ProviderDiagnosticsPanel {
                Layout.fillWidth: true
                providerId: root.providerId
                events: [
                    {
                        severity: root.connectionState === "failed" ? "error"
                            : root.connectionState === "succeeded" ? "success"
                            : "info",
                        title: root.connectionTitle(),
                        message: root.connectionMessage,
                        createdAt: root.timeLabel(root.connectionTest ? root.connectionTest.finishedAt : 0)
                    },
                    {
                        severity: root.providerError !== "" ? "error" : "info",
                        title: root.providerError !== "" ? qsTr("Provider error") : qsTr("Provider configuration ready"),
                        message: root.providerError,
                        createdAt: ""
                    }
                ]
            }

            ErrorNotice {
                visible: root.providerError !== ""
                providerId: root.providerId
                title: qsTr("Last Provider Error")
                message: root.providerError
                detail: root.providerError
                density: "diagnostic"
                severity: "error"
                onCopyRequested: function(text) {
                    AppController.copyWithFeedback(text)
                }
            }
        }
    }

    component SectionTitle: Label {
        color: AppTheme.textPrimary
        font.pixelSize: AppTheme.fontSizeMd
        font.bold: true
        Layout.fillWidth: true
    }

    component UsageMetricRow: RowLayout {
        id: usageMetricRow
        property string label: ""
        property var metric: null
        property color tintColor: AppTheme.accentColor
        property double percent: metric && metric.percent !== undefined ? metric.percent : 0
        property string detail: metric && metric.resetDescription !== undefined ? metric.resetDescription : ""

        Layout.fillWidth: true
        Layout.preferredHeight: detail !== "" ? 48 : 36
        spacing: 10
        visible: metric !== null && metric !== undefined

        Label {
            Layout.preferredWidth: 116
            text: parent.label
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            elide: Text.ElideRight
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            UsageProgressBar {
                Layout.fillWidth: true
                value: usageMetricRow.percent
                tintColor: usageMetricRow.tintColor
            }

            Label {
                Layout.fillWidth: true
                visible: usageMetricRow.detail !== ""
                text: usageMetricRow.detail
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeXs
                elide: Text.ElideRight
            }
        }

        Label {
            Layout.preferredWidth: 48
            text: parent.percent.toFixed(0) + "%"
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignRight
        }
    }
}

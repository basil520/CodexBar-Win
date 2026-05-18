import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import "components" as Components

Rectangle {
    id: root
    width: 360
    height: 720
    color: windowBackgroundColor
    radius: glassEffectActive ? 0 : 12
    clip: !glassEffectActive
    antialiasing: true
    border.color: AppTheme.surfaceBorder
    border.width: 1

    readonly property bool glassEffectActive: SettingsStore.glassEffectEnabled
    readonly property color windowBackgroundColor: AppTheme.surfaceWindow
    readonly property color titleBarBackgroundColor: glassEffectActive ? "transparent" : AppTheme.surfaceTitleBar
    readonly property color cardBackgroundColor: AppTheme.surfaceCard

    property var costData: TrayViewModel.costData
    property var displayCostData: TrayViewModel.displayCostData
    property bool isRefreshing: TrayViewModel.isRefreshing
    property bool costExpanded: false
    property var providerCostRows: []
    property var expandedCards: ({})
    property int rev: LanguageManager.translationRevision
    property int refreshStartTime: 0
    property string refreshDuration: ""

    // Phase 2: Provider selection state
    property string selectedProviderID: TrayViewModel.selectedProviderID

    // Phase 3: Codex account state
    property var codexAccountState: TrayViewModel.codexAccountState || ({})

    // Phase 3: Codex accounts (derived from codexAccountState)
    property var codexAccounts: root.codexAccountState && root.codexAccountState.accounts ? root.codexAccountState.accounts : []

    Components.AcrylicBackdrop {
        anchors.fill: parent
        tint: AppTheme.bgPrimary
    }

    Component.onCompleted: TrayViewModel.requestCostUsageViewData()
    onCostExpandedChanged: refreshProviderCostRows()
    onSelectedProviderIDChanged: refreshProviderCostRows()

    Connections {
        target: AppController
        function onCopyFeedbackTriggered(text) {
            toast.show(text, toast.typeSuccess)
        }
    }

    Connections {
        target: TrayViewModel
        function onCostUsageRefreshingChanged() { root.refreshCostSummary() }
        function onCostDataChanged() { root.refreshCostSummary() }
        function onDisplayCostDataChanged() { root.refreshCostSummary() }
        function onProviderCostRowsChanged() { root.refreshProviderCostRows() }
        function onCodexAccountStateChanged() {
            root.codexAccountState = TrayViewModel.codexAccountState || ({})
        }
        function onProviderDataChanged() {
            detailFlickable.refreshDetailData()
        }
        function onIsRefreshingChanged() {
            root.isRefreshing = TrayViewModel.isRefreshing
            if (root.isRefreshing) {
                root.refreshStartTime = Date.now()
                refreshTimer.start()
            } else {
                refreshTimer.stop()
                root.refreshStartTime = 0
                root.refreshDuration = ""
            }
        }
    }

    Timer {
        id: refreshTimer
        interval: 1000
        repeat: true
        onTriggered: {
            if (root.refreshStartTime > 0) {
                var seconds = Math.floor((Date.now() - root.refreshStartTime) / 1000)
                root.refreshDuration = seconds + "s"
            }
        }
    }

    // Drop shadow mimic
    Rectangle {
        anchors.fill: parent
        anchors.margins: -1
        radius: 13
        color: "transparent"
        border.color: AppTheme.surfaceBorder
        border.width: 1
        z: -1
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // === Header ===
        Components.SelectiveRadiusRect {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            fillColor: root.titleBarBackgroundColor
            topLeftRadius: glassEffectActive ? 0 : 12
            topRightRadius: glassEffectActive ? 0 : 12
            bottomLeftRadius: 0
            bottomRightRadius: 0

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: AppTheme.surfaceBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 8
                Text {
                    text: qsTr("CodexBar")
                    color: AppTheme.textPrimary
                    font.pixelSize: 15
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: TrayViewModel.providerCount + " " + qsTr("providers")
                    color: AppTheme.textTertiary
                    font.pixelSize: 11
                }
            }

            MouseArea {
                id: headerDragArea
                anchors.fill: parent
                cursorShape: Qt.SizeAllCursor
                property int pressX: 0
                property int pressY: 0
                onPressed: {
                    pressX = mouseX
                    pressY = mouseY
                }
                onPositionChanged: {
                    AppController.moveTrayPanel(mouseX - pressX, mouseY - pressY)
                }
            }
        }

        // === Provider Switcher (Phase 2) ===
        Components.ProviderSwitcher {
            id: providerSwitcher
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 8
            providerList: TrayViewModel.providerSwitcherList
            selectedProviderID: root.selectedProviderID
            isSwitching: TrayViewModel.providerSwitching
            onSelectProvider: function(providerId) {
                TrayViewModel.selectProvider(providerId)
            }
        }

        // === Codex Account Switcher (Phase 3) ===
        Components.CodexAccountSwitcher {
            id: codexAccountSwitcher
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 4
            visible: root.selectedProviderID === "codex" && codexAccounts.length > 1
            accounts: codexAccounts
            selectedAccountID: root.codexAccountState ? (root.codexAccountState.activeAccountID || "") : ""
            isSwitching: root.codexAccountState ? (root.codexAccountState.isAuthenticating === true) : false
            onSelectAccount: function(accountID) {
                TrayViewModel.setCodexActiveAccount(accountID)
            }
        }

        // === Token Usage Card ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: costSection.implicitHeight
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.topMargin: 10
            Layout.bottomMargin: 8
            color: "transparent"
            clip: true

            ColumnLayout {
                id: costSection
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 0

                // Header row
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    radius: 8
                    color: costMouse.containsMouse ? AppTheme.surfaceHover : AppTheme.surfacePane

                    Behavior on color { ColorAnimation { duration: 150 } }

                    MouseArea {
                        id: costMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!TrayViewModel.costUsageEnabled) {
                                TrayViewModel.ensureCostUsageEnabled()
                            } else if (displayCostData.hasData) {
                                root.costExpanded = !root.costExpanded
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 8

                        Text {
                            Layout.preferredWidth: 12
                            text: root.costExpanded && displayCostData.hasData ? "▾" : "▸"
                            color: AppTheme.textTertiary
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Token Usage")
                            color: AppTheme.textSecondary
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                        }
                        Rectangle {
                            Layout.preferredWidth: 8
                            Layout.preferredHeight: 8
                            Layout.alignment: Qt.AlignVCenter
                            radius: 4
                            color: TrayViewModel.costUsageRefreshing ? AppTheme.statusDegraded
                                 : displayCostData.hasData ? AppTheme.statusOk : AppTheme.statusUnknown

                            SequentialAnimation on opacity {
                                running: TrayViewModel.costUsageRefreshing
                                loops: Animation.Infinite
                                NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                                NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                            }
                        }
                        Text {
                            text: qsTr("Details")
                            color: detailsMouse.containsMouse ? AppTheme.textSecondary : AppTheme.textTertiary
                            font.pixelSize: 10

                            MouseArea {
                                id: detailsMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    TrayViewModel.ensureCostUsageEnabled()
                                    AppController.openUsage()
                                }
                            }
                        }
                        Text {
                            Layout.maximumWidth: 86
                            text: displayCostData.hasData
                                ? "$" + displayCostData.sessionCostUSD.toFixed(2) + " " + qsTr("today")
                                : TrayViewModel.costUsageRefreshing ? qsTr("scanning...") : qsTr("no data")
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }
                }

                // Body
                ColumnLayout {
                    id: costBody
                    Layout.fillWidth: true
                    visible: root.costExpanded && displayCostData.hasData
                    spacing: 8
                    Layout.topMargin: 8

                    // Today + 30d summary
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        radius: 8
                        color: AppTheme.surfaceChart

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            anchors.topMargin: 8
                            anchors.bottomMargin: 8
                            spacing: 10

                            CostMetricCell {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 1
                                title: qsTr("Today")
                                value: "$" + formatCost(displayCostData.sessionCostUSD)
                                detail: fmtNum(displayCostData.sessionTokens) + " " + qsTr("tokens")
                                valueColor: AppTheme.statusOk
                            }

                            Rectangle {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                color: AppTheme.surfaceBorder
                            }

                            CostMetricCell {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 1
                                title: qsTr("30 days")
                                value: "$" + formatCost(displayCostData.last30DaysCostUSD)
                                detail: fmtNum(displayCostData.last30DaysTokens) + " " + qsTr("tokens")
                                valueColor: AppTheme.accentColor
                            }
                        }
                    }

                    // Daily bars
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        radius: 8
                        color: AppTheme.surfaceChart
                        clip: true

                        Row {
                            id: dailyBarChart
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 2
                            layoutDirection: Qt.RightToLeft
                            property double dailyMaxCost: {
                                var maxCost = 0
                                if (root.costExpanded && displayCostData.daily) {
                                    for (var i = 0; i < displayCostData.daily.length; i++)
                                        maxCost = Math.max(maxCost, displayCostData.daily[i].costUSD)
                                }
                                return maxCost
                            }

                            Repeater {
                                model: root.costExpanded && displayCostData.daily ? displayCostData.daily.slice(-21) : []
                                delegate: Rectangle {
                                    width: Math.max(2, parent.width / 21 - 2)
                                    height: {
                                        var chartHeight = Math.max(2, parent ? parent.height : 44)
                                        return dailyBarChart.dailyMaxCost > 0
                                            ? Math.max(2, chartHeight * modelData.costUSD / dailyBarChart.dailyMaxCost)
                                            : 2
                                    }
                                    y: Math.max(0, (parent ? parent.height : 44) - height)
                                    radius: 1
                                    color: index % 7 === 0 ? AppTheme.accentColor : AppTheme.surfaceSelected

                                    Rectangle {
                                        anchors.bottom: parent.bottom
                                        width: parent.width
                                        height: 1
                                        color: AppTheme.surfaceBorder
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4
                            Text { text: qsTr("max"); color: AppTheme.textInverse; font.pixelSize: 8 }
                            Text { text: "0"; color: AppTheme.textInverse; font.pixelSize: 8 }
                        }
                    }

                    // Legend
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Rectangle { width: 10; height: 10; radius: 5; color: AppTheme.accentColor }
                        Text { text: qsTr("Mon"); color: AppTheme.textTertiary; font.pixelSize: 9 }
                        Rectangle { width: 1; height: 10; color: AppTheme.surfaceBorder }
                        Rectangle { width: 10; height: 10; radius: 5; color: AppTheme.surfaceSelected }
                        Text { text: qsTr("other day"); color: AppTheme.textTertiary; font.pixelSize: 9 }
                    }

                    // Per-provider breakdown
                    Repeater {
                        model: root.providerCostRows  // 已按 selectedProviderID 过滤
                        delegate: ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            property bool providerExpanded: false

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 28
                                radius: 6
                                color: providerMouse.containsMouse ? AppTheme.surfaceHover : AppTheme.surfaceControl

                                Behavior on color { ColorAnimation { duration: 150 } }

                                MouseArea {
                                    id: providerMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: providerExpanded = !providerExpanded
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 8
                                    anchors.rightMargin: 8
                                    spacing: 6

                                    Text {
                                        Layout.preferredWidth: 12
                                        text: providerExpanded ? "▾" : "▸"
                                        color: AppTheme.textTertiary
                                        font.pixelSize: 10
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: 8
                                        Layout.preferredHeight: 8
                                        radius: 4
                                        color: brandColorFor(modelData.providerId)
                                    }
                                    Text {
                                        text: {
                                            var names = {
                                                "codex": "Codex", "claude": "Claude",
                                                "opencodego": "OpenCode Go",
                                                "qianfan": "QianFan"
                                            }
                                            return names[modelData.providerId] || modelData.providerId
                                        }
                                        color: AppTheme.textSecondary
                                        font.pixelSize: 11
                                        font.bold: true
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        text: "$" + formatCost(modelData.last30DaysCostUSD)
                                        color: AppTheme.textTertiary
                                        font.pixelSize: 10
                                    }
                                    Text {
                                        text: fmtNum(modelData.last30DaysTokens) + " " + qsTr("tokens")
                                        color: AppTheme.textInverse
                                        font.pixelSize: 9
                                    }
                                }
                            }

                            // Model breakdown (expanded)
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: providerExpanded
                                spacing: 2
                                Layout.leftMargin: 28

                                Repeater {
                                    model: modelData.models || []
                                    delegate: RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: "└─"
                                            color: AppTheme.textDisabled
                                            font.pixelSize: 9
                                        }
                                        Text {
                                            text: modelData.name
                                            color: AppTheme.textSecondary
                                            font.pixelSize: 10
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: "$" + formatCost(modelData.costUSD)
                                            color: AppTheme.textTertiary
                                            font.pixelSize: 9
                                        }
                                        Text {
                                            text: fmtNum(modelData.tokens) + " tk"
                                            color: AppTheme.textInverse
                                            font.pixelSize: 9
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // === Provider List (Overview) ===
        ListView {
            id: providerList
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedProviderID === ""
            clip: true
            spacing: 6
            topMargin: 2
            bottomMargin: 10
            leftMargin: 12
            rightMargin: 12

            model: TrayViewModel.providers
            delegate: Rectangle {
                id: cardDelegate
                width: providerList.width - 24
                height: cardContent.height + 24
                radius: 10
                color: mouseArea.containsMouse ? AppTheme.surfaceHover : root.cardBackgroundColor
                border.color: mouseArea.containsMouse ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                border.width: 1

                property string providerId: model.providerId || ""
                property var snap: model.snap || ({})
                property bool expanded: root.expandedCards[providerId] === true
                property color brandColor: brandColorFor(providerId)
                property bool isDetailProvider: providerId === "deepseek"
                    || providerId === "warp"
                    || providerId === "kilo"
                    || providerId === "abacus"
                    || providerId === "codebuff"
                property var tokenAccounts: model.tokenAccounts || []
                property string defaultTokenAccountId: model.defaultTokenAccountId || ""
                property string statusUrl: model.statusUrl || ""
                property var dashboard: model.dashboard || ({})
                property bool hasTokenAccounts: tokenAccounts && tokenAccounts.length > 0
                property var accountOptions: model.accountOptions || []
                property string primaryLabel: snap.displayName === "OpenRouter" && snap.openRouterUsage !== undefined
                    ? qsTr("API key limit") : snap.sessionLabel

                Behavior on border.color { ColorAnimation { duration: 150 } }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Card expand/collapse always works
                        var cards = Object.assign({}, root.expandedCards)
                        cards[cardDelegate.providerId] = !cards[cardDelegate.providerId]
                        root.expandedCards = cards

                        // Refresh only if not already refreshing
                        if (!root.isRefreshing) {
                            TrayViewModel.refreshProvider(cardDelegate.providerId)
                        }
                    }
                }

                ColumnLayout {
                    id: cardContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 6

                    // === Title row ===
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Text {
                            text: snap.displayName
                            color: AppTheme.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                            Layout.fillWidth: true
                        }
                        Text {
                            text: snap.loginMethod && snap.loginMethod !== "" ? snap.loginMethod : ""
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            visible: !!snap.loginMethod && snap.loginMethod !== "" && !cardDelegate.expanded
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: cardDelegate.hasTokenAccounts

                        Text {
                            text: qsTr("Account")
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.preferredWidth: 54
                        }

                        Components.SettingsComboBox {
                                objectName: "accountSwitcher_" + cardDelegate.providerId
                            Layout.fillWidth: true
                            Layout.preferredHeight: 30
                            model: cardDelegate.accountOptions
                            selectedValue: cardDelegate.defaultTokenAccountId
                            onValueActivated: function(value) {
                                if (value !== cardDelegate.defaultTokenAccountId) {
                                    TrayViewModel.requestSetDefaultTokenAccount(cardDelegate.providerId, value)
                                    root.tokenAccountRevision += 1
                                }
                            }
                        }
                    }

                    // === Subtitle row ===
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        visible: snap.error !== ""
                        Text {
                            text: snap.error
                            color: AppTheme.statusOutage
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            maximumLineCount: 2
                            wrapMode: Text.WordWrap
                        }
                    }

                    // === Refresh status ===
                    Text {
                        Layout.fillWidth: true
                        visible: !snap.hasUsage && snap.error === ""
                        text: root.isRefreshing ? qsTr("Refreshing...") : qsTr("No usage yet")
                        color: AppTheme.textDisabled
                        font.pixelSize: 12
                    }

                    // === Primary bar ===
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage
                        spacing: 6
                        Text {
                            text: cardDelegate.isDetailProvider ? qsTr("Balance") : cardDelegate.primaryLabel
                            color: AppTheme.textSecondary
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 80
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: AppTheme.surfaceBorder
                            Rectangle {
                                width: Math.max(0, parent.width * (cardDelegate.isDetailProvider ? snap.primaryRemaining : snap.primaryUsed) / 100)
                                height: parent.height
                                radius: 3
                                color: cardDelegate.brandColor
                                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                            Rectangle {
                                visible: !cardDelegate.isDetailProvider && snap.primaryPacePercent !== undefined && snap.primaryPacePercent >= 0
                                x: Math.max(0, Math.min(parent.width - 3, parent.width * (snap.primaryPacePercent || 0) / 100 - 1))
                                width: 3
                                height: parent.height
                                color: (snap.primaryPaceOnTop !== false) ? AppTheme.statusOk : AppTheme.statusOutage
                                radius: 1
                                Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                        }
                        Text {
                            text: (snap.primaryDisplayPercent !== undefined ? snap.primaryDisplayPercent : snap.primaryRemaining).toFixed(0) + "%"
                            color: snap.primaryRemaining > 50 ? AppTheme.statusOk
                                 : snap.primaryRemaining > 20 ? AppTheme.statusDegraded
                                 : AppTheme.statusOutage
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 50
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // Primary pace + reset
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.primaryPaceLeftLabel !== undefined
                        spacing: 4
                        Text {
                            text: snap.primaryPaceLeftLabel || ""
                            color: AppTheme.textSecondary
                            font.pixelSize: 10
                            Layout.preferredWidth: 80
                        }
                        Text {
                            text: snap.primaryPaceRightLabel || ""
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.primaryResetDesc !== undefined && snap.primaryResetDesc !== ""
                        spacing: 4
                        Item { Layout.preferredWidth: 80 }
                        Text {
                            text: qsTr("Resets") + " " + (snap.primaryResetDesc || "")
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    // === Primary detail (balance/credit info text line) ===
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.primaryDetail !== undefined && snap.primaryDetail !== ""
                        spacing: 4
                        Item { Layout.preferredWidth: 80 }
                        Text {
                            text: snap.primaryDetail || ""
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    // === Secondary bar ===
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.hasSecondary === true
                        spacing: 6
                        Text {
                            text: snap.weeklyLabel
                            color: AppTheme.textSecondary
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 80
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: AppTheme.surfaceBorder
                            Rectangle {
                                width: Math.max(0, parent.width * snap.secondaryUsed / 100)
                                height: parent.height
                                radius: 3
                                color: cardDelegate.brandColor
                                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                            Rectangle {
                                visible: snap.secondaryPacePercent !== undefined && snap.secondaryPacePercent >= 0
                                x: Math.max(0, Math.min(parent.width - 3, parent.width * (snap.secondaryPacePercent || 0) / 100 - 1))
                                width: 3
                                height: parent.height
                                color: (snap.secondaryPaceOnTop !== false) ? AppTheme.statusOk : AppTheme.statusOutage
                                radius: 1
                                Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                        }
                        Text {
                            text: (snap.secondaryDisplayPercent !== undefined ? snap.secondaryDisplayPercent : snap.secondaryRemaining).toFixed(0) + "%"
                            color: snap.secondaryRemaining > 50 ? AppTheme.accentColor
                                 : snap.secondaryRemaining > 20 ? AppTheme.statusDegraded
                                 : AppTheme.statusOutage
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 50
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // Secondary pace + reset
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.hasSecondary === true && snap.secondaryPaceLeftLabel !== undefined
                        spacing: 4
                        Text {
                            text: snap.secondaryPaceLeftLabel || ""
                            color: AppTheme.textSecondary
                            font.pixelSize: 10
                            Layout.preferredWidth: 80
                        }
                        Text {
                            text: snap.secondaryPaceRightLabel || ""
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.hasSecondary === true && snap.secondaryResetDesc !== undefined && snap.secondaryResetDesc !== ""
                        spacing: 4
                        Item { Layout.preferredWidth: 80 }
                        Text {
                            text: qsTr("Resets") + " " + (snap.secondaryResetDesc || "")
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    // === Tertiary bar (e.g. Sonnet, 5h, Opus) ===
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.hasTertiary === true
                        spacing: 6
                        Text {
                            text: snap.opusLabel || qsTr("Opus")
                            color: AppTheme.textSecondary
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 80
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: AppTheme.surfaceBorder
                            Rectangle {
                                width: Math.max(0, parent.width * (snap.tertiaryUsed || 0) / 100)
                                height: parent.height
                                radius: 3
                                color: cardDelegate.brandColor
                                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                        }
                        Text {
                            text: (snap.tertiaryDisplayPercent !== undefined
                                ? snap.tertiaryDisplayPercent
                                : (snap.tertiaryRemaining !== undefined ? snap.tertiaryRemaining : 100)).toFixed(0) + "%"
                            color: snap.tertiaryRemaining > 50 ? AppTheme.accentColor
                                 : snap.tertiaryRemaining > 20 ? AppTheme.statusDegraded
                                 : AppTheme.statusOutage
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 50
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.hasTertiary === true && snap.tertiaryResetDesc !== undefined && snap.tertiaryResetDesc !== ""
                        spacing: 4
                        Item { Layout.preferredWidth: 80 }
                        Text {
                            text: qsTr("Resets") + " " + (snap.tertiaryResetDesc || "")
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    // === Codex Credits ===
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 28
                        visible: cardDelegate.providerId === "codex" && snap.hasCredits === true
                        color: root.cardBackgroundColor
                        radius: 6

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            Text {
                                            text: qsTr("Credits")
                                color: AppTheme.textTertiary
                                font.pixelSize: 11
                                font.bold: true
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: "$" + (snap.creditsRemaining || 0).toFixed(2) + " left"
                                color: (snap.creditsRemaining || 0) > 10 ? AppTheme.statusOk
                                     : (snap.creditsRemaining || 0) > 2 ? AppTheme.statusDegraded
                                     : AppTheme.statusOutage
                                font.pixelSize: 13
                                font.bold: true
                            }
                        }
                    }

                    // === Codex Credits Error ===
                    Text {
                        Layout.fillWidth: true
                        visible: cardDelegate.providerId === "codex" && snap.creditsError !== undefined && snap.creditsError !== ""
                        text: snap.creditsError || ""
                        color: AppTheme.statusDegraded
                        font.pixelSize: 10
                        elide: Text.ElideRight
                        maximumLineCount: 1
                    }

                    // === Provider cost bar ===
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasProviderCost === true
                        spacing: 6
                        Text {
                            text: snap.providerCostCurrency === "Quota" ? qsTr("Quota usage") : qsTr("Extra usage")
                            color: AppTheme.textSecondary
                            font.pixelSize: 11
                            font.bold: true
                            Layout.preferredWidth: 80
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: AppTheme.surfaceBorder
                            Rectangle {
                                width: Math.max(0, parent.width * Math.min(100, (snap.providerCostUsed || 0) / Math.max(1, snap.providerCostLimit || 1) * 100) / 100)
                                height: parent.height
                                radius: 3
                                color: AppTheme.statusDegraded
                                Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: snap.hasProviderCost === true
                        spacing: 4
                        Item { Layout.preferredWidth: 80 }
                        Text {
                            text: qsTr("This month") + ": $" + (snap.providerCostUsed || 0).toFixed(2) + " / $" + (snap.providerCostLimit || 0).toFixed(2)
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.fillWidth: true
                        }
                        Text {
                            text: {
                                var pct = snap.providerCostLimit > 0 ? ((snap.providerCostUsed || 0) / (snap.providerCostLimit || 1) * 100).toFixed(0) : "0"
                                return qsTr("%1 used").arg(pct)
                            }
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // === Updated timestamp ===
                    Text {
                        Layout.fillWidth: true
                        visible: snap.hasUsage && snap.updatedAt > 0 && !root.isRefreshing
                        text: qsTr("Updated") + " " + timeAgo(snap.updatedAt)
                        color: AppTheme.textDisabled
                        font.pixelSize: 9
                    }

                    // === Expanded detail row ===
                    ColumnLayout {
                        Layout.fillWidth: true
                        visible: cardDelegate.expanded
                        spacing: 4
                        Layout.topMargin: 4

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: AppTheme.surfaceBorder
                        }

                        // Zai MCP details
                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: snap.zaiUsage !== undefined && snap.zaiUsage.timeLimit !== undefined
                            spacing: 2
                            Text {
                                text: qsTr("MCP details")
                                color: AppTheme.textSecondary
                                font.pixelSize: 10
                                font.bold: true
                            }
                            Repeater {
                                model: cardDelegate.expanded && snap.zaiUsage && snap.zaiUsage.timeLimit
                                    ? snap.zaiUsage.timeLimit.usageDetails : []
                                delegate: RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    Text {
                                        text: modelData.modelCode
                                        color: AppTheme.textTertiary
                                        font.pixelSize: 9
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        text: fmtNum(modelData.usage)
                                        color: AppTheme.textSecondary
                                        font.pixelSize: 9
                                    }
                                }
                            }
                        }

                        // OpenRouter key quota status
                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: snap.openRouterUsage !== undefined
                            spacing: 2
                            Text {
                                visible: !!snap.openRouterUsage && snap.openRouterUsage.keyQuotaStatus === 1
                                text: qsTr("No limit set for the API key")
                                color: AppTheme.statusDegraded
                                font.pixelSize: 10
                            }
                            Text {
                                visible: !!snap.openRouterUsage && snap.openRouterUsage.keyQuotaStatus === 2
                                text: qsTr("API key limit unavailable right now")
                                color: AppTheme.statusOutage
                                font.pixelSize: 10
                            }
                            Text {
                                visible: !!snap.openRouterUsage && snap.openRouterUsage.keyRemaining !== undefined
                                text: snap.openRouterUsage && snap.openRouterUsage.keyRemaining !== undefined
                                    ? "$" + (snap.openRouterUsage.keyRemaining || 0).toFixed(2) + "/$" + (snap.openRouterUsage.keyLimit || 0).toFixed(2) + " " + qsTr("left")
                                    : ""
                                color: AppTheme.textSecondary
                                font.pixelSize: 10
                            }
                        }

                        // Subscription Utilization chart
                        Loader {
                            Layout.fillWidth: true
                            active: cardDelegate.expanded && (cardDelegate.providerId === "codex" || cardDelegate.providerId === "claude")
                            sourceComponent: PlanUtilizationChart {
                                providerId: cardDelegate.providerId
                                hasTertiarySeries: snap.hasTertiary === true
                                tertiarySeriesLabel: snap.opusLabel || qsTr("Opus")
                            }
                        }

                        // Codex Dashboard Details (expandable)
                        ColumnLayout {
                            Layout.fillWidth: true
                            visible: cardDelegate.expanded && cardDelegate.providerId === "codex"
                            spacing: 6

                            property var dashboard: cardDelegate.expanded && cardDelegate.providerId === "codex"
                                ? cardDelegate.dashboard : ({})
                            property bool hasDashboard: dashboard && dashboard.creditEvents !== undefined

                            Rectangle {
                                Layout.fillWidth: true
                                height: 1
                                color: AppTheme.surfaceBorder
                                visible: parent.hasDashboard
                            }

                            // Credit Events
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: parent.hasDashboard && parent.dashboard.creditEvents && parent.dashboard.creditEvents.length > 0
                                spacing: 4
                                Text {
                                    text: qsTr("Credit Events")
                                    color: AppTheme.textSecondary
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                                Repeater {
                                    model: cardDelegate.expanded ? (parent.parent.dashboard.creditEvents || []) : []
                                    delegate: RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: modelData.date ? new Date(modelData.date).toLocaleDateString(Qt.locale(), "yyyy-MM-dd") : ""
                                            color: AppTheme.textTertiary
                                            font.pixelSize: 9
                                            Layout.preferredWidth: 70
                                        }
                                        Text {
                                            text: modelData.service || ""
                                            color: AppTheme.textTertiary
                                            font.pixelSize: 9
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: "$" + (modelData.amount || 0).toFixed(2)
                                            color: modelData.amount >= 0 ? AppTheme.statusOk : AppTheme.statusOutage
                                            font.pixelSize: 9
                                            horizontalAlignment: Text.AlignRight
                                        }
                                    }
                                }
                            }

                            // Usage by Service
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: parent.hasDashboard && parent.dashboard.usageByService && parent.dashboard.usageByService.length > 0
                                spacing: 4
                                Text {
                                    text: qsTr("Usage by Service")
                                    color: AppTheme.textSecondary
                                    font.pixelSize: 10
                                    font.bold: true
                                }
                                Repeater {
                                    model: cardDelegate.expanded ? (parent.parent.dashboard.usageByService || []) : []
                                    delegate: RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 4
                                        Text {
                                            text: modelData.service || ""
                                            color: AppTheme.textTertiary
                                            font.pixelSize: 9
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            text: fmtNum(modelData.tokens || 0) + " tk"
                                            color: AppTheme.textSecondary
                                            font.pixelSize: 9
                                        }
                                        Text {
                                            text: "$" + (modelData.costUSD || 0).toFixed(2)
                                            color: AppTheme.textSecondary
                                            font.pixelSize: 9
                                            horizontalAlignment: Text.AlignRight
                                        }
                                    }
                                }
                            }

                            // Purchase URL
                            Text {
                                Layout.fillWidth: true
                                visible: parent.hasDashboard && parent.dashboard.purchaseURL
                                text: "<a href=\"" + (parent.dashboard.purchaseURL || "") + "\">" + qsTr("Purchase credits") + "</a>"
                                color: AppTheme.accentColor
                                font.pixelSize: 10
                                textFormat: Text.RichText
                                onLinkActivated: Qt.openUrlExternally(link)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            Text {
                                text: qsTr("Updated")
                                color: AppTheme.textTertiary
                                font.pixelSize: 10
                                Layout.preferredWidth: 80
                            }
                            Text {
                                        text: snap.updatedAt && snap.updatedAt > 0
                                            ? new Date(snap.updatedAt).toLocaleTimeString(Qt.locale(), "hh:mm:ss")
                                            : "-"
                                        color: AppTheme.textInverse
                                        font.pixelSize: 10
                                        Layout.fillWidth: true
                                    }
                                }

                                // Status Page action
                                RowLayout {
                                    Layout.fillWidth: true
                                    visible: cardDelegate.statusUrl !== ""
                                    spacing: 6
                                    ActionButton {
                                        text: qsTr("Status")
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 26
                                        onClicked: {
                                            if (cardDelegate.statusUrl) AppController.openExternalUrl(cardDelegate.statusUrl)
                                        }
                                    }
                                }
                    }
                }
            }
        }

        // === Provider Detail (Phase 2) ===
        Flickable {
            id: detailFlickable
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.selectedProviderID !== ""
            contentWidth: width
            contentHeight: detailColumn.implicitHeight
            clip: true

            property var detailData: ({})

            function refreshDetailData() {
                detailData = TrayViewModel.providerData(root.selectedProviderID) || ({})
            }

            Component.onCompleted: refreshDetailData()
            Connections {
                target: root
                function onSelectedProviderIDChanged() {
                    detailFlickable.refreshDetailData()
                }
            }

            Column {
                id: detailColumn
                width: parent.width
                spacing: 4

                // Token Account Switcher (Phase 3) - for non-codex providers
                Components.TokenAccountSwitcher {
                    id: tokenAccountSwitcher
                    width: parent.width - 24
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: root.selectedProviderID !== "" && root.selectedProviderID !== "codex"
                        && (detailFlickable.detailData.tokenAccounts || []).length > 1
                    accounts: detailFlickable.detailData.tokenAccounts || []
                    selectedAccountID: detailFlickable.detailData.defaultTokenAccountId || ""
                    isSwitching: false
                    onSelectAccount: function(accountID) {
                        TrayViewModel.requestSetDefaultTokenAccount(root.selectedProviderID, accountID)
                    }
                }

                Components.ProviderDetailCard {
                    id: detailCard
                    width: parent.width
                    embedded: true
                    providerId: root.selectedProviderID
                    snap: detailFlickable.detailData.snap || ({})
                    tokenAccounts: detailFlickable.detailData.tokenAccounts || []
                    defaultTokenAccountId: detailFlickable.detailData.defaultTokenAccountId || ""
                    accountOptions: detailFlickable.detailData.accountOptions || []
                    statusUrl: detailFlickable.detailData.statusUrl || ""
                    dashboard: detailFlickable.detailData.dashboard || ({})
                    isRefreshing: root.isRefreshing
                }

                // TrayMenuActions inside Flickable, right below card
                Components.TrayMenuActions {
                    id: trayMenuActions
                    currentProviderID: root.selectedProviderID
                    currentSnapshot: detailFlickable.detailData.snap || ({})
                    currentError: detailFlickable.detailData.snap ? (detailFlickable.detailData.snap.error || "") : ""
                    dashboardURL: detailFlickable.detailData.dashboard ? (detailFlickable.detailData.dashboard.purchaseURL || "") : ""
                }
            }
        }

        // === Footer ===
        Components.SelectiveRadiusRect {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            fillColor: root.titleBarBackgroundColor
            topLeftRadius: 0
            topRightRadius: 0
            bottomLeftRadius: glassEffectActive ? 0 : 12
            bottomRightRadius: glassEffectActive ? 0 : 12

            Rectangle {
                anchors.top: parent.top
                width: parent.width; height: 1
                color: AppTheme.surfaceBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                ActionButton {
                    text: root.isRefreshing
                        ? qsTr("Refreshing...") + (root.refreshDuration ? " " + root.refreshDuration : "")
                        : qsTr("Refresh")
                    enabled: !root.isRefreshing
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    onClicked: TrayViewModel.refresh()
                }
                ActionButton {
                    text: qsTr("Settings")
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    onClicked: AppController.toggleSettings()
                }
                ActionButton {
                    text: qsTr("About")
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    onClicked: AppController.showAbout()
                }
                ActionButton {
                    text: qsTr("Quit")
                    textColor: AppTheme.statusOutage
                    hoverColor: AppTheme.withAlpha(AppTheme.statusOutage, 0.18)
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    onClicked: AppController.quitApp()
                }
            }
        }
    }

    function fmtNum(n) {
        if (n === undefined || n === null) return "0"
        if (n >= 1000000) return (n / 1000000).toFixed(1) + "M"
        if (n >= 1000) return (n / 1000).toFixed(1) + "K"
        return n.toString()
    }

    function formatCost(n) {
        var value = Number(n || 0)
        if (Math.abs(value) >= 1000) return value.toFixed(1)
        if (Math.abs(value) >= 100) return value.toFixed(2)
        return value.toFixed(4)
    }

    function refreshCostSummary() {
        root.costData = TrayViewModel.costData
        root.displayCostData = TrayViewModel.displayCostData
        root.refreshProviderCostRows()
    }

    function refreshProviderCostRows() {
        if (root.costExpanded && root.displayCostData && root.displayCostData.hasData) {
            root.providerCostRows = TrayViewModel.providerCostUsageForProvider(root.selectedProviderID)
        } else {
            root.providerCostRows = []
        }
    }

    function brandColorFor(providerId) {
        return AppTheme.providerBrandColor(providerId)
    }

    function timeAgo(ms) {
        if (!ms || ms <= 0) return qsTr("never")
        var ago = Date.now() - ms
        if (ago < 60000) return qsTr("just now")
        if (ago < 3600000) return Math.floor(ago / 60000) + qsTr("m ago")
        if (ago < 86400000) return Math.floor(ago / 3600000) + qsTr("h ago")
        return Math.floor(ago / 86400000) + qsTr("d ago")
    }

    component CostMetricCell: Item {
        id: metricCell
        property string title: ""
        property string value: ""
        property string detail: ""
        property color valueColor: AppTheme.textSecondary

        implicitHeight: cellColumn.implicitHeight
        clip: true

        Column {
            id: cellColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 1

            Text {
                width: parent.width
                text: metricCell.title
                color: AppTheme.textTertiary
                font.pixelSize: 10
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: metricCell.value
                color: metricCell.valueColor
                font.pixelSize: 16
                minimumPixelSize: 10
                fontSizeMode: Text.HorizontalFit
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: metricCell.detail
                color: AppTheme.textTertiary
                font.pixelSize: 10
                elide: Text.ElideRight
            }
        }
    }

    component ActionButton: Rectangle {
        property string text: ""
        property color textColor: AppTheme.textSecondary
        property color hoverColor: AppTheme.surfaceHover
        signal clicked()

        radius: 6
        color: btnMouse.containsMouse ? hoverColor : "transparent"
        Behavior on color { ColorAnimation { duration: 150 } }

        Text {
            anchors.centerIn: parent
            text: parent.text
            color: parent.textColor
            font.pixelSize: 12
        }

        MouseArea {
            id: btnMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
    }

    Components.TrayToast { id: toast }
}

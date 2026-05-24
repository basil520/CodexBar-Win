import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import ".."
import "../components"
import "../components/usage" as UsageComponents

Rectangle {
    id: root
    color: SettingsStore.glassEffectEnabled ? "transparent" : AppTheme.surfaceWindow

    property var costData: UsageDetailsViewModel.costData
    property var providerRows: UsageDetailsViewModel.providerRows
    property var providerDetails: UsageDetailsViewModel.providerDetails
    property int rev: LanguageManager.translationRevision

    Component.onCompleted: UsageDetailsViewModel.activate()
    Component.onDestruction: UsageDetailsViewModel.deactivate()

    ScrollView {
        id: scroll
        anchors.fill: parent
        anchors.margins: 16
        clip: true

        ScrollBar.vertical: ScrollBar {
            id: elegantScrollBar
            policy: ScrollBar.AsNeeded
            active: hovered || pressed
                || (scroll.contentItem && (scroll.contentItem.moving === true || scroll.contentItem.flicking === true))

            background: Rectangle {
                color: "transparent"
            }

            contentItem: Rectangle {
                implicitWidth: 4
                radius: 2
                opacity: elegantScrollBar.active ? 1.0 : 0.0
                color: elegantScrollBar.hovered 
                    ? AppTheme.textSecondary 
                    : Qt.rgba(AppTheme.textSecondary.r, AppTheme.textSecondary.g, AppTheme.textSecondary.b, 0.35)

                Behavior on color { ColorAnimation { duration: 150 } }
                Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutQuad } }
                Behavior on implicitWidth { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
            }

            states: State {
                name: "hoveredState"; when: elegantScrollBar.hovered
                PropertyChanges { target: elegantScrollBar.contentItem; implicitWidth: 8; radius: 4 }
            }
        }

        UsageComponents.UsageTrendDeck {
            width: scroll.availableWidth
            spacing: 14

            UsageComponents.UsageCommandBar {
                Layout.fillWidth: true
                rangeLabel: qsTr("Last 30 days")
                freshnessLabel: root.costData && root.costData.updatedAt
                    ? qsTr("Updated ") + Qt.formatDateTime(new Date(root.costData.updatedAt), "MM-dd hh:mm")
                    : qsTr("Waiting for usage data")
                refreshing: UsageDetailsViewModel.costUsageRefreshing
                onRefreshRequested: UsageDetailsViewModel.refreshCostUsage()
            }

            UsageOverviewHero {
                Layout.fillWidth: true
                costData: root.costData
                tokenProviderCount: UsageDetailsViewModel.tokenProviderCount
                costUsageEnabled: UsageDetailsViewModel.costUsageEnabled
                costUsageRefreshing: UsageDetailsViewModel.costUsageRefreshing
                onRefreshRequested: UsageDetailsViewModel.refreshCostUsage()
            }

            Repeater {
                model: root.providerRows

                delegate: UsageProviderRow {
                    Layout.fillWidth: true
                    property var rowProvider: modelData
                    property var rowProviderDetail: root.providerDetails[rowProvider.providerId] || ({})
                    property bool rowExpanded: false
                    readonly property bool rowCanExpand: rowProvider.hasTokenData
                        && (rowProvider.hasDetailAvailable === true
                            || ((rowProviderDetail.models || []).length > 0))
                    readonly property color rowAccentColor: rowProvider.brandColor || root.brandColorFor(rowProvider.providerId)

                    provider: rowProvider
                    providerDetail: rowProviderDetail
                    providerId: rowProvider.providerId || ""
                    providerName: rowProvider.displayName || rowProvider.providerId || ""
                    accentColor: rowAccentColor
                    kindText: root.kindLabel(rowProvider.kind || "token")
                    subtitleText: rowProvider.hasTokenData
                        ? root.fmtNum(rowProvider.last30DaysTokens || 0) + " " + qsTr("tokens")
                        : root.kindLabel(rowProvider.kind || "token")
                    summary: root.usageSummary(rowProvider, "last30DaysCostUSD", "last30DaysTokens")
                    status: rowProvider.hasTokenData ? "none" : "warning"
                    expanded: rowExpanded
                    canExpand: rowCanExpand
                    providerEnabled: rowProvider.enabled !== false
                    trendValues: root.lastItems(rowProvider.daily || [], 30)

                    onToggleRequested: {
                        if (!rowExpanded && rowCanExpand) {
                            UsageDetailsViewModel.requestProviderDetail(rowProvider.providerId)
                        }
                        rowExpanded = !rowExpanded
                    }
                }
            }

            UsageComponents.UsageForecastPanel {
                Layout.fillWidth: true
                forecastText: root.providerRows.length === 0
                    ? qsTr("Enable providers or refresh usage before forecasting quota risk.")
                    : qsTr("Forecast is based on cached usage rows and never blocks this view.")
                riskLevel: root.providerRows.length === 0 ? "warning" : "info"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                radius: AppTheme.radiusSm
                color: AppTheme.surfacePane
                border.color: AppTheme.surfaceBorder
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    Label {
                        text: qsTr("Total")
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fontSizeMd
                        font.bold: true
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        text: "$" + root.formatCost(root.costData.last30DaysCostUSD || 0)
                            + " · " + root.fmtNum(root.costData.last30DaysTokens || 0) + " " + qsTr("tokens")
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fontSizeMd
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                visible: root.providerRows.length === 0
                spacing: 8

                Item { Layout.fillHeight: true }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("No token providers enabled")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeMd
                    horizontalAlignment: Text.AlignHCenter
                }

                Item { Layout.fillHeight: true }
            }

            Item { Layout.preferredHeight: 8 }
        }
    }

    function kindLabel(kind) {
        if (kind === "credit") return qsTr("Credit")
        if (kind === "quota") return qsTr("Quota")
        return qsTr("Token")
    }

    function lastItems(items, count) {
        if (!items) return []
        var start = Math.max(0, items.length - count)
        var result = []
        for (var i = start; i < items.length; ++i) result.push(items[i])
        return result
    }

    function usageSummary(provider, costKey, tokenKey) {
        if (!provider || !provider.hasTokenData) return provider ? root.kindLabel(provider.kind || "token") : ""
        return "$" + root.formatCost(provider[costKey] || 0)
            + " · " + root.fmtNum(provider[tokenKey] || 0) + " " + qsTr("tokens")
    }

    function fmtNum(n) {
        var value = Number(n || 0)
        if (value >= 1000000) return (value / 1000000).toFixed(1) + "M"
        if (value >= 1000) return (value / 1000).toFixed(1) + "K"
        return value.toString()
    }

    function formatCost(n) {
        var value = Number(n || 0)
        var absValue = Math.abs(value)
        if (absValue === 0) return "0.00"
        if (absValue < 0.01) return value.toFixed(4)
        if (absValue >= 1000) return value.toFixed(1)
        return value.toFixed(2)
    }

    function brandColorFor(providerId) {
        return AppTheme.providerBrandColor(providerId)
    }

}

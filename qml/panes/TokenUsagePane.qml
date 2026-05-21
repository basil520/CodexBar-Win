import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import ".."
import "../components"

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

        ColumnLayout {
            width: scroll.availableWidth
            spacing: 14

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Token Usage Overview")
                        color: AppTheme.textPrimary
                        font.pixelSize: 22
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.formatUpdatedAt(root.costData.updatedAt || 0)
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        elide: Text.ElideRight
                    }
                }

                StatusPill {
                    text: !UsageDetailsViewModel.costUsageEnabled
                        ? qsTr("Off")
                        : UsageDetailsViewModel.costUsageRefreshing ? qsTr("Scanning") : qsTr("Last 30 days")
                    toneColor: !UsageDetailsViewModel.costUsageEnabled
                        ? AppTheme.textTertiary
                        : UsageDetailsViewModel.costUsageRefreshing ? AppTheme.statusDegraded : AppTheme.statusOk
                }

                ActionButton {
                    text: UsageDetailsViewModel.costUsageRefreshing ? qsTr("Scanning") : qsTr("Refresh")
                    compact: true
                    enabled: UsageDetailsViewModel.costUsageEnabled && !UsageDetailsViewModel.costUsageRefreshing
                    onClicked: UsageDetailsViewModel.refreshCostUsage()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 166
                radius: AppTheme.radiusMd
                color: AppTheme.surfaceCard
                border.color: AppTheme.surfaceBorder
                border.width: 1
                clip: true

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 4
                    color: AppTheme.accentColor
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    anchors.leftMargin: 18
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 68
                        spacing: 16

                        SummaryMetric {
                            title: qsTr("Today")
                            value: "$" + root.formatCost(root.costData.sessionCostUSD || 0)
                            detail: root.fmtNum(root.costData.sessionTokens || 0) + " " + qsTr("tokens")
                            accentColor: AppTheme.accentColor
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            sparkPoints: [6, 12, 9, 15, 11, 20, 18]
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: AppTheme.surfaceBorder
                        }

                        SummaryMetric {
                            title: qsTr("30 days")
                            value: "$" + root.formatCost(root.costData.last30DaysCostUSD || 0)
                            detail: root.fmtNum(root.costData.last30DaysTokens || 0) + " " + qsTr("tokens")
                            accentColor: AppTheme.statusOk
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            sparkPoints: {
                                var dailyList = root.costData.daily || [];
                                if (dailyList.length >= 7) {
                                    var items = root.lastItems(dailyList, 7);
                                    var pts = [];
                                    for (var i = 0; i < items.length; ++i) {
                                        pts.push(root.dailyValue(items[i]));
                                    }
                                    return pts;
                                }
                                return [32, 48, 42, 58, 52, 68, 62];
                            }
                        }

                        Rectangle {
                            Layout.preferredWidth: 1
                            Layout.fillHeight: true
                            color: AppTheme.surfaceBorder
                        }

                        SummaryMetric {
                            title: qsTr("Providers")
                            value: UsageDetailsViewModel.tokenProviderCount.toString()
                            detail: qsTr("token sources")
                            accentColor: AppTheme.statusDegraded
                            Layout.fillWidth: true
                            Layout.preferredWidth: 1
                            sparkPoints: [1, 2, 2, 3, 3, 3, Math.max(1, UsageDetailsViewModel.tokenProviderCount || 3)]
                        }
                    }

                    MiniBars {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        daily: root.lastItems(root.costData.daily || [], 30)
                        tintColor: AppTheme.accentColor
                    }
                }
            }

            Repeater {
                model: root.providerRows

                delegate: ProviderUsageCard {
                    Layout.fillWidth: true
                    provider: modelData
                }
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

    function dailyValue(day) {
        if (!day) return 0
        var cost = Number(day.costUSD || 0)
        if (cost > 0) return cost
        return Number(day.totalTokens || 0)
    }

    function maxDailyValue(days) {
        var maxValue = 0
        if (!days) return 0
        for (var i = 0; i < days.length; ++i) {
            maxValue = Math.max(maxValue, root.dailyValue(days[i]))
        }
        return maxValue
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

    function formatUpdatedAt(updatedAt) {
        var prefix = qsTr("Provider and model breakdown")
        if (UsageDetailsViewModel.costUsageRefreshing) {
            return prefix + " · " + qsTr("Scanning")
        }
        var timestamp = Number(updatedAt || 0)
        if (timestamp <= 0) {
            return prefix
        }
        return prefix + " · " + qsTr("Updated") + " "
            + new Date(timestamp).toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm")
    }

    component SummaryMetric: Item {
        id: metric
        property string title: ""
        property string value: ""
        property string detail: ""
        property color accentColor: AppTheme.accentColor
        property var sparkPoints: [10, 15, 12, 24, 18, 30, 28]
        property bool hovered: metricMouse.containsMouse

        implicitHeight: 58
        clip: true

        MouseArea {
            id: metricMouse
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
        }

        RowLayout {
            anchors.fill: parent
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: metric.title
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: metric.value
                    color: metric.accentColor
                    font.pixelSize: 21
                    minimumPixelSize: 13
                    fontSizeMode: Text.HorizontalFit
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: metric.detail
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeSm
                    elide: Text.ElideRight
                }
            }

            MetricSparkline {
                id: sparkline
                Layout.preferredWidth: 68
                Layout.preferredHeight: 30
                Layout.alignment: Qt.AlignVCenter
                strokeColor: metric.accentColor
                hovered: metric.hovered
                dataPoints: metric.sparkPoints
            }
        }
    }

    component MiniBars: Item {
        id: chart
        property var daily: []
        property color tintColor: AppTheme.accentColor
        property real maxValue: root.maxDailyValue(daily)

        clip: true

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: AppTheme.surfaceBorder
        }

        Row {
            id: bars
            anchors.fill: parent
            spacing: 2

            Repeater {
                model: chart.daily

                delegate: Item {
                    width: chart.daily.length > 0
                        ? Math.max(3, (bars.width - Math.max(0, chart.daily.length - 1) * bars.spacing) / chart.daily.length)
                        : bars.width
                    height: bars.height

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: chart.maxValue > 0
                            ? Math.max(2, parent.height * root.dailyValue(modelData) / chart.maxValue)
                            : 2
                        radius: 2
                        color: chart.tintColor
                        opacity: 0.78
                    }
                }
            }
        }
    }

    component ProviderUsageCard: SurfaceCard {
        id: card
        property var provider: ({})
        property var providerDetail: root.providerDetails[provider.providerId] || ({})
        property bool expanded: false
        readonly property bool canExpand: provider.hasTokenData
            && (provider.hasDetailAvailable === true
                || ((providerDetail.models || []).length > 0))
        readonly property bool detailLoading: providerDetail.state === "loading"
        readonly property var detailModels: providerDetail.models || []
        readonly property color accentColor: provider.brandColor || root.brandColorFor(provider.providerId)

        onExpandedChanged: {
            if (expanded && canExpand) {
                UsageDetailsViewModel.requestProviderDetail(provider.providerId)
            }
        }

        Layout.preferredHeight: contentColumn.implicitHeight + 24
        radius: AppTheme.radiusLg
        interactive: true
        selected: card.expanded
        tone: card.provider.hasTokenData ? "neutral" : "warning"
        opacity: card.provider.enabled === false ? 0.64 : 1
        clip: true

        // 物理悬浮微缩放动效
        scale: cardMouse.containsMouse ? 1.006 : 1.0

        Behavior on scale {
            NumberAnimation {
                duration: AppTheme.duration(AppTheme.motionNormal)
                easing.type: AppTheme.easeStandard
            }
        }

        // 非阻塞纯悬停检测 MouseArea (不吞噬任何子控件事件)
        MouseArea {
            id: cardMouse
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 4
            color: card.accentColor
        }

        ColumnLayout {
            id: contentColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 12
            anchors.leftMargin: 16
            spacing: 10

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 30

                UsageProviderRow {
                    anchors.fill: parent
                    provider: card.provider
                    providerId: card.provider.providerId || ""
                    providerName: card.provider.displayName || card.provider.providerId || ""
                    accentColor: card.accentColor
                    kindText: root.kindLabel(card.provider.kind || "token")
                    subtitleText: card.provider.hasTokenData
                        ? root.fmtNum(card.provider.last30DaysTokens || 0) + " " + qsTr("tokens")
                        : root.kindLabel(card.provider.kind || "token")
                    summary: root.usageSummary(card.provider, "last30DaysCostUSD", "last30DaysTokens")
                    status: card.provider.hasTokenData ? "none" : "warning"
                    expanded: card.expanded
                    canExpand: card.canExpand
                    providerEnabled: card.provider.enabled !== false
                    onToggleRequested: card.expanded = !card.expanded
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 22
                Layout.preferredHeight: card.provider.hasTokenData ? 50 : 4
                spacing: 12
                visible: card.provider.hasTokenData

                StatBlock {
                    title: qsTr("Today")
                    value: "$" + root.formatCost(card.provider.sessionCostUSD || 0)
                    detail: root.fmtNum(card.provider.sessionTokens || 0) + " " + qsTr("tokens")
                    Layout.fillWidth: true
                }

                StatBlock {
                    title: qsTr("30 days")
                    value: "$" + root.formatCost(card.provider.last30DaysCostUSD || 0)
                    detail: root.fmtNum(card.provider.last30DaysTokens || 0) + " " + qsTr("tokens")
                    Layout.fillWidth: true
                }

                MiniBars {
                    Layout.preferredWidth: 180
                    Layout.maximumWidth: 180
                    Layout.fillHeight: true
                    daily: root.lastItems(card.provider.daily || [], 30)
                    tintColor: card.accentColor
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 22
                visible: card.expanded && card.canExpand
                spacing: 6

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: AppTheme.surfaceBorder
                }

                Label {
                    Layout.fillWidth: true
                    visible: card.detailLoading
                    text: qsTr("Loading model breakdown")
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeSm
                    elide: Text.ElideRight
                }

                Repeater {
                    model: card.detailModels

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 24
                        spacing: 10

                        Label {
                            Layout.fillWidth: true
                            text: modelData.name || qsTr("Unknown model")
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fontSizeSm
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.preferredWidth: 92
                            text: "$" + root.formatCost(modelData.costUSD || 0)
                            color: AppTheme.textSecondary
                            font.pixelSize: AppTheme.fontSizeSm
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.preferredWidth: 100
                            text: root.fmtNum(modelData.tokens || 0) + " " + qsTr("tokens")
                            color: AppTheme.textTertiary
                            font.pixelSize: AppTheme.fontSizeSm
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    component StatBlock: Item {
        id: stat
        property string title: ""
        property string value: ""
        property string detail: ""

        implicitHeight: 46
        clip: true

        Column {
            anchors.fill: parent
            spacing: 2

            Text {
                width: parent.width
                text: stat.title
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: stat.value
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                minimumPixelSize: 10
                fontSizeMode: Text.HorizontalFit
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: stat.detail
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }
        }
    }
}

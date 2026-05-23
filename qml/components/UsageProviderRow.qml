import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property var provider: ({})
    property var providerDetail: ({})
    property string providerId: ""
    property string providerName: ""
    property color accentColor: AppTheme.providerBrandColor(effectiveProviderId)
    property string summary: ""
    property string subtitleText: ""
    property string kindText: ""
    property string status: "ok"
    property bool expanded: false
    property bool canExpand: false
    property bool providerEnabled: true
    property bool showStats: true
    property string todayText: ""
    property string periodText: ""
    property string tokenText: ""
    property var trendValues: []

    readonly property string effectiveProviderId: {
        var row = provider || {}
        return row.providerId || providerId
    }
    readonly property string effectiveProviderName: {
        var row = provider || {}
        return providerName || row.displayName || row.providerId || providerId
    }
    readonly property bool effectiveProviderEnabled: {
        var row = provider || {}
        return providerEnabled && row.enabled !== false
    }
    readonly property bool hasTokenData: {
        var row = provider || {}
        return row.hasTokenData === true
    }
    readonly property var detailModels: providerDetail && providerDetail.models ? providerDetail.models : []
    readonly property bool detailLoading: providerDetail && providerDetail.state === "loading"
    readonly property bool detailEmpty: providerDetail && providerDetail.state === "ready" && detailModels.length === 0
    readonly property bool effectiveCanExpand: canExpand
        || (hasTokenData && ((provider && provider.hasDetailAvailable === true) || detailModels.length > 0))
    readonly property string effectiveSummary: summary || tokenText || periodText || usageSummary("last30DaysCostUSD", "last30DaysTokens")
    readonly property string effectiveSubtitle: subtitleText || tokenText || kindText
    readonly property var effectiveTrendValues: trendValues && trendValues.length > 0
        ? trendValues
        : ((provider && provider.daily) ? provider.daily : [])

    signal toggleRequested()

    implicitHeight: contentColumn.implicitHeight + 24
    radius: AppTheme.radiusLg
    interactive: true
    selected: root.expanded
    tone: root.hasTokenData ? "neutral" : "warning"
    opacity: root.effectiveProviderEnabled ? 1 : 0.62
    activeFocusOnTab: root.effectiveCanExpand
    clip: true

    Accessible.role: Accessible.Button
    Accessible.name: effectiveProviderName
        + (kindText !== "" ? ", " + kindText : "")
        + (effectiveSummary !== "" ? ", " + effectiveSummary : "")
    Accessible.description: effectiveCanExpand
        ? (expanded ? qsTr("Model breakdown expanded") : qsTr("Model breakdown collapsed"))
        : effectiveSubtitle

    function requestToggle() {
        if (root.effectiveCanExpand) {
            root.toggleRequested()
        }
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

    function usageSummary(costKey, tokenKey) {
        var row = provider || {}
        if (!row.hasTokenData) return kindText
        return "$" + root.formatCost(row[costKey] || 0)
            + " - " + root.fmtNum(row[tokenKey] || 0) + " " + qsTr("tokens")
    }

    function dailyValue(day) {
        if (!day) return 0
        if (typeof day === "number") return Number(day || 0)
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

    Keys.onReturnPressed: function(event) {
        event.accepted = true
        requestToggle()
    }
    Keys.onEnterPressed: function(event) {
        event.accepted = true
        requestToggle()
    }
    Keys.onSpacePressed: function(event) {
        event.accepted = true
        requestToggle()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 4
        color: root.accentColor
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.effectiveCanExpand
        hoverEnabled: true
        cursorShape: root.effectiveCanExpand ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.requestToggle()
    }

    ColumnLayout {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        anchors.leftMargin: 16
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 34
            spacing: 8

            Text {
                Layout.preferredWidth: 14
                Layout.fillHeight: true
                text: root.effectiveCanExpand ? (root.expanded ? "v" : ">") : ""
                color: root.effectiveCanExpand ? AppTheme.textSecondary : root.accentColor
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ProviderIdentityBadge {
                Layout.preferredWidth: AppTheme.avatarSizeList
                Layout.preferredHeight: AppTheme.avatarSizeList
                Layout.alignment: Qt.AlignVCenter
                size: AppTheme.avatarSizeList
                providerId: root.effectiveProviderId
                displayName: root.effectiveProviderName
                brandColor: root.accentColor
                enabled: root.effectiveProviderEnabled
                selected: root.expanded
                severity: root.status
                context: "normal"
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: 1

                Label {
                    Layout.fillWidth: true
                    text: root.effectiveProviderName
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: text !== ""
                    text: root.effectiveSubtitle
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeXs
                    elide: Text.ElideRight
                }
            }

            StatusPill {
                visible: text !== ""
                text: root.kindText
                toneColor: root.accentColor
            }

            Label {
                Layout.preferredWidth: Math.min(230, Math.max(150, implicitWidth))
                text: root.effectiveSummary
                color: root.status === "warning" ? AppTheme.textPrimary : AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                font.bold: root.status === "warning"
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                visible: text !== ""
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 22
            Layout.preferredHeight: root.hasTokenData && root.showStats ? 50 : 0
            spacing: 12
            visible: root.hasTokenData && root.showStats

            StatBlock {
                title: qsTr("Today")
                value: root.todayText || "$" + root.formatCost(root.provider.sessionCostUSD || 0)
                detail: root.fmtNum(root.provider.sessionTokens || 0) + " " + qsTr("tokens")
                Layout.fillWidth: true
            }

            StatBlock {
                title: qsTr("30 days")
                value: root.periodText || "$" + root.formatCost(root.provider.last30DaysCostUSD || 0)
                detail: root.fmtNum(root.provider.last30DaysTokens || 0) + " " + qsTr("tokens")
                Layout.fillWidth: true
            }

            MiniBars {
                Layout.preferredWidth: 180
                Layout.maximumWidth: 180
                Layout.fillHeight: true
                daily: root.effectiveTrendValues
                tintColor: root.accentColor
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 22
            visible: root.expanded && root.effectiveCanExpand
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: AppTheme.chartGrid
            }

            Label {
                Layout.fillWidth: true
                visible: root.detailLoading
                text: qsTr("Loading model breakdown")
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                visible: root.detailEmpty
                text: qsTr("No model breakdown available")
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }

            Repeater {
                model: root.detailModels

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

    FocusRing {
        anchors.fill: parent
        active: root.activeFocus
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
            color: AppTheme.chartTrack
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
}

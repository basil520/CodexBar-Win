import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ColumnLayout {
    id: root

    property var costData: ({})
    property int tokenProviderCount: 0
    property bool costUsageEnabled: true
    property bool costUsageRefreshing: false

    signal refreshRequested()

    spacing: 10
    implicitHeight: headerRow.implicitHeight + overviewCard.implicitHeight + spacing

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

    function formatUpdatedAt(updatedAt) {
        var prefix = qsTr("Provider and model breakdown")
        if (root.costUsageRefreshing) {
            return prefix + " - " + qsTr("Scanning")
        }
        var timestamp = Number(updatedAt || 0)
        if (timestamp <= 0) {
            return prefix
        }
        return prefix + " - " + qsTr("Updated") + " "
            + new Date(timestamp).toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm")
    }

    RowLayout {
        id: headerRow
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
            text: !root.costUsageEnabled
                ? qsTr("Off")
                : root.costUsageRefreshing ? qsTr("Scanning") : qsTr("Last 30 days")
            toneColor: !root.costUsageEnabled
                ? AppTheme.textTertiary
                : root.costUsageRefreshing ? AppTheme.statusDegraded : AppTheme.statusOk
        }

        ActionButton {
            text: root.costUsageRefreshing ? qsTr("Scanning") : qsTr("Refresh")
            compact: true
            enabled: root.costUsageEnabled && !root.costUsageRefreshing
            onClicked: root.refreshRequested()
        }
    }

    SurfaceCard {
        id: overviewCard
        Layout.fillWidth: true
        Layout.preferredHeight: 166
        implicitHeight: 166
        radius: AppTheme.radiusLg
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
                        var dailyList = root.costData.daily || []
                        if (dailyList.length >= 7) {
                            var items = root.lastItems(dailyList, 7)
                            var pts = []
                            for (var i = 0; i < items.length; ++i) {
                                pts.push(root.dailyValue(items[i]))
                            }
                            return pts
                        }
                        return [32, 48, 42, 58, 52, 68, 62]
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    color: AppTheme.surfaceBorder
                }

                SummaryMetric {
                    title: qsTr("Providers")
                    value: root.tokenProviderCount.toString()
                    detail: qsTr("token sources")
                    accentColor: AppTheme.statusDegraded
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    sparkPoints: [1, 2, 2, 3, 3, 3, Math.max(1, root.tokenProviderCount || 3)]
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
}

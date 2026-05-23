import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: root

    property var displayCostData: ({})
    property var providerCostRows: []
    property bool expanded: false
    property bool costUsageEnabled: true
    property bool costUsageRefreshing: false

    readonly property bool hasData: displayCostData && displayCostData.hasData === true

    signal enableRequested()
    signal toggleExpandedRequested()
    signal openDetailsRequested()

    implicitHeight: costSection.implicitHeight
    color: "transparent"
    clip: true

    function fmtNum(n) {
        var value = Number(n || 0)
        if (value >= 1000000) return (value / 1000000).toFixed(1) + "M"
        if (value >= 1000) return (value / 1000).toFixed(1) + "K"
        return value.toString()
    }

    function formatCost(n) {
        var value = Number(n || 0)
        if (Math.abs(value) >= 1000) return value.toFixed(1)
        if (Math.abs(value) >= 100) return value.toFixed(2)
        return value.toFixed(4)
    }

    function brandColorFor(providerId) {
        return AppTheme.providerBrandColor(providerId)
    }

    function providerLabel(providerId) {
        var names = {
            "codex": "Codex",
            "claude": "Claude",
            "opencodego": "OpenCode Go",
            "qianfan": "QianFan"
        }
        return names[providerId] || providerId
    }

    function usageStateText() {
        if (!root.costUsageEnabled) return qsTr("Token usage is off")
        if (root.costUsageRefreshing) return qsTr("Token usage is scanning")
        if (root.hasData) return qsTr("Token usage data available")
        return qsTr("No token usage data")
    }

    function activateSummary() {
        if (!root.costUsageEnabled) {
            root.enableRequested()
        } else if (root.hasData) {
            root.toggleExpandedRequested()
        }
    }

    ColumnLayout {
        id: costSection
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            radius: AppTheme.radiusMd
            color: costMouse.containsMouse ? AppTheme.surfaceHover : AppTheme.surfacePane
            activeFocusOnTab: true
            Accessible.role: Accessible.Button
            Accessible.name: qsTr("Token Usage")
            Accessible.description: root.usageStateText()

            Keys.onReturnPressed: function(event) {
                event.accepted = true
                root.activateSummary()
            }
            Keys.onEnterPressed: function(event) {
                event.accepted = true
                root.activateSummary()
            }
            Keys.onSpacePressed: function(event) {
                event.accepted = true
                root.activateSummary()
            }

            Behavior on color {
                ColorAnimation {
                    duration: AppTheme.duration(AppTheme.motionFast)
                    easing.type: AppTheme.easeStandard
                }
            }

            MouseArea {
                id: costMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: root.activateSummary()
            }

            FocusRing {
                anchors.fill: parent
                anchors.margins: -2
                radius: parent.radius + 2
                active: parent.activeFocus
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text {
                    Layout.preferredWidth: 12
                    text: root.expanded && root.hasData ? "v" : ">"
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeXs
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Token Usage")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    font.bold: true
                    elide: Text.ElideRight
                }

                Rectangle {
                    Layout.preferredWidth: 8
                    Layout.preferredHeight: 8
                    Layout.alignment: Qt.AlignVCenter
                    radius: 4
                    color: root.costUsageRefreshing ? AppTheme.statusDegraded
                        : root.hasData ? AppTheme.statusOk : AppTheme.statusUnknown

                    SequentialAnimation on opacity {
                        running: root.costUsageRefreshing && !AppTheme.reduceMotion
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 0.3; duration: AppTheme.duration(AppTheme.motionSlow) }
                        NumberAnimation { from: 0.3; to: 1.0; duration: AppTheme.duration(AppTheme.motionSlow) }
                    }
                }

                ActionButton {
                    Layout.preferredHeight: 32
                    text: qsTr("Details")
                    compact: true
                    variant: "ghost"
                    onClicked: {
                        root.enableRequested()
                        root.openDetailsRequested()
                    }
                }

                Text {
                    Layout.maximumWidth: 86
                    text: root.hasData
                        ? "$" + Number(root.displayCostData.sessionCostUSD || 0).toFixed(2) + " " + qsTr("today")
                        : root.costUsageRefreshing ? qsTr("scanning...") : qsTr("no data")
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeXs
                    elide: Text.ElideRight
                }
            }
        }

        ColumnLayout {
            id: costBody
            Layout.fillWidth: true
            visible: root.expanded && root.hasData
            spacing: 8
            Layout.topMargin: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                radius: AppTheme.radiusMd
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
                        value: "$" + root.formatCost(root.displayCostData.sessionCostUSD)
                        detail: root.fmtNum(root.displayCostData.sessionTokens) + " " + qsTr("tokens")
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
                        value: "$" + root.formatCost(root.displayCostData.last30DaysCostUSD)
                        detail: root.fmtNum(root.displayCostData.last30DaysTokens) + " " + qsTr("tokens")
                        valueColor: AppTheme.accentColor
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                radius: AppTheme.radiusMd
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
                        if (root.expanded && root.displayCostData.daily) {
                            for (var i = 0; i < root.displayCostData.daily.length; i++)
                                maxCost = Math.max(maxCost, Number(root.displayCostData.daily[i].costUSD || 0))
                        }
                        return maxCost
                    }

                    Repeater {
                        model: root.expanded && root.displayCostData.daily ? root.displayCostData.daily.slice(-21) : []

                        delegate: Rectangle {
                            width: Math.max(2, parent.width / 21 - 2)
                            height: {
                                var chartHeight = Math.max(2, parent ? parent.height : 44)
                                return dailyBarChart.dailyMaxCost > 0
                                    ? Math.max(2, chartHeight * Number(modelData.costUSD || 0) / dailyBarChart.dailyMaxCost)
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

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Rectangle { width: 10; height: 10; radius: 5; color: AppTheme.accentColor }
                Text { text: qsTr("Mon"); color: AppTheme.textTertiary; font.pixelSize: AppTheme.fontSizeXs }
                Rectangle { width: 1; height: 10; color: AppTheme.surfaceBorder }
                Rectangle { width: 10; height: 10; radius: 5; color: AppTheme.surfaceSelected }
                Text { text: qsTr("other day"); color: AppTheme.textTertiary; font.pixelSize: AppTheme.fontSizeXs }
            }

            Repeater {
                model: root.providerCostRows || []

                delegate: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    property bool providerExpanded: false

                    function toggleProvider() {
                        providerExpanded = !providerExpanded
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 32
                        radius: AppTheme.radiusSm
                        color: providerMouse.containsMouse ? AppTheme.surfaceHover : AppTheme.surfaceControl
                        activeFocusOnTab: true
                        Accessible.role: Accessible.Button
                        Accessible.name: root.providerLabel(modelData.providerId)
                        Accessible.description: providerExpanded
                            ? qsTr("Model breakdown expanded")
                            : qsTr("Model breakdown collapsed")

                        Keys.onReturnPressed: function(event) {
                            event.accepted = true
                            toggleProvider()
                        }
                        Keys.onEnterPressed: function(event) {
                            event.accepted = true
                            toggleProvider()
                        }
                        Keys.onSpacePressed: function(event) {
                            event.accepted = true
                            toggleProvider()
                        }

                        Behavior on color {
                            ColorAnimation {
                                duration: AppTheme.duration(AppTheme.motionFast)
                                easing.type: AppTheme.easeStandard
                            }
                        }

                        MouseArea {
                            id: providerMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: toggleProvider()
                        }

                        FocusRing {
                            anchors.fill: parent
                            anchors.margins: -2
                            radius: parent.radius + 2
                            active: parent.activeFocus
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 6

                            Text {
                                Layout.preferredWidth: 12
                                text: providerExpanded ? "v" : ">"
                                color: AppTheme.textTertiary
                                font.pixelSize: AppTheme.fontSizeXs
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Rectangle {
                                Layout.preferredWidth: 8
                                Layout.preferredHeight: 8
                                radius: 4
                                color: root.brandColorFor(modelData.providerId)
                            }

                            Text {
                                text: root.providerLabel(modelData.providerId)
                                color: AppTheme.textSecondary
                                font.pixelSize: AppTheme.fontSizeXs
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: "$" + root.formatCost(modelData.last30DaysCostUSD)
                                color: AppTheme.textTertiary
                                font.pixelSize: AppTheme.fontSizeXs
                            }

                            Text {
                                text: root.fmtNum(modelData.last30DaysTokens) + " " + qsTr("tokens")
                                color: AppTheme.textInverse
                                font.pixelSize: 9
                            }
                        }
                    }

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
                                    text: "-"
                                    color: AppTheme.textDisabled
                                    font.pixelSize: 9
                                }

                                Text {
                                    text: modelData.name
                                    color: AppTheme.textSecondary
                                    font.pixelSize: AppTheme.fontSizeXs
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: "$" + root.formatCost(modelData.costUSD)
                                    color: AppTheme.textTertiary
                                    font.pixelSize: 9
                                }

                                Text {
                                    text: root.fmtNum(modelData.tokens) + " tk"
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
                font.pixelSize: AppTheme.fontSizeXs
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
                font.pixelSize: AppTheme.fontSizeXs
                elide: Text.ElideRight
            }
        }
    }
}

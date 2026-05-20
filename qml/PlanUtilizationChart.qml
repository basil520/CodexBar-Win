import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0

Rectangle {
    id: chartRoot
    property string providerId
    property string currentSeries: "session"
    property var chartPoints: []
    property bool hasTertiarySeries: false
    property string tertiarySeriesKey: "opus"
    property string tertiarySeriesLabel: qsTr("Opus")
    property string noDataText: qsTr("No session utilization data yet.")
    property int rev: LanguageManager.translationRevision
    property int dataRevision: 0
    property bool ready: false

    property real animationFactor: 0.0
    Behavior on animationFactor {
        NumberAnimation {
            duration: 350
            easing.type: Easing.OutCubic
        }
    }

    color: AppTheme.surfaceChart
    radius: 8
    height: 130
    implicitHeight: 130

    function seriesModel() {
        var items = [
            { key: "session", label: qsTr("Session") },
            { key: "weekly", label: qsTr("Weekly") }
        ];

        if (chartRoot.hasTertiarySeries) {
            items.push({
                key: chartRoot.tertiarySeriesKey,
                label: chartRoot.tertiarySeriesLabel || qsTr("Opus")
            });
        }

        return items;
    }

    function ensureValidSeries() {
        var items = seriesModel();
        for (var i = 0; i < items.length; i++) {
            if (items[i].key === chartRoot.currentSeries) {
                return;
            }
        }
        chartRoot.currentSeries = items.length > 0 ? items[0].key : "session";
    }

    function refreshChart() {
        if (!chartRoot.ready) {
            return;
        }

        ensureValidSeries();
        chartRoot.chartPoints = UsageStore.utilizationChartData(chartRoot.providerId, chartRoot.currentSeries);
        
        // Reset and trigger heights physical grow animation
        chartRoot.animationFactor = 0.0;
        chartRoot.animationFactor = 1.0;
        
        chartCanvas.requestPaint();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Repeater {
                model: {
                    chartRoot.rev
                    chartRoot.hasTertiarySeries
                    chartRoot.tertiarySeriesKey
                    chartRoot.tertiarySeriesLabel
                    return chartRoot.seriesModel()
                }
                delegate: Rectangle {
                    property bool isSelected: chartRoot.currentSeries === modelData.key
                    width: seriesLabel.width + 12
                    height: 20
                    radius: 4
                    color: isSelected ? AppTheme.surfaceSelected : "transparent"
                    border.color: isSelected ? AppTheme.surfaceAccentBorder : "transparent"

                    Text {
                        id: seriesLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        color: isSelected ? AppTheme.textPrimary : AppTheme.textTertiary
                        font.pixelSize: 10
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            chartRoot.currentSeries = modelData.key
                            chartRoot.refreshChart()
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: chartPoints.length > 0 ? qsTr("%1 pts").arg(chartPoints.length) : qsTr("No data")
                color: AppTheme.textInverse
                font.pixelSize: 9
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Canvas {
                id: chartCanvas
                anchors.fill: parent

                property real animFactor: chartRoot.animationFactor
                onAnimFactorChanged: requestPaint()

                function drawRoundedRect(ctx, x, y, width, height, radius) {
                    ctx.beginPath();
                    var realY = height < 0 ? y + height : y;
                    var realHeight = Math.abs(height);
                    if (realHeight < radius * 2) {
                        radius = realHeight / 2;
                    }
                    if (width < radius * 2) {
                        radius = width / 2;
                    }
                    ctx.moveTo(x + radius, realY);
                    ctx.lineTo(x + width - radius, realY);
                    ctx.quadraticCurveTo(x + width, realY, x + width, realY + radius);
                    ctx.lineTo(x + width, realY + realHeight - radius);
                    ctx.quadraticCurveTo(x + width, realY + realHeight, x + width - radius, realY + realHeight);
                    ctx.lineTo(x + radius, realY + realHeight);
                    ctx.quadraticCurveTo(x, realY + realHeight, x, realY + realHeight - radius);
                    ctx.lineTo(x, realY + radius);
                    ctx.quadraticCurveTo(x, realY, x + radius, realY);
                    ctx.closePath();
                }

                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    ctx.fillStyle = AppTheme.surfaceChart;
                    ctx.fillRect(0, 0, width, height);

                    if (chartPoints.length === 0) {
                        ctx.fillStyle = AppTheme.textDisabled;
                        ctx.font = "10px sans-serif";
                        ctx.textAlign = "center";
                        ctx.fillText(chartRoot.noDataText, width / 2, height / 2);
                        return;
                    }

                    var barWidth = Math.max(2, (width - 20) / chartPoints.length - 2);
                    var startX = 10;

                    for (var i = 0; i < chartPoints.length; i++) {
                        var pct = chartPoints[i].usedPercent;
                        // Animate heights using the growth factor
                        var barHeight = Math.max(1, (pct / 100.0) * (height - 20) * chartRoot.animationFactor);

                        // Draw track using rounded corners
                        ctx.fillStyle = AppTheme.surfaceTrack;
                        drawRoundedRect(ctx, startX + i * (barWidth + 2), height - 10, barWidth, -(height - 20), 2);
                        ctx.fill();

                        // Create soft vertical linear gradient for active bars
                        var baseColor = AppTheme.statusOk;
                        if (pct > 80) baseColor = AppTheme.statusOutage;
                        else if (pct > 60) baseColor = AppTheme.statusDegraded;

                        var grad = ctx.createLinearGradient(0, height - 10, 0, height - 10 - barHeight);
                        grad.addColorStop(0.0, baseColor);
                        grad.addColorStop(1.0, Qt.lighter(baseColor, 1.25));

                        ctx.fillStyle = grad;
                        drawRoundedRect(ctx, startX + i * (barWidth + 2), height - 10, barWidth, -barHeight, 2);
                        ctx.fill();
                    }

                    ctx.fillStyle = AppTheme.textInverse;
                    ctx.font = "8px sans-serif";
                    ctx.textAlign = "left";
                    ctx.fillText("100%", 0, 12);
                    ctx.fillText("0%", 0, height - 4);
                }
            }

            Rectangle {
                id: hoverTooltip
                visible: false
                width: tooltipLabel.implicitWidth + 16
                height: 22
                radius: 6
                color: AppTheme.surfacePopup
                border.color: AppTheme.surfaceAccentBorder
                border.width: 1
                z: 50

                Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }
                Behavior on y { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }

                Label {
                    id: tooltipLabel
                    anchors.centerIn: parent
                    color: AppTheme.textPrimary
                    font.pixelSize: 10
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onPositionChanged: function(mouse) {
                    if (chartPoints.length === 0) return;
                    var barWidth = Math.max(2, (chartCanvas.width - 20) / chartPoints.length - 2);
                    var idx = Math.floor((mouse.x - 10) / (barWidth + 2));
                    if (idx >= 0 && idx < chartPoints.length) {
                        var labelText = qsTr("%1: %2% used")
                            .arg(chartPoints[idx].dateLabel)
                            .arg(chartPoints[idx].usedPercent.toFixed(1));
                        
                        tooltipLabel.text = labelText;
                        hoverTooltip.visible = true;
                        
                        var targetX = 10 + idx * (barWidth + 2) + barWidth / 2 - hoverTooltip.width / 2;
                        hoverTooltip.x = Math.max(2, Math.min(chartCanvas.width - hoverTooltip.width - 2, targetX));
                        
                        var pct = chartPoints[idx].usedPercent;
                        var barHeight = Math.max(1, (pct / 100.0) * (chartCanvas.height - 20) * chartRoot.animationFactor);
                        hoverTooltip.y = chartCanvas.height - 10 - barHeight - hoverTooltip.height - 4;
                    } else {
                        hoverTooltip.visible = false;
                    }
                }
                onExited: {
                    hoverTooltip.visible = false;
                }
            }
        }
    }

    Component.onCompleted: {
        ready = true
        refreshChart()
    }

    onProviderIdChanged: {
        refreshChart()
    }

    onHasTertiarySeriesChanged: {
        refreshChart()
    }

    onTertiarySeriesKeyChanged: {
        refreshChart()
    }

    onTertiarySeriesLabelChanged: {
        refreshChart()
    }

    onDataRevisionChanged: {
        refreshChart()
    }

    Connections {
        enabled: chartRoot.visible
        target: UsageStore
        function onSnapshotRevisionChanged() {
            chartRoot.refreshChart()
        }
    }

    Connections {
        enabled: chartRoot.visible
        target: LanguageManager
        function onTranslationRevisionChanged() {
            chartRoot.ensureValidSeries()
            hoverDetail.text = ""
            chartCanvas.requestPaint()
        }
    }
}

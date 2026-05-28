import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import "components" as Components

Components.ChartFrame {
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
    property bool refreshing: false
    property color accentColor: AppTheme.accentColor
    property bool hoverDetailEnabled: true

    property real animationFactor: 0.0
    Behavior on animationFactor {
        NumberAnimation {
            duration: AppTheme.duration(AppTheme.motionPanel)
            easing.type: Easing.OutCubic
        }
    }

    height: 160
    implicitHeight: 160
    empty: chartPoints.length === 0
    emptyText: chartRoot.noDataText
    loading: chartRoot.refreshing

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
        
        if (PerformanceState.anyUiVisible) {
            // Reset and trigger heights physical grow animation
            chartRoot.animationFactor = 0.0;
            chartRoot.animationFactor = 1.0;
            chartCanvas.requestVisiblePaint();
        } else {
            chartRoot.animationFactor = 1.0;
        }
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
                visible: PerformanceState.anyUiVisible
                onAnimFactorChanged: requestVisiblePaint()
                onWidthChanged: requestVisiblePaint()
                onHeightChanged: requestVisiblePaint()

                function requestVisiblePaint() {
                    if (PerformanceState.anyUiVisible && width > 0 && height > 0) {
                        requestPaint()
                    }
                }

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
                    if (!PerformanceState.anyUiVisible) return;

                    if (chartPoints.length === 0) {
                        ctx.fillStyle = AppTheme.textDisabled;
                        ctx.font = "10px sans-serif";
                        ctx.textAlign = "center";
                        ctx.fillText(chartRoot.noDataText, width / 2, height / 2);
                        return;
                    }

                    var paddingLeft = 28;
                    var paddingRight = 8;
                    var paddingTop = 10;
                    var paddingBottom = 16;

                    var renderWidth = width - paddingLeft - paddingRight;
                    var renderHeight = height - paddingTop - paddingBottom;

                    ctx.strokeStyle = AppTheme.chartGrid;
                    ctx.lineWidth = 1;
                    ctx.beginPath();
                    ctx.moveTo(paddingLeft, paddingTop);
                    ctx.lineTo(width - paddingRight, paddingTop);
                    ctx.moveTo(paddingLeft, height - paddingBottom);
                    ctx.lineTo(width - paddingRight, height - paddingBottom);
                    ctx.stroke();

                    var pointsCount = chartPoints.length;
                    var gap = 4;
                    var barWidth = (renderWidth - (pointsCount - 1) * gap) / pointsCount;
                    if (barWidth < 2) {
                        gap = 2;
                        barWidth = (renderWidth - (pointsCount - 1) * gap) / pointsCount;
                    }
                    if (barWidth < 2) {
                        barWidth = 2;
                    }

                    for (var i = 0; i < pointsCount; i++) {
                        var pct = chartPoints[i].usedPercent;
                        // Animate heights using the growth factor
                        var barHeight = Math.max(1, (pct / 100.0) * renderHeight * chartRoot.animationFactor);
                        var startX = paddingLeft + i * (barWidth + gap);

                        // Draw track using rounded corners
                        ctx.fillStyle = AppTheme.chartTrack;
                        drawRoundedRect(ctx, startX, height - paddingBottom, barWidth, -renderHeight, 2);
                        ctx.fill();

                        // Create soft vertical linear gradient for active bars
                        var baseColor = AppTheme.statusOk;
                        if (pct > 80) baseColor = AppTheme.statusOutage;
                        else if (pct > 60) baseColor = AppTheme.statusDegraded;

                        var grad = ctx.createLinearGradient(0, height - paddingBottom, 0, height - paddingBottom - barHeight);
                        grad.addColorStop(0.0, baseColor);
                        grad.addColorStop(1.0, Qt.lighter(baseColor, 1.25));

                        ctx.fillStyle = grad;
                        drawRoundedRect(ctx, startX, height - paddingBottom, barWidth, -barHeight, 2);
                        ctx.fill();
                    }

                    ctx.fillStyle = AppTheme.chartAxis;
                    ctx.font = "8px sans-serif";
                    ctx.textAlign = "right";
                    ctx.fillText("100%", paddingLeft - 6, paddingTop + 8);
                    ctx.fillText("0%", paddingLeft - 6, height - paddingBottom - 2);
                }
            }

            Components.ChartHoverDetail {
                id: hoverDetail
                floating: true
                accentColor: chartRoot.accentColor
                visible: chartRoot.hoverDetailEnabled && chartMouse.hoveredIndex >= 0 && chartMouse.activePoint !== null
                width: implicitWidth
                height: implicitHeight
                x: Math.max(4, Math.min(chartMouse.hoverX + 10, parent.width - width - 4))
                y: {
                    var above = chartMouse.hoverY - height - 10
                    if (above >= 4) return above
                    return Math.max(4, Math.min(chartMouse.hoverY + 12, parent.height - height - 4))
                }
                primaryText: chartMouse.activePoint
                    ? qsTr("%1: %2% used")
                        .arg(chartMouse.activePoint.dateLabel)
                        .arg(Number(chartMouse.activePoint.usedPercent || 0).toFixed(1))
                    : ""
                secondaryText: chartRoot.currentSeries === "weekly"
                    ? qsTr("Weekly utilization")
                    : chartRoot.currentSeries === "session"
                        ? qsTr("Session utilization")
                        : chartRoot.tertiarySeriesLabel
            }

            MouseArea {
                id: chartMouse
                anchors.fill: parent
                hoverEnabled: true
                property int hoveredIndex: -1
                property real hoverX: 0
                property real hoverY: 0
                property var activePoint: null
                onPositionChanged: function(mouse) {
                    hoverX = mouse.x;
                    hoverY = mouse.y;
                    if (chartPoints.length === 0) {
                        hoveredIndex = -1;
                        activePoint = null;
                        return;
                    }

                    var paddingLeft = 28;
                    var paddingRight = 8;
                    var paddingTop = 10;
                    var paddingBottom = 16;

                    var renderWidth = chartCanvas.width - paddingLeft - paddingRight;
                    var renderHeight = chartCanvas.height - paddingTop - paddingBottom;

                    var pointsCount = chartPoints.length;
                    var gap = 4;
                    var barWidth = (renderWidth - (pointsCount - 1) * gap) / pointsCount;
                    if (barWidth < 2) {
                        gap = 2;
                        barWidth = (renderWidth - (pointsCount - 1) * gap) / pointsCount;
                    }
                    if (barWidth < 2) {
                        barWidth = 2;
                    }

                    var idx = Math.floor((mouse.x - paddingLeft) / (barWidth + gap));
                    if (idx >= 0 && idx < chartPoints.length) {
                        hoveredIndex = idx;
                        activePoint = chartPoints[idx];
                    } else {
                        hoveredIndex = -1;
                        activePoint = null;
                    }
                }
                onExited: {
                    hoveredIndex = -1;
                    activePoint = null;
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
            tooltipLabel.text = ""
            chartCanvas.requestVisiblePaint()
        }
    }

    Connections {
        target: PerformanceState
        function onAnyUiVisibleChanged() {
            if (PerformanceState.anyUiVisible) {
                chartCanvas.requestVisiblePaint()
            }
        }
    }
}

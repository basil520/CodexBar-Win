import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."
import "../components"

SettingsPage {
    title: qsTr("Display")
    subtitle: qsTr("Tune how usage and tray state are presented.")

    SettingsGroupBox {
        RowLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: qsTr("Theme")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                }

                Text {
                    text: qsTr("Choose the overall color style.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                }
            }

            SettingsComboBox {
                Layout.preferredWidth: 180
                model: [
                    { value: 0, label: qsTr("Dark") },
                    { value: 1, label: qsTr("Midnight Blue") },
                    { value: 2, label: qsTr("Amethyst") }
                ]
                selectedValue: SettingsStore.theme
                onValueActivated: function(value) {
                    SettingsStore.theme = value
                }
            }
        }

        SettingsToggleRow {
            title: qsTr("Glass Effect")
            subtitle: qsTr("Use native Windows acrylic blur behind app windows.")
            checked: SettingsStore.glassEffectEnabled
            onToggled: function(checked) {
                SettingsStore.glassEffectEnabled = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            enabled: SettingsStore.glassEffectEnabled
            opacity: enabled ? 1.0 : 0.45
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingXs

                Label {
                    text: qsTr("Glass Opacity")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Label {
                    text: qsTr("Lower values make the glass more transparent.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Slider {
                Layout.preferredWidth: 220
                from: 5
                to: 95
                stepSize: 5
                snapMode: Slider.SnapAlways
                live: true
                value: SettingsStore.glassEffectOpacity
                onMoved: SettingsStore.glassEffectOpacity = Math.round(value)
            }

            Label {
                Layout.preferredWidth: 42
                text: SettingsStore.glassEffectOpacity + "%"
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Merge Icons")
            subtitle: qsTr("Show a single combined tray icon for enabled providers.")
            checked: SettingsStore.mergeIcons
            onToggled: function(checked) {
                SettingsStore.mergeIcons = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Show Usage Amount Used")
            subtitle: qsTr("Use consumed percentage instead of remaining percentage.")
            checked: SettingsStore.usageBarsShowUsed
            onToggled: function(checked) {
                SettingsStore.usageBarsShowUsed = checked
            }
        }
    }

    SettingsGroupBox {
        SettingsToggleRow {
            title: qsTr("Show Absolute Reset Times")
            subtitle: qsTr("Display exact reset times instead of relative wording.")
            checked: SettingsStore.resetTimesShowAbsolute
            onToggled: function(checked) {
                SettingsStore.resetTimesShowAbsolute = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Optional Credits and Extra Usage")
            subtitle: qsTr("Show additional provider-specific credit and usage fields.")
            checked: SettingsStore.showOptionalCreditsAndExtraUsage
            onToggled: function(checked) {
                SettingsStore.showOptionalCreditsAndExtraUsage = checked
            }
        }

        SettingsToggleRow {
            title: qsTr("Claude Peak Hours")
            subtitle: qsTr("Show peak hours indicator for Claude usage pricing.")
            checked: SettingsStore.claudePeakHoursEnabled
            onToggled: function(checked) {
                SettingsStore.claudePeakHoursEnabled = checked
            }
        }
    }

    SettingsSectionHeader {
        text: qsTr("Taskbar & Tray Icon Customization")
    }

    SettingsGroupBox {
        RowLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: qsTr("Tray Display Mode")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Text {
                    text: qsTr("Change how usage info is presented in the system taskbar.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                }
            }

            SettingsComboBox {
                Layout.preferredWidth: 180
                model: [
                    { value: 0, label: qsTr("Icon Only") },
                    { value: 1, label: qsTr("Percentage") },
                    { value: 2, label: qsTr("Remaining Time") },
                    { value: 3, label: qsTr("Custom Time") }
                ]
                selectedValue: SettingsStore.trayDisplayMode
                onValueActivated: function(value) {
                    SettingsStore.trayDisplayMode = value
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingXs

                Label {
                    text: qsTr("Warning Threshold")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Label {
                    text: qsTr("Show warnings when usage remaining drops below this level.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Slider {
                id: warningSlider
                Layout.preferredWidth: 220
                from: 5
                to: 95
                stepSize: 5
                snapMode: Slider.SnapAlways
                live: true
                value: SettingsStore.warningThreshold
                onMoved: SettingsStore.warningThreshold = Math.round(value)
            }

            Label {
                Layout.preferredWidth: 42
                text: SettingsStore.warningThreshold + "%"
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignRight
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingXs

                Label {
                    text: qsTr("Critical Threshold")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Label {
                    text: qsTr("Show alert warning when usage remaining drops below this level.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Slider {
                id: criticalSlider
                Layout.preferredWidth: 220
                from: 1
                to: 50
                stepSize: 1
                snapMode: Slider.SnapAlways
                live: true
                value: SettingsStore.criticalThreshold
                onMoved: SettingsStore.criticalThreshold = Math.round(value)
            }

            Label {
                Layout.preferredWidth: 42
                text: SettingsStore.criticalThreshold + "%"
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    SettingsSectionHeader {
        text: qsTr("Real-Time Preview")
    }

    SettingsGroupBox {
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            spacing: AppTheme.spacingMd

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    text: qsTr("Simulated Usage Level")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }

                Label {
                    text: qsTr("Drag to simulate different remaining capacity levels.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                }
            }

            Slider {
                id: simUsageSlider
                Layout.preferredWidth: 220
                from: 0
                to: 100
                stepSize: 1
                value: 78
                live: true
            }

            Label {
                Layout.preferredWidth: 42
                text: Math.round(simUsageSlider.value) + "%"
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                horizontalAlignment: Text.AlignRight
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#0a0a0f"
            radius: 8
            border.width: 1
            border.color: "#1e1e2d"
            clip: true

            Rectangle {
                anchors.fill: parent
                opacity: 0.05
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#ffffff" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }

            Label {
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Simulated Windows Taskbar")
                color: "#6c6c80"
                font.pixelSize: 12
                font.bold: true
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                height: 32

                Item {
                    width: 24
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = "#a0a0b0";
                            ctx.lineWidth = 1.5;
                            ctx.beginPath();
                            ctx.moveTo(8, 14);
                            ctx.lineTo(12, 10);
                            ctx.lineTo(16, 14);
                            ctx.stroke();
                        }
                    }
                }

                Item {
                    width: 24
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = "#a0a0b0";
                            ctx.fillStyle = "#a0a0b0";
                            ctx.lineWidth = 1.5;
                            
                            ctx.beginPath();
                            ctx.arc(12, 16, 1, 0, 2 * Math.PI);
                            ctx.fill();
                            
                            ctx.beginPath();
                            ctx.arc(12, 16, 4, 1.25 * Math.PI, 1.75 * Math.PI);
                            ctx.stroke();
                            
                            ctx.beginPath();
                            ctx.arc(12, 16, 7, 1.25 * Math.PI, 1.75 * Math.PI);
                            ctx.stroke();
                        }
                    }
                }

                Item {
                    width: 24
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = "#a0a0b0";
                            ctx.fillStyle = "#a0a0b0";
                            ctx.lineWidth = 1.5;

                            ctx.beginPath();
                            ctx.moveTo(6, 10);
                            ctx.lineTo(9, 10);
                            ctx.lineTo(13, 6);
                            ctx.lineTo(13, 18);
                            ctx.lineTo(9, 14);
                            ctx.lineTo(6, 14);
                            ctx.closePath();
                            ctx.fill();

                            ctx.beginPath();
                            ctx.arc(12, 12, 4, -0.3 * Math.PI, 0.3 * Math.PI);
                            ctx.stroke();
                            ctx.beginPath();
                            ctx.arc(12, 12, 7, -0.3 * Math.PI, 0.3 * Math.PI);
                            ctx.stroke();
                        }
                    }
                }

                Item {
                    width: 28
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Rectangle {
                        x: 2
                        y: 7
                        width: 18
                        height: 10
                        color: "transparent"
                        border.color: "#a0a0b0"
                        border.width: 1.5
                        radius: 1
                        
                        Rectangle {
                            x: 2
                            y: 2
                            width: 12 * 0.8
                            height: 6
                            color: "#a0a0b0"
                            radius: 0.5
                        }
                    }
                    
                    Rectangle {
                        x: 20
                        y: 10
                        width: 2
                        height: 4
                        color: "#a0a0b0"
                        radius: 0.5
                    }
                }

                Item {
                    id: simTrayIcon
                    width: 24
                    height: 24
                    anchors.verticalCenter: parent.verticalCenter

                    property double pRem: simUsageSlider.value / 100.0
                    property double wRem: 0.85
                    property double pPct: simUsageSlider.value
                    property double warningTh: SettingsStore.warningThreshold
                    property double criticalTh: SettingsStore.criticalThreshold
                    property int displayMode: SettingsStore.trayDisplayMode

                    property color primaryColor: pPct > warningTh ? "#40c840" :
                                                 pPct > criticalTh ? "#dcb428" :
                                                 "#dc3c3c"
                    property color weeklyColor: "#40a0c8"
                    property color trackColor: "#3c3c50"

                    Rectangle {
                        anchors.fill: parent
                        visible: simTrayIcon.displayMode !== 0
                        color: "#2d2d3d"
                        radius: 5
                        border.width: 1
                        border.color: "#3e3e52"

                        Label {
                            anchors.centerIn: parent
                            font.pixelSize: 8
                            font.bold: true
                            color: simTrayIcon.primaryColor
                            text: {
                                if (simTrayIcon.displayMode === 1) {
                                    return Math.round(simTrayIcon.pPct) + "%"
                                } else {
                                    var hours = simTrayIcon.pRem * 24.0
                                    if (hours >= 1.0) {
                                        return Math.round(hours) + "h"
                                    } else {
                                        return Math.max(1, Math.round(hours * 60.0)) + "m"
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        anchors.fill: parent
                        visible: simTrayIcon.displayMode === 0

                        Rectangle {
                            x: 0
                            y: 6
                            width: parent.width
                            height: 3
                            color: simTrayIcon.trackColor
                            radius: 1
                            
                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.width * simTrayIcon.pRem
                                color: simTrayIcon.primaryColor
                                radius: 1
                            }
                        }

                        Rectangle {
                            x: 0
                            y: 13
                            width: parent.width
                            height: 2
                            color: simTrayIcon.trackColor
                            radius: 1

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.width * simTrayIcon.wRem
                                color: simTrayIcon.weeklyColor
                                radius: 1
                            }
                        }
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 1
                    
                    Label {
                        text: "14:44"
                        color: "#a0a0b0"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        text: "2026/05/20"
                        color: "#707080"
                        font.pixelSize: 8
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Rectangle {
                    width: 4
                    height: 32
                    color: "transparent"
                }
            }
        }
    }
}

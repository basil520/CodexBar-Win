import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    id: root

    property real simulatedUsage: simUsageSlider.value
    property color capacityColor: simulatedUsage > SettingsStore.warningThreshold
        ? AppTheme.statusOk
        : simulatedUsage > SettingsStore.criticalThreshold
            ? AppTheme.statusDegraded
            : AppTheme.statusOutage

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
                Layout.fillWidth: true
                text: qsTr("Drag to preview tray, card, and chart states with the current theme.")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
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
            text: Math.round(root.simulatedUsage) + "%"
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignRight
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 126
        radius: AppTheme.radiusMd
        color: AppTheme.surfacePreview
        border.width: 1
        border.color: AppTheme.surfaceBorder
        clip: true

        Rectangle {
            anchors.fill: parent
            color: AppTheme.surfaceScrim
            opacity: SettingsStore.glassEffectEnabled ? 0.28 : 0.0
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: AppTheme.spacingLg

            Components.ProviderAvatar {
                Layout.preferredWidth: AppTheme.avatarSizeHero
                Layout.preferredHeight: AppTheme.avatarSizeHero
                size: AppTheme.avatarSizeHero
                providerId: "codex"
                displayName: "Codex"
                brandColor: AppTheme.providerBrandColor("codex")
                selected: true
                showProgressRing: true
                usagePercent: root.simulatedUsage
                severity: root.simulatedUsage <= SettingsStore.criticalThreshold ? "error"
                    : root.simulatedUsage <= SettingsStore.warningThreshold ? "warning"
                    : "ok"
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingSm

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Preview Provider")
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeLg
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Glass, theme, thresholds, and tray display use the same tokens as production UI.")
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: AppTheme.progressBarHeight
                    radius: height / 2
                    color: AppTheme.chartTrack

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * root.simulatedUsage / 100
                        radius: parent.radius
                        color: root.capacityColor
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 96
                Layout.fillHeight: true
                radius: AppTheme.radiusSm
                color: AppTheme.surfaceControl
                border.width: 1
                border.color: AppTheme.surfaceBorder

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: AppTheme.spacingSm

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Tray")
                        color: AppTheme.textTertiary
                        font.pixelSize: AppTheme.fontSizeXs
                    }

                    Label {
                        Layout.fillWidth: true
                        text: SettingsStore.trayDisplayMode === 1
                            ? Math.round(root.simulatedUsage) + "%"
                            : SettingsStore.trayDisplayMode === 0
                                ? qsTr("Icon")
                                : qsTr("Ready")
                        color: root.capacityColor
                        font.pixelSize: AppTheme.fontSizeLg
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 3
                        radius: 2
                        color: AppTheme.chartTrack

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * root.simulatedUsage / 100
                            radius: parent.radius
                            color: root.capacityColor
                        }
                    }
                }
            }
        }
    }
}

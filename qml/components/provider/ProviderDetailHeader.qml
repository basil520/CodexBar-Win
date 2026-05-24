import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    id: root
    objectName: "providerDetailHeader"

    property string providerId: ""
    property var descriptor: ({})
    property var providerStatus: ({ "state": "unknown" })
    property var connectionTest: ({ "state": "idle" })
    property string providerError: ""
    property color brandColor: descriptor && descriptor.brandColor ? descriptor.brandColor : AppTheme.accentColor

    signal dashboardRequested(string url)
    signal statusRequested(string url)
    signal refreshRequested()
    signal testConnectionRequested()
    signal enabledToggled(bool enabled)

    readonly property string displayName: descriptor && descriptor.displayName ? descriptor.displayName : providerId
    readonly property bool providerEnabled: descriptor ? descriptor.enabled !== false : true
    readonly property string statusState: providerError !== "" ? "outage"
        : (providerStatus && providerStatus.state ? providerStatus.state : "unknown")
    readonly property string connectionState: connectionTest && connectionTest.state ? connectionTest.state : "idle"
    readonly property string dashboardUrl: descriptor && descriptor.dashboardURL ? descriptor.dashboardURL : ""
    readonly property string statusUrl: descriptor && descriptor.statusURL ? descriptor.statusURL : ""
    readonly property var sourceModes: descriptor && descriptor.sourceModes ? descriptor.sourceModes : []

    function statusText(state) {
        if (root.providerError !== "") return qsTr("Needs attention")
        if (state === "ok") return qsTr("Operational")
        if (state === "degraded") return qsTr("Degraded")
        if (state === "outage") return qsTr("Outage")
        return qsTr("Unknown")
    }

    function statusColor(state) {
        if (root.providerError !== "") return AppTheme.statusOutage
        if (state === "ok" || state === "succeeded") return AppTheme.statusOk
        if (state === "degraded" || state === "testing") return AppTheme.statusDegraded
        if (state === "outage" || state === "failed") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    function sourceSummary() {
        if (sourceModes.length > 0) return qsTr("Sources: ") + sourceModes.join(", ")
        return qsTr("Descriptor-driven provider settings")
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 14

        Components.ProviderIdentityBadge {
            visible: root.width >= 420
            Layout.preferredWidth: visible ? 48 : 0
            Layout.preferredHeight: visible ? 48 : 0
            Layout.alignment: Qt.AlignTop
            size: 48
            providerId: root.providerId
            displayName: root.displayName
            brandColor: root.brandColor
            selected: true
            enabled: root.providerEnabled
            severity: root.providerError !== "" ? "error" : "none"
            context: "hero"
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            spacing: 8

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Label {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    text: root.displayName
                    color: AppTheme.textPrimary
                    font.pixelSize: 24
                    font.bold: true
                    elide: Text.ElideRight
                }

                Components.StatusPill {
                    text: root.statusText(root.statusState)
                    toneColor: root.statusColor(root.statusState)
                    Layout.alignment: Qt.AlignLeft
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: root.sourceSummary()
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.width < 560 ? 1 : 2
                columnSpacing: 10
                rowSpacing: 8

                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Components.ActionButton {
                        compact: true
                        text: qsTr("Dashboard")
                        visible: root.dashboardUrl !== ""
                        enabled: visible
                        onClicked: root.dashboardRequested(root.dashboardUrl)
                    }

                    Components.ActionButton {
                        compact: true
                        text: qsTr("Status")
                        visible: root.statusUrl !== ""
                        enabled: visible
                        onClicked: root.statusRequested(root.statusUrl)
                    }

                    Components.ActionButton {
                        compact: true
                        text: qsTr("Refresh")
                        onClicked: root.refreshRequested()
                    }

                    Components.ActionButton {
                        compact: true
                        variant: "primary"
                        busy: root.connectionState === "testing"
                        text: root.connectionState === "testing" ? qsTr("Testing") : qsTr("Test Connection")
                        enabled: root.connectionState !== "testing"
                        onClicked: root.testConnectionRequested()
                    }
                }

                RowLayout {
                    Layout.alignment: root.width < 560 ? Qt.AlignLeft : Qt.AlignRight
                    spacing: 8

                    Label {
                        text: qsTr("Enabled")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                    }

                    Components.SettingsSwitch {
                        accessibleName: qsTr("Provider enabled")
                        checked: root.providerEnabled
                        onToggled: function(checked) {
                            root.enabledToggled(checked)
                        }
                    }
                }
            }
        }
    }
}

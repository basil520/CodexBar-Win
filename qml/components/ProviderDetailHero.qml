import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SettingsGroupBox {
    id: root

    property string providerId: ""
    property var descriptor: ({})
    property var providerStatus: ({ "state": "unknown" })
    property string providerError: ""
    property color brandColor: descriptor && descriptor.brandColor ? descriptor.brandColor : AppTheme.accentColor

    signal dashboardRequested(string url)
    signal statusRequested(string url)
    signal refreshRequested()
    signal enabledToggled(bool enabled)

    readonly property string displayName: descriptor && descriptor.displayName ? descriptor.displayName : providerId
    readonly property bool providerEnabled: descriptor ? descriptor.enabled !== false : true
    readonly property string statusState: providerStatus && providerStatus.state ? providerStatus.state : "unknown"
    readonly property string dashboardUrl: descriptor && descriptor.dashboardURL ? descriptor.dashboardURL : ""
    readonly property string statusUrl: descriptor && descriptor.statusURL ? descriptor.statusURL : ""
    readonly property var sourceModes: descriptor && descriptor.sourceModes ? descriptor.sourceModes : []

    function statusText(state) {
        if (state === "ok") return qsTr("Operational")
        if (state === "degraded") return qsTr("Degraded")
        if (state === "outage") return qsTr("Outage")
        return qsTr("Unknown")
    }

    function statusColor(state) {
        if (state === "ok" || state === "succeeded") return AppTheme.statusOk
        if (state === "degraded" || state === "testing") return AppTheme.statusDegraded
        if (state === "outage" || state === "failed") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 14

        ProviderAvatar {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            Layout.alignment: Qt.AlignTop
            size: 48
            providerId: root.providerId
            displayName: root.displayName
            brandColor: root.brandColor
            selected: true
            enabled: root.providerEnabled
            severity: root.providerError !== "" ? "error" : "none"
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            spacing: 8

            GridLayout {
                Layout.fillWidth: true
                columns: root.width < 420 ? 1 : 2
                columnSpacing: 8
                rowSpacing: 6

                Label {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    text: root.displayName
                    color: AppTheme.textPrimary
                    font.pixelSize: 24
                    font.bold: true
                    elide: Text.ElideRight
                }

                StatusPill {
                    Layout.alignment: root.width < 420 ? Qt.AlignLeft : Qt.AlignRight
                    text: root.statusText(root.statusState)
                    toneColor: root.statusColor(root.statusState)
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: root.sourceModes.length > 0
                    ? qsTr("Sources: ") + root.sourceModes.join(", ")
                    : qsTr("Descriptor-driven provider settings")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                elide: Text.ElideRight
            }

            GridLayout {
                Layout.fillWidth: true
                columns: root.width < 520 ? 1 : 2
                columnSpacing: 12
                rowSpacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    ActionButton {
                        compact: true
                        text: qsTr("Dashboard")
                        visible: root.dashboardUrl !== ""
                        enabled: visible
                        onClicked: root.dashboardRequested(root.dashboardUrl)
                    }

                    ActionButton {
                        compact: true
                        text: qsTr("Status")
                        visible: root.statusUrl !== ""
                        enabled: visible
                        onClicked: root.statusRequested(root.statusUrl)
                    }

                    ActionButton {
                        compact: true
                        text: qsTr("Refresh")
                        onClicked: root.refreshRequested()
                    }

                    Item { Layout.fillWidth: true }
                }

                RowLayout {
                    Layout.alignment: root.width < 520 ? Qt.AlignLeft : Qt.AlignRight
                    spacing: 8

                    Label {
                        text: qsTr("Enabled")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                    }

                    SettingsSwitch {
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

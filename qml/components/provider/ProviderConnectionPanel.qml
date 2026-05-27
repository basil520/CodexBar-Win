import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    id: root

    property var connectionTest: ({"state": "idle"})
    property string connectionState: connectionTest && connectionTest.state ? connectionTest.state : "idle"
    property string connectionMessage: connectionTest && connectionTest.message ? connectionTest.message : ""
    property bool detailsExpanded: false

    signal testConnectionRequested()

    function statusColor(state) {
        if (state === "succeeded") return AppTheme.statusOk
        if (state === "testing") return AppTheme.statusDegraded
        if (state === "failed") return AppTheme.statusOutage
        return AppTheme.statusUnknown
    }

    function titleForState() {
        if (connectionState === "testing") return qsTr("Testing connection")
        if (connectionState === "succeeded") return qsTr("Connection OK")
        if (connectionState === "failed") return qsTr("Connection failed")
        return qsTr("Not tested")
    }

    function durationLabel(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return ""
        if (value < 1000) return Math.round(value) + " ms"
        return (value / 1000.0).toFixed(1) + " s"
    }

    function timeLabel(ms) {
        var value = Number(ms || 0)
        if (value <= 0) return ""
        return Qt.formatDateTime(new Date(value), "yyyy-MM-dd hh:mm:ss")
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: qsTr("Connection")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
            }

            Components.ConnectionStatus {
                statusState: root.connectionState
                message: root.connectionState === "idle" ? qsTr("Not tested") : root.connectionMessage
            }

            Components.ActionButton {
                text: root.connectionState === "testing" ? qsTr("Testing...") : qsTr("Test Connection")
                variant: "primary"
                compact: true
                enabled: root.connectionState !== "testing"
                onClicked: root.testConnectionRequested()
            }
        }

        Components.FeedbackBanner {
            Layout.fillWidth: true
            visible: root.connectionState !== "idle"
            status: root.connectionState === "failed" ? "error"
                : root.connectionState === "succeeded" ? "success"
                : "warning"
            title: root.titleForState()
            message: root.connectionMessage
                || (root.connectionState === "testing" ? qsTr("Running one non-interactive provider refresh.") : "")
            actionText: root.connectionState === "failed" ? qsTr("Retry") : ""
            onActionRequested: root.testConnectionRequested()
        }

        Label {
            Layout.fillWidth: true
            text: {
                var ts = root.timeLabel(root.connectionTest ? root.connectionTest.finishedAt : 0)
                var duration = root.durationLabel(root.connectionTest ? root.connectionTest.durationMs : 0)
                if (ts === "" && duration === "") return ""
                if (duration === "") return qsTr("Last tested: ") + ts
                if (ts === "") return duration
                return qsTr("Last tested: ") + ts + " · " + duration
            }
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeSm
            visible: text !== ""
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: root.connectionState === "failed"

            Components.ActionButton {
                text: root.detailsExpanded ? qsTr("Hide Details") : qsTr("Show Details")
                compact: true
                variant: "ghost"
                onClicked: root.detailsExpanded = !root.detailsExpanded
            }

            Components.ActionButton {
                text: qsTr("Copy")
                compact: true
                variant: "ghost"
                onClicked: AppController.copyText((root.connectionTest && root.connectionTest.details)
                    ? root.connectionTest.details
                    : root.connectionMessage)
            }

            Item { Layout.fillWidth: true }
        }

        Label {
            Layout.fillWidth: true
            text: root.connectionTest && root.connectionTest.details
                ? root.connectionTest.details
                : root.connectionMessage
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 8
            elide: Text.ElideRight
            visible: root.connectionState === "failed" && root.detailsExpanded
        }
    }
}

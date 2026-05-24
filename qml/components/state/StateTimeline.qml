import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    id: root

    property var events: []
    property int maxEvents: 4
    property string emptyTitle: qsTr("No recent activity")

    function severityColor(severity) {
        if (severity === "error" || severity === "blocked") return AppTheme.statusOutage
        if (severity === "warning" || severity === "stale") return AppTheme.statusDegraded
        if (severity === "success" || severity === "ready") return AppTheme.statusOk
        return AppTheme.accentColor
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: AppTheme.spacingSm

        Label {
            Layout.fillWidth: true
            text: qsTr("Activity Timeline")
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            elide: Text.ElideRight
        }

        Components.FeedbackBanner {
            Layout.fillWidth: true
            visible: !root.events || root.events.length === 0
            compact: true
            status: "info"
            title: root.emptyTitle
            message: qsTr("Recent UI events will appear here without storing secrets.")
        }

        Repeater {
            model: root.events ? Math.min(root.events.length, root.maxEvents) : 0

            RowLayout {
                Layout.fillWidth: true
                spacing: AppTheme.spacingSm

                Rectangle {
                    Layout.preferredWidth: 8
                    Layout.preferredHeight: 8
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 5
                    radius: 4
                    color: root.severityColor(root.events[index].severity || "info")
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Label {
                        Layout.fillWidth: true
                        text: root.events[index].title || ""
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fontSizeSm
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.events[index].message || ""
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        visible: text !== ""
                    }
                }

                Label {
                    text: root.events[index].createdAt || root.events[index].createdAtUtc || ""
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeXs
                    visible: text !== ""
                }
            }
        }
    }
}

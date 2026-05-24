import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Rectangle {
    id: root
    objectName: "providerIssuePanel"

    property string providerId: ""
    property string providerName: providerId
    property color brandColor: AppTheme.accentColor
    property var issue: ({})

    signal showDetailsRequested(string section)
    signal copyRequested(string text)
    signal retryRequested()
    signal importRequested()

    readonly property string severity: issue && issue.severity ? issue.severity : "error"
    readonly property color toneColor: severity === "warning" ? AppTheme.statusDegraded
        : severity === "info" ? AppTheme.accentColor
        : AppTheme.statusOutage
    readonly property string titleText: issue && issue.title ? issue.title : qsTr("Provider needs attention")
    readonly property string messageText: issue && issue.message ? issue.message : ""
    readonly property string detailText: issue && issue.detail ? issue.detail : ""
    readonly property string sectionTarget: issue && issue.section ? issue.section : "diagnostics"
    readonly property string primaryAction: issue && issue.primaryAction ? issue.primaryAction : "showDetails"
    readonly property string copyPayload: issue && issue.copyPayload ? issue.copyPayload : (detailText !== "" ? detailText : messageText)

    Layout.fillWidth: true
    implicitHeight: issueColumn.implicitHeight + 24
    radius: AppTheme.radiusLg
    color: AppTheme.withAlpha(root.toneColor, AppTheme.glassActive ? 0.16 : 0.12)
    border.width: 1
    border.color: AppTheme.withAlpha(root.toneColor, AppTheme.glassActive ? 0.54 : 0.42)
    visible: issue && issue.kind && issue.kind !== ""

    ColumnLayout {
        id: issueColumn
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 11

            Components.ProviderIdentityBadge {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 42
                Layout.alignment: Qt.AlignTop
                size: 42
                providerId: root.providerId
                displayName: root.providerName
                brandColor: root.brandColor
                selected: true
                severity: root.severity === "error" ? "error" : "warning"
                context: "hero"
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    text: root.titleText
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeLg
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    text: root.messageText
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    visible: text !== ""
                }
            }

            Label {
                text: issue && issue.meta ? issue.meta : ""
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeSm
                visible: text !== ""
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.detailText
            color: AppTheme.textTertiary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 3
            elide: Text.ElideRight
            visible: text !== "" && text !== root.messageText
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Components.ActionButton {
                compact: true
                variant: root.severity === "error" ? "danger" : "secondary"
                text: qsTr("Show Details")
                onClicked: root.showDetailsRequested(root.sectionTarget)
            }

            Components.ActionButton {
                compact: true
                variant: "ghost"
                text: qsTr("Copy")
                enabled: root.copyPayload !== ""
                onClicked: root.copyRequested(root.copyPayload)
            }

            Components.ActionButton {
                compact: true
                variant: "secondary"
                text: root.primaryAction === "import" ? qsTr("Import Again") : qsTr("Retry")
                onClicked: {
                    if (root.primaryAction === "import") root.importRequested()
                    else root.retryRequested()
                }
            }

            Item { Layout.fillWidth: true }
        }
    }
}

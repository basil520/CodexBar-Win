import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root

    property string title: qsTr("Error")
    property string message: ""
    property string detail: ""
    property string providerId: ""
    property string category: ""
    property string suggestedAction: ""
    property string density: "compact"
    property string severity: "error"
    property bool showCopy: true
    property bool expanded: false
    property string copyPayload: ProviderErrorClassifier.copyText(providerId, detail !== "" ? detail : message)
    readonly property bool compact: density === "compact"
    readonly property bool diagnostic: density === "diagnostic"
    readonly property color toneColor: severity === "warning" ? AppTheme.statusDegraded : AppTheme.statusOutage

    signal copyRequested(string text)

    Layout.preferredHeight: content.implicitHeight + (compact ? 14 : 18)
    implicitHeight: content.implicitHeight + (compact ? 14 : 18)
    radius: AppTheme.radiusSm
    color: AppTheme.withAlpha(toneColor, AppTheme.glassActive ? 0.14 : 0.10)
    border.width: 1
    border.color: AppTheme.withAlpha(toneColor, AppTheme.glassActive ? 0.46 : 0.34)
    visible: message !== "" || detail !== ""
    clip: true

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: compact ? 8 : 10
        anchors.rightMargin: compact ? 6 : 8
        spacing: compact ? 4 : 7

        RowLayout {
            Layout.fillWidth: true
            spacing: 7

            Rectangle {
                Layout.preferredWidth: 7
                Layout.preferredHeight: 7
                Layout.alignment: Qt.AlignVCenter
                radius: 4
                color: root.toneColor
            }

            Label {
                Layout.fillWidth: true
                text: root.compact && root.message !== "" ? root.message : root.title
                color: root.compact ? AppTheme.textPrimary : root.toneColor
                font.pixelSize: root.compact ? AppTheme.fontSizeSm : AppTheme.fontSizeMd
                font.bold: !root.compact
                maximumLineCount: root.compact ? 2 : 1
                wrapMode: Text.WrapAnywhere
                elide: Text.ElideRight
            }

            IconButton {
                visible: root.showCopy
                enabled: root.copyPayload !== ""
                width: 24
                height: 24
                implicitWidth: 24
                implicitHeight: 24
                symbol: "copy"
                accessibleName: qsTr("Copy error")
                tooltip: enabled ? qsTr("Copy error") : qsTr("Nothing to copy")
                iconColor: root.toneColor
                onActivated: root.copyRequested(root.copyPayload)
            }

            ActionButton {
                visible: root.detail !== "" && !root.compact
                text: root.expanded ? qsTr("Hide") : qsTr("Details")
                compact: true
                variant: "ghost"
                onClicked: root.expanded = !root.expanded
            }
        }

        Label {
            Layout.fillWidth: true
            visible: !root.compact && root.message !== ""
            text: root.message
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 3
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            visible: !root.compact && root.suggestedAction !== ""
            text: root.suggestedAction
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            visible: (root.expanded || root.diagnostic) && root.detail !== ""
            text: root.detail
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WrapAnywhere
            maximumLineCount: root.diagnostic ? 14 : 8
            elide: Text.ElideRight
        }
    }
}

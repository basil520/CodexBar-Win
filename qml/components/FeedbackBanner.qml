import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: root

    property string status: "info"
    property string title: ""
    property string message: ""
    property string actionText: ""
    property bool compact: false
    property color toneColor: status === "error" ? AppTheme.statusOutage
        : status === "warning" ? AppTheme.statusDegraded
        : status === "success" ? AppTheme.statusOk
        : AppTheme.accentColor

    signal actionRequested()

    visible: title !== "" || message !== ""
    radius: AppTheme.radiusMd
    color: status === "error" ? AppTheme.surfaceDangerSoft
        : status === "warning" ? AppTheme.surfaceWarningSoft
        : status === "success" ? AppTheme.surfaceSuccessSoft
        : AppTheme.surfaceCard
    border.width: 1
    border.color: AppTheme.withAlpha(toneColor, 0.42)
    implicitHeight: Math.max(compact ? 42 : 58, content.implicitHeight + 18)

    RowLayout {
        id: content
        anchors.fill: parent
        anchors.margins: compact ? 10 : 14
        spacing: AppTheme.spacingMd

        Rectangle {
            Layout.preferredWidth: 8
            Layout.preferredHeight: 8
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 5
            radius: 4
            color: root.toneColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: root.title
                visible: text !== ""
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                text: root.message
                visible: text !== ""
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
                maximumLineCount: root.compact ? 1 : 3
                elide: Text.ElideRight
            }
        }

        ActionButton {
            visible: root.actionText !== ""
            text: root.actionText
            compact: true
            variant: root.status === "error" ? "danger" : "secondary"
            onClicked: root.actionRequested()
        }
    }
}

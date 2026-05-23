import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

SurfaceCard {
    id: root

    property string title: ""
    property string subtitle: ""
    property bool expanded: false
    property bool canExpand: true
    default property alias content: expandedContent.data

    signal toggled()

    implicitHeight: header.implicitHeight + (expanded ? expandedContent.implicitHeight + AppTheme.spacingMd : 0) + 24
    interactive: canExpand
    selected: expanded
    activeFocusOnTab: canExpand

    Accessible.role: Accessible.Button
    Accessible.name: title
    Accessible.description: expanded ? qsTr("Expanded") : qsTr("Collapsed")

    function toggle() {
        if (canExpand) {
            toggled()
        }
    }

    Keys.onReturnPressed: toggle()
    Keys.onEnterPressed: toggle()
    Keys.onSpacePressed: toggle()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: AppTheme.spacingMd

        RowLayout {
            id: header
            Layout.fillWidth: true
            spacing: AppTheme.spacingMd

            ChevronIcon {
                Layout.preferredWidth: 14
                Layout.preferredHeight: 14
                expanded: root.expanded
                strokeColor: root.canExpand ? AppTheme.textSecondary : AppTheme.textDisabled
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: root.title
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                    elide: Text.ElideRight
                }

                Label {
                    Layout.fillWidth: true
                    visible: text !== ""
                    text: root.subtitle
                    color: AppTheme.textSecondary
                    font.pixelSize: AppTheme.fontSizeSm
                    elide: Text.ElideRight
                }
            }
        }

        ColumnLayout {
            id: expandedContent
            Layout.fillWidth: true
            visible: root.expanded
            spacing: AppTheme.spacingSm
        }
    }

    MouseArea {
        anchors.fill: header
        enabled: root.canExpand
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggle()
    }

    FocusRing {
        anchors.fill: parent
        active: root.activeFocus
    }
}

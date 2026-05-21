import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

ScrollView {
    id: root
    property string title: ""
    property string subtitle: ""
    property int maxContentWidth: 680
    default property alias content: body.data

    clip: true
    contentWidth: availableWidth

    ScrollBar.vertical: ScrollBar {
        id: elegantScrollBar
        policy: ScrollBar.AsNeeded
        active: hovered || pressed
            || (root.contentItem && (root.contentItem.moving === true || root.contentItem.flicking === true))

        background: Rectangle {
            color: "transparent"
        }

        contentItem: Rectangle {
            implicitWidth: 4
            radius: 2
            opacity: elegantScrollBar.active ? 1.0 : 0.0
            color: elegantScrollBar.hovered 
                ? AppTheme.textSecondary 
                : Qt.rgba(AppTheme.textSecondary.r, AppTheme.textSecondary.g, AppTheme.textSecondary.b, 0.35)

            Behavior on color { ColorAnimation { duration: 150 } }
            Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutQuad } }
            Behavior on implicitWidth { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        }

        states: State {
            name: "hoveredState"; when: elegantScrollBar.hovered
            PropertyChanges { target: elegantScrollBar.contentItem; implicitWidth: 8; radius: 4 }
        }
    }

    contentItem: Item {
        width: root.availableWidth
        height: body.implicitHeight + 48

        ColumnLayout {
            id: body
            x: 28
            y: 24
            width: Math.max(0, Math.min(root.availableWidth - 56, root.maxContentWidth))
            spacing: 12

            Label {
                text: root.title
                color: AppTheme.textPrimary
                font.pixelSize: 22
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: root.subtitle
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeMd
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                visible: text !== ""
                Layout.bottomMargin: 8
            }
        }
    }
}

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

    ScrollBar.vertical: ElegantScrollBar {
        flickable: root.contentItem
    }

    contentItem: Flickable {
        id: pageFlickable
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: Math.max(height, body.implicitHeight + 48)
        interactive: contentHeight > height

        ColumnLayout {
            id: body
            x: 28
            y: 24
            width: Math.max(0, Math.min(pageFlickable.width - 56, root.maxContentWidth))
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

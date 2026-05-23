import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import ".."

Rectangle {
    id: root

    property var storageItems: []
    property var cleanupItems: []
    property color barColor: AppTheme.accentColor

    readonly property bool hasData: storageItems.length > 0

    color: AppTheme.surfaceChart
    radius: 8
    implicitWidth: 276
    implicitHeight: contentColumn.implicitHeight + 16
    clip: true

    function formatBytes(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(1) + " MB"
        if (bytes >= 1024) return (bytes / 1024).toFixed(1) + " KB"
        return bytes + " B"
    }

    function elidePath(path, maxLen) {
        if (path.length <= maxLen) return path
        var half = Math.floor(maxLen / 2) - 1
        return path.substring(0, half) + "..." + path.substring(path.length - half)
    }

    function visibleStorageItems() {
        var items = []
        for (var i = 0; i < Math.min(8, root.storageItems.length); i++) {
            var item = root.storageItems[i]
            if (item.isMoreIndicator) continue
            items.push(item)
        }
        return items
    }

    function moreCount() {
        var count = 0
        for (var i = 0; i < root.storageItems.length; i++) {
            if (root.storageItems[i].isMoreIndicator) {
                count = root.storageItems[i].remainingCount
                break
            }
        }
        if (root.storageItems.length > 8 && count === 0) {
            count = root.storageItems.length - 8
        }
        return count
    }

    function totalBytes() {
        var total = 0
        for (var i = 0; i < root.storageItems.length; i++) {
            if (!root.storageItems[i].isMoreIndicator) {
                total += root.storageItems[i].bytes || 0
            }
        }
        return total
    }

    ColumnLayout {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        spacing: 8

        // Title
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: qsTr("Storage")
                color: AppTheme.textSecondary
                font.pixelSize: 11
                font.bold: true
            }

            Text {
                visible: root.hasData
                text: qsTr("Total: %1").arg(root.formatBytes(root.totalBytes()))
                color: AppTheme.textInverse
                font.pixelSize: 10
            }
        }

        // No data state
        FeedbackBanner {
            Layout.fillWidth: true
            visible: !root.hasData
            status: "info"
            title: qsTr("No storage data available")
            message: qsTr("Storage details will appear after the provider reports local cache usage.")
            compact: true
        }

        // Storage components
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.hasData
            spacing: 6

            Repeater {
                model: root.visibleStorageItems()

                delegate: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    property var itemData: modelData

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: root.elidePath(itemData.path, 40)
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            Layout.fillWidth: true
                            elide: Text.ElideMiddle

                            ToolTip.visible: pathMouse.containsMouse
                            ToolTip.text: itemData.path
                            ToolTip.delay: 500

                            MouseArea {
                                id: pathMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.NoButton
                            }
                        }

                        CopyIconButton {
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                            copyText: itemData.path
                        }

                        Text {
                            text: itemData.bytesDisplay || root.formatBytes(itemData.bytes)
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                            Layout.preferredWidth: 60
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    // Capsule progress bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 6
                        radius: 3
                        color: AppTheme.surfaceTrack

                        Rectangle {
                            width: Math.max(2, parent.width * Math.min(1, itemData.fraction || 0))
                            height: parent.height
                            radius: 3
                            color: root.barColor
                        }
                    }
                }
            }

            // More items indicator
            Text {
                Layout.fillWidth: true
                visible: root.moreCount() > 0
                text: qsTr("%1 more items").arg(root.moreCount())
                color: AppTheme.textDisabled
                font.pixelSize: 10
            }
        }

        // Cleanup suggestions
        DisclosureRow {
            Layout.fillWidth: true
            visible: root.cleanupItems.length > 0
            title: qsTr("Cleanup ideas")
            subtitle: qsTr("%1 suggestions").arg(root.cleanupItems.length)
            expanded: cleanupExpanded
            property bool cleanupExpanded: true
            onToggled: cleanupExpanded = !cleanupExpanded

            Repeater {
                model: root.cleanupItems

                delegate: ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    property var cleanupData: modelData

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: cleanupData.title
                            color: AppTheme.textTertiary
                            font.pixelSize: 10
                            font.bold: true
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Text {
                            text: cleanupData.bytesDisplay || root.formatBytes(cleanupData.bytes)
                            color: AppTheme.textInverse
                            font.pixelSize: 10
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: root.elidePath(cleanupData.path, 35)
                            color: AppTheme.textInverse
                            font.pixelSize: 9
                            Layout.fillWidth: true
                            elide: Text.ElideMiddle
                        }

                        CopyIconButton {
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            copyText: cleanupData.path
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: cleanupData.consequence
                        color: AppTheme.textDisabled
                        font.pixelSize: 9
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}

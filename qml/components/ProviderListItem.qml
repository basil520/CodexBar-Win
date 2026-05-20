import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import ".."

Rectangle {
    id: root
    property string providerName: ""
    property string providerId: ""
    property color brandColor: AppTheme.accentColor
    property bool isEnabled: false
    property bool isSelected: false
    property var usageData: null
    property string status: "unknown"
    property string lastUpdated: ""
    property int itemIndex: 0

    signal clicked()
    signal toggleChanged(bool checked)
    signal dragStarted(int index)
    signal dragFinished(int fromIndex, int toIndex)

    height: AppTheme.listItemHeight
    radius: AppTheme.radiusMd
    color: {
        if (root.isSelected) return AppTheme.surfaceSelected
        if (rowMouse.containsMouse) return AppTheme.surfaceInteractiveHover
        return "transparent"
    }

    Behavior on color {
        ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
    }

    Drag.active: dragArea.drag.active
    Drag.source: root
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    MouseArea {
        id: rowMouse
        anchors.fill: parent
        z: 0
        hoverEnabled: true
        onClicked: root.clicked()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: 3
        height: 24
        radius: 2
        color: root.brandColor
        visible: root.isSelected
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 8
        spacing: 8
        z: 1

        Item {
            id: dragHandle
            Layout.preferredWidth: 10
            Layout.preferredHeight: 22
            Layout.alignment: Qt.AlignVCenter
            opacity: rowMouse.containsMouse || root.isSelected || dragArea.drag.active ? 1.0 : 0.0

            Column {
                anchors.centerIn: parent
                spacing: 2

                Repeater {
                    model: 3
                    Row {
                        spacing: 2
                        Repeater {
                            model: 2
                            Rectangle {
                                width: 2
                                height: 2
                                radius: 1
                                color: AppTheme.textTertiary
                            }
                        }
                    }
                }
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                cursorShape: Qt.OpenHandCursor
                drag.target: root
                drag.axis: Drag.YAxis
                onPressed: {
                    cursorShape = Qt.ClosedHandCursor
                    root.dragStarted(root.itemIndex)
                }
                onReleased: {
                    cursorShape = Qt.OpenHandCursor
                    root.dragFinished(root.itemIndex, -1)
                    root.y = 0
                }
            }
        }

        ProviderAvatar {
            Layout.preferredWidth: AppTheme.avatarSizeList
            Layout.preferredHeight: AppTheme.avatarSizeList
            Layout.alignment: Qt.AlignVCenter
            size: AppTheme.avatarSizeList
            providerId: root.providerId
            displayName: root.providerName
            brandColor: root.brandColor
            selected: root.isSelected
            enabled: root.isEnabled
            severity: root.status === "outage" ? "error" : (root.status === "degraded" ? "warning" : "none")
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            Label {
                Layout.fillWidth: true
                text: root.providerName
                color: root.isEnabled ? AppTheme.textPrimary : AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: root.isSelected
                elide: Text.ElideRight
            }

            UsageProgressBar {
                Layout.fillWidth: true
                Layout.minimumWidth: 44
                Layout.maximumWidth: 92
                visible: root.usageData !== null
                    && root.usageData !== undefined
                    && root.usageData.percent !== undefined
                value: root.usageData !== null
                    && root.usageData !== undefined
                    && root.usageData.percent !== undefined
                    ? root.usageData.percent
                    : 0
                tintColor: root.brandColor
            }
        }

        Rectangle {
            Layout.preferredWidth: 8
            Layout.preferredHeight: 8
            Layout.alignment: Qt.AlignVCenter
            radius: width / 2
            color: {
                switch (root.status) {
                case "ok": return AppTheme.statusOk
                case "degraded": return AppTheme.statusDegraded
                case "outage": return AppTheme.statusOutage
                default: return AppTheme.statusUnknown
                }
            }
            opacity: root.isEnabled ? 1.0 : 0.45
        }

        SettingsSwitch {
            Layout.alignment: Qt.AlignVCenter
            checked: root.isEnabled
            onToggled: function(checked) {
                root.toggleChanged(checked)
            }
        }
    }

    DropArea {
        anchors.fill: parent
        onEntered: {
            if (drag.source !== root && drag.source && drag.source.itemIndex !== undefined) {
                root.dragFinished(drag.source.itemIndex, root.itemIndex)
            }
        }
    }
}

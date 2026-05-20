import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

Rectangle {
    id: root

    property var providerList: []
    property string selectedProviderID: ""
    property bool isSwitching: false
    property int avatarSize: AppTheme.avatarSizeDock
    property bool showUsageRings: true

    signal selectProvider(string providerId)

    width: parent ? parent.width - 24 : 276
    implicitHeight: 56
    height: implicitHeight
    radius: AppTheme.radiusLg
    color: AppTheme.surfacePane
    border.width: 1
    border.color: AppTheme.borderSubtle
    visible: providerList.length > 0
    clip: true

    function brandColorFor(providerId) {
        return AppTheme.providerBrandColor(providerId)
    }

    function usageFor(modelData) {
        if (!modelData || modelData.hasUsage !== true) return 0
        if (modelData.weeklyRemaining !== undefined && modelData.weeklyRemaining !== null) {
            return Math.max(0, Math.min(100, modelData.weeklyRemaining))
        }
        if (modelData.sessionRemaining !== undefined && modelData.sessionRemaining !== null) {
            return Math.max(0, Math.min(100, modelData.sessionRemaining))
        }
        return 0
    }

    Flickable {
        id: flicker
        anchors.fill: parent
        anchors.margins: 6
        contentWidth: buttonRow.width
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (wheel.angleDelta.y !== 0 || wheel.angleDelta.x !== 0) {
                    var delta = wheel.angleDelta.y !== 0 ? wheel.angleDelta.y : -wheel.angleDelta.x
                    var maxX = Math.max(0, flicker.contentWidth - flicker.width)
                    flicker.contentX = Math.max(0, Math.min(maxX, flicker.contentX - delta))
                    wheel.accepted = true
                }
            }
        }

        Row {
            id: buttonRow
            height: parent.height
            spacing: AppTheme.spacingSm
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: AppTheme.spacingXs
            rightPadding: AppTheme.spacingXs

            Repeater {
                model: root.providerList

                delegate: Item {
                    id: btn
                    width: 44
                    height: 44
                    anchors.verticalCenter: parent.verticalCenter

                    property bool isSelected: modelData.providerId === root.selectedProviderID
                    property color itemBrandColor: root.brandColorFor(modelData.providerId)

                    scale: itemMouse.containsMouse ? (itemMouse.pressed ? 0.96 : 1.04) : 1.0
                    opacity: modelData.enabled === false ? 0.58 : 1.0

                    Behavior on scale {
                        NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeEmphasized }
                    }
                    Behavior on opacity {
                        NumberAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width
                        height: parent.height
                        radius: width / 2
                        color: btn.isSelected ? AppTheme.withAlpha(btn.itemBrandColor, 0.18)
                            : (itemMouse.containsMouse ? AppTheme.surfaceInteractiveHover : "transparent")
                        border.width: btn.isSelected ? 1 : 0
                        border.color: AppTheme.withAlpha(btn.itemBrandColor, 0.62)

                        Behavior on color {
                            ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
                        }
                    }

                    ProviderAvatar {
                        anchors.centerIn: parent
                        size: root.avatarSize
                        providerId: modelData.providerId || ""
                        displayName: modelData.displayName || modelData.providerId || ""
                        iconSource: modelData.iconSource || ""
                        brandColor: btn.itemBrandColor
                        selected: btn.isSelected
                        enabled: modelData.enabled !== false
                        severity: modelData.error ? "error" : "none"
                        showProgressRing: root.showUsageRings && modelData.hasUsage === true
                        usagePercent: root.usageFor(modelData)
                        density: "compact"
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: root.isSwitching ? Qt.BusyCursor : Qt.PointingHandCursor
                        enabled: !root.isSwitching
                        onClicked: {
                            if (modelData.providerId !== root.selectedProviderID) {
                                root.selectProvider(modelData.providerId)
                            }
                        }
                    }

                    ToolTip.visible: itemMouse.containsMouse
                    ToolTip.delay: 450
                    ToolTip.text: {
                        var label = modelData.displayName || modelData.providerId || ""
                        if (modelData.error) return label + qsTr(" - needs attention")
                        if (modelData.enabled === false) return label + qsTr(" - disabled")
                        return label
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 14
        visible: flicker.contentWidth > flicker.width && flicker.contentX > 0
        gradient: Gradient {
            GradientStop { position: 0.0; color: AppTheme.surfacePane }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 14
        visible: flicker.contentWidth > flicker.width && flicker.contentX < flicker.contentWidth - flicker.width
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: AppTheme.surfacePane }
        }
    }
}

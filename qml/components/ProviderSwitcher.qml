import QtQuick 2.15
import ".."

Rectangle {
    id: root
    property var providerList: []
    property string selectedProviderID: ""
    property bool isSwitching: false

    signal selectProvider(string providerId)

    width: parent ? parent.width - 24 : 276
    radius: AppTheme.radiusMd
    color: AppTheme.surfacePane
    visible: providerList.length > 0
    implicitHeight: 44
    height: implicitHeight

    function brandColorFor(providerId) {
        return AppTheme.providerBrandColor(providerId)
    }

    Flickable {
        id: flicker
        anchors.fill: parent
        anchors.margins: 5
        contentWidth: buttonRow.width
        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (wheel.angleDelta.y !== 0) {
                    var newX = flicker.contentX - wheel.angleDelta.y;
                    newX = Math.max(0, Math.min(newX, flicker.contentWidth - flicker.width));
                    flicker.contentX = newX;
                    wheel.accepted = true;
                }
            }
        }

        Row {
            id: buttonRow
            spacing: 1
            height: parent.height

            Repeater {
                model: root.providerList
                delegate: Rectangle {
                    id: btn
                    width: 72
                    height: 34
                    radius: 6

                    property bool isSelected: modelData.providerId === root.selectedProviderID

                    color: {
                        if (isSelected) return AppTheme.accentColor;
                        if (btnHover.hovered) return AppTheme.surfaceHover;
                        return "transparent";
                    }

                    Behavior on color {
                        ColorAnimation { duration: 120; easing.type: Easing.OutQuad }
                    }

                    Column {
                        id: contentRow
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: -2
                        spacing: 2

                        ProviderAvatar {
                            anchors.horizontalCenter: parent.horizontalCenter
                            size: 18
                            providerId: modelData.providerId || ""
                            displayName: modelData.displayName || modelData.providerId || ""
                            iconSource: modelData.iconSource || ""
                            brandColor: root.brandColorFor(modelData.providerId)
                            selected: btn.isSelected
                            enabled: modelData.enabled !== false
                            severity: modelData.error ? "error" : "none"
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.displayName || modelData.providerId
                            font.pixelSize: 10
                            color: btn.isSelected ? AppTheme.textPrimary : AppTheme.textSecondary
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        height: 3
                        radius: 1.5
                        color: AppTheme.surfaceBorder
                        visible: !btn.isSelected && modelData.hasUsage === true && modelData.weeklyRemaining !== undefined && modelData.weeklyRemaining !== null

                        Rectangle {
                            width: parent.width * Math.max(0, Math.min(1, (modelData.weeklyRemaining || 0) / 100))
                            height: parent.height
                            radius: 1.5
                            color: root.brandColorFor(modelData.providerId)
                            Behavior on width {
                                NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                            }
                        }
                    }

                    TapHandler {
                        enabled: !root.isSwitching
                        onTapped: {
                            if (modelData.providerId !== root.selectedProviderID) {
                                root.selectProvider(modelData.providerId);
                            }
                        }
                    }

                    HoverHandler {
                        id: btnHover
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
        }
    }

    // Left scroll hint
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 10
        radius: parent.radius
        visible: flicker.contentWidth > flicker.width && flicker.contentX > 0
        gradient: Gradient {
            GradientStop { position: 0.0; color: AppTheme.surfacePane }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    // Right scroll hint
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 10
        radius: parent.radius
        visible: flicker.contentWidth > flicker.width && flicker.contentX < flicker.contentWidth - flicker.width
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: AppTheme.surfacePane }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: AppTheme.surfaceControl
        opacity: root.isSwitching ? 0.3 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }
    }
}

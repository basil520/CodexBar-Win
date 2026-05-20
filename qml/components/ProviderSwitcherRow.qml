import QtQuick 2.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    property var providerList: []
    property string selectedProviderID: ""
    property bool isSwitching: false

    signal selectProvider(string providerId)

    width: parent ? parent.width - 24 : 276
    radius: 10
    color: AppTheme.surfacePane || "#1c1c28"
    border.width: 1
    border.color: AppTheme.surfaceBorder || "#2d2d3d"
    visible: providerList.length > 0
    implicitHeight: 58
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
            spacing: 10
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 6
            rightPadding: 6

            Repeater {
                model: root.providerList
                delegate: Item {
                    id: btn
                    width: 46
                    height: 46
                    anchors.verticalCenter: parent.verticalCenter

                    property bool isSelected: modelData.providerId === root.selectedProviderID

                    scale: mouseArea.containsMouse ? (mouseArea.pressed ? 0.96 : 1.02) : 1.0
                    Behavior on scale {
                        NumberAnimation { duration: 180; easing.type: Easing.OutBack }
                    }

                    // Thin dynamic progress circle around provider icon
                    Canvas {
                        id: progressRing
                        anchors.fill: parent
                        property double usageVal: {
                            var v = 100.0;
                            if (modelData.weeklyRemaining !== undefined && modelData.weeklyRemaining !== null) {
                                v = modelData.weeklyRemaining;
                            } else if (modelData.primaryRemaining !== undefined && modelData.primaryRemaining !== null) {
                                v = modelData.primaryRemaining;
                            }
                            return Math.max(0, Math.min(100, v)) / 100.0;
                        }
                        
                        property color ringColor: root.brandColorFor(modelData.providerId)
                        
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            
                            var cx = width / 2;
                            var cy = height / 2;
                            var radius = 20;
                            
                            // Background track
                            ctx.beginPath();
                            ctx.strokeStyle = btn.isSelected ? Qt.rgba(ringColor.r, ringColor.g, ringColor.b, 0.2) : "#2c2c3d";
                            ctx.lineWidth = 1.5;
                            ctx.arc(cx, cy, radius, 0, 2 * Math.PI);
                            ctx.stroke();
                            
                            // Progress arc
                            if (usageVal > 0) {
                                ctx.beginPath();
                                ctx.strokeStyle = ringColor;
                                ctx.lineWidth = btn.isSelected ? 2.5 : 1.5;
                                ctx.arc(cx, cy, radius, -0.5 * Math.PI, (usageVal * 2 * Math.PI) - 0.5 * Math.PI);
                                ctx.stroke();
                            }
                        }

                        onUsageValChanged: requestPaint()
                        onRingColorChanged: requestPaint()
                        Connections {
                            target: btn
                            function onIsSelectedChanged() { progressRing.requestPaint(); }
                        }
                    }

                    // Centered provider identity. ProviderAvatar handles dark logos and fallback text.
                    ProviderAvatar {
                        anchors.centerIn: parent
                        size: 32
                        providerId: modelData.providerId || ""
                        displayName: modelData.displayName || modelData.providerId || ""
                        iconSource: modelData.iconSource || ""
                        brandColor: root.brandColorFor(modelData.providerId)
                        selected: btn.isSelected
                        enabled: modelData.enabled !== false
                        severity: modelData.error ? "error" : "none"
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (!root.isSwitching && modelData.providerId !== root.selectedProviderID) {
                                root.selectProvider(modelData.providerId);
                            }
                        }
                    }

                    ToolTip {
                        visible: mouseArea.containsMouse
                        delay: 500
                        text: modelData.displayName || modelData.providerId
                    }
                }
            }
        }
    }

    // Scroll hints
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
}

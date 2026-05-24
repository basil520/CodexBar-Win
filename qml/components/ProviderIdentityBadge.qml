import QtQuick 2.15
import ".."

Item {
    id: root

    property string providerId: ""
    property string displayName: ""
    property string iconSource: ""
    property int size: 24
    property bool selected: false
    property string severity: "none"
    property bool showProgressRing: false
    property real usagePercent: 100
    property bool showAccountBadge: false
    property bool showSessionBadge: false
    property string badgeText: ""
    property string context: size <= 20 ? "compact" : (size >= 44 ? "hero" : "normal")
    property color brandColor: identity && identity.brandColor
        ? identity.brandColor
        : AppTheme.providerBrandColor(providerId)

    readonly property var identity: registry.identityFor(providerId)
    readonly property string accessibleLabel: displayName !== "" ? displayName : providerId

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size

    Accessible.name: accessibleLabel
    Accessible.description: identity.iconMode + " " + identity.vesselShape

    ProviderIdentityRegistry {
        id: registry
    }

    ProviderAvatar {
        anchors.fill: parent
        providerId: root.providerId
        displayName: root.displayName
        iconSource: root.iconSource
        brandColor: root.brandColor
        size: root.size
        selected: root.selected
        enabled: root.enabled
        severity: root.severity
        showProgressRing: root.showProgressRing
        usagePercent: root.usagePercent
        density: root.context
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: root.badgeText !== "" ? Math.max(12, badgeLabel.implicitWidth + 6) : Math.max(8, Math.round(root.size * 0.26))
        height: Math.max(8, Math.round(root.size * 0.26))
        radius: height / 2
        color: root.showAccountBadge ? AppTheme.surfacePopup : AppTheme.accentColor
        border.width: 1
        border.color: root.brandColor
        visible: root.showAccountBadge || root.showSessionBadge || root.badgeText !== ""

        Text {
            id: badgeLabel
            anchors.centerIn: parent
            text: root.badgeText
            visible: text !== ""
            color: AppTheme.textPrimary
            font.pixelSize: Math.max(7, Math.round(root.size * 0.18))
            font.bold: true
        }
    }
}

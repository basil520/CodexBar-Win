import QtQuick 2.15
import ".."

Item {
    id: root

    property string providerId: ""
    property string displayName: ""
    property string iconSource: ""
    property color brandColor: AppTheme.providerBrandColor(providerId)
    property int size: 24
    property bool selected: false
    property string severity: "none"
    property bool showProgressRing: false
    property real usagePercent: 100
    property string density: size <= 20 ? "compact" : (size >= 44 ? "hero" : "normal")

    readonly property var iconPolicy: policy.iconPolicy(providerId)

    implicitWidth: size
    implicitHeight: size
    width: size
    height: size

    ProviderIconPolicy {
        id: policy
    }

    ProviderIconVessel {
        anchors.fill: parent
        providerId: root.providerId
        displayName: root.displayName
        iconSource: root.iconSource
        brandColor: root.brandColor
        size: root.size
        selected: root.selected
        severity: root.severity
        showProgressRing: root.showProgressRing
        usagePercent: root.usagePercent
        density: root.density
        policy: root.iconPolicy
        enabled: root.enabled
    }
}

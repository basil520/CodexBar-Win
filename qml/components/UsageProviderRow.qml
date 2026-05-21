import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Item {
    id: root

    property var provider: ({})
    property string providerId: ""
    property string providerName: ""
    property color accentColor: AppTheme.providerBrandColor(effectiveProviderId)
    property string summary: ""
    property string subtitleText: ""
    property string kindText: ""
    property string status: "ok"
    property bool expanded: false
    property bool canExpand: false
    property bool providerEnabled: true
    property string todayText: ""
    property string periodText: ""
    property string tokenText: ""
    property var trendValues: []

    readonly property string effectiveProviderId: {
        var row = provider || {}
        return row.providerId || providerId
    }
    readonly property string effectiveProviderName: {
        var row = provider || {}
        return providerName || row.displayName || row.providerId || providerId
    }
    readonly property bool effectiveProviderEnabled: {
        var row = provider || {}
        return providerEnabled && row.enabled !== false
    }
    readonly property string effectiveSummary: summary || tokenText || periodText
    readonly property string effectiveSubtitle: subtitleText || tokenText

    signal toggleRequested()

    implicitHeight: 34
    opacity: effectiveProviderEnabled ? 1 : 0.62
    activeFocusOnTab: canExpand
    clip: true

    Accessible.role: Accessible.Button
    Accessible.name: effectiveProviderName
        + (kindText !== "" ? ", " + kindText : "")
        + (effectiveSummary !== "" ? ", " + effectiveSummary : "")
    Accessible.description: canExpand ? (expanded ? "Expanded" : "Collapsed") : effectiveSubtitle

    function requestToggle() {
        if (canExpand) {
            toggleRequested()
        }
    }

    Keys.onReturnPressed: requestToggle()
    Keys.onEnterPressed: requestToggle()
    Keys.onSpacePressed: requestToggle()

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            Layout.preferredWidth: 14
            Layout.fillHeight: true
            text: root.canExpand ? (root.expanded ? "v" : ">") : ""
            color: root.canExpand ? AppTheme.textSecondary : root.accentColor
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        ProviderAvatar {
            Layout.preferredWidth: AppTheme.avatarSizeList
            Layout.preferredHeight: AppTheme.avatarSizeList
            Layout.alignment: Qt.AlignVCenter
            size: AppTheme.avatarSizeList
            providerId: root.effectiveProviderId
            displayName: root.effectiveProviderName
            brandColor: root.accentColor
            enabled: root.effectiveProviderEnabled
            selected: root.expanded
            severity: root.status
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 1

            Label {
                Layout.fillWidth: true
                text: root.effectiveProviderName
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                visible: text !== ""
                text: root.effectiveSubtitle
                color: AppTheme.textTertiary
                font.pixelSize: AppTheme.fontSizeXs
                elide: Text.ElideRight
            }
        }

        StatusPill {
            visible: text !== ""
            text: root.kindText
            toneColor: root.accentColor
        }

        Label {
            Layout.preferredWidth: Math.min(230, Math.max(150, implicitWidth))
            text: root.effectiveSummary
            color: root.status === "warning" ? AppTheme.textPrimary : AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            font.bold: root.status === "warning"
            horizontalAlignment: Text.AlignRight
            elide: Text.ElideRight
            visible: text !== ""
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.canExpand
        hoverEnabled: true
        cursorShape: root.canExpand ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.requestToggle()
    }

    FocusRing {
        anchors.fill: parent
        active: root.activeFocus
    }
}

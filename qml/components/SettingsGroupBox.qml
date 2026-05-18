import QtQuick 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    color: SettingsStore.glassEffectEnabled
        ? Qt.rgba(AppTheme.bgCard.r, AppTheme.bgCard.g, AppTheme.bgCard.b,
                  Math.min(0.72, Math.max(0.18, SettingsStore.glassEffectOpacity / 100 + 0.04)))
        : AppTheme.bgCard
    radius: AppTheme.radiusMd
    border.width: 1
    border.color: AppTheme.borderColor
    Layout.fillWidth: true
    implicitHeight: groupLayout.implicitHeight + 28
    default property alias content: groupLayout.data

    ColumnLayout {
        id: groupLayout
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10
    }
}

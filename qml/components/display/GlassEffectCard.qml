import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    Components.SettingsToggleRow {
        title: qsTr("Glass Effect")
        subtitle: qsTr("Use native system glass behind app windows when available.")
        checked: SettingsStore.glassEffectEnabled
        onToggled: function(checked) {
            SettingsStore.glassEffectEnabled = checked
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 56
        enabled: SettingsStore.glassEffectEnabled
        opacity: enabled ? 1.0 : 0.45
        spacing: AppTheme.spacingMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingXs

            Label {
                text: qsTr("Glass Opacity")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Lower values make the glass more transparent.")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
            }
        }

        Slider {
            Layout.preferredWidth: 220
            from: 5
            to: 95
            stepSize: 5
            snapMode: Slider.SnapAlways
            live: true
            value: SettingsStore.glassEffectOpacity
            onMoved: SettingsStore.glassEffectOpacity = Math.round(value)
        }

        Label {
            Layout.preferredWidth: 42
            text: SettingsStore.glassEffectOpacity + "%"
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignRight
        }
    }

    Components.SettingsToggleRow {
        title: qsTr("Reduce Motion")
        subtitle: qsTr("Minimize decorative animation while keeping essential feedback.")
        checked: SettingsStore.reduceMotion
        onToggled: function(checked) {
            SettingsStore.reduceMotion = checked
        }
    }

    Components.SettingsComboRow {
        title: qsTr("Visual Effects Quality")
        subtitle: qsTr("Lower quality reduces ambient animation and layered effects.")
        comboBoxWidth: 190
        model: [
            { value: "high", label: qsTr("High") },
            { value: "balanced", label: qsTr("Balanced") },
            { value: "low", label: qsTr("Low") }
        ]
        selectedValue: SettingsStore.visualEffectsQuality
        onValueActivated: function(value) {
            SettingsStore.visualEffectsQuality = value
        }
    }
}

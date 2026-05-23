import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CodexBarX 1.0
import "../.."
import ".." as Components

Components.SettingsGroupBox {
    Components.SettingsToggleRow {
        title: qsTr("Merge Icons")
        subtitle: qsTr("Show a single combined tray icon for enabled providers.")
        checked: SettingsStore.mergeIcons
        onToggled: function(checked) {
            SettingsStore.mergeIcons = checked
        }
    }

    Components.SettingsToggleRow {
        title: qsTr("Show Usage Amount Used")
        subtitle: qsTr("Use consumed percentage instead of remaining percentage.")
        checked: SettingsStore.usageBarsShowUsed
        onToggled: function(checked) {
            SettingsStore.usageBarsShowUsed = checked
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: AppTheme.spacingMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: qsTr("Tray Display Mode")
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Change how usage info is presented in the system taskbar.")
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
            }
        }

        Components.SettingsComboBox {
            Layout.preferredWidth: 190
            model: [
                { value: 0, label: qsTr("Icon Only") },
                { value: 1, label: qsTr("Percentage") },
                { value: 2, label: qsTr("Remaining Time") },
                { value: 3, label: qsTr("Custom Time") }
            ]
            selectedValue: SettingsStore.trayDisplayMode
            onValueActivated: function(value) {
                SettingsStore.trayDisplayMode = value
            }
        }
    }

    ThresholdSlider {
        title: qsTr("Warning Threshold")
        subtitle: qsTr("Show warnings when usage remaining drops below this level.")
        value: SettingsStore.warningThreshold
        fromValue: 5
        toValue: 95
        onMovedTo: function(value) {
            SettingsStore.warningThreshold = value
        }
    }

    ThresholdSlider {
        title: qsTr("Critical Threshold")
        subtitle: qsTr("Show alert warning when usage remaining drops below this level.")
        value: SettingsStore.criticalThreshold
        fromValue: 1
        toValue: 50
        onMovedTo: function(value) {
            SettingsStore.criticalThreshold = value
        }
    }

    component ThresholdSlider: RowLayout {
        property string title: ""
        property string subtitle: ""
        property int value: 0
        property int fromValue: 0
        property int toValue: 100
        signal movedTo(int value)

        Layout.fillWidth: true
        Layout.preferredHeight: 56
        spacing: AppTheme.spacingMd

        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppTheme.spacingXs

            Label {
                text: title
                color: AppTheme.textPrimary
                font.pixelSize: AppTheme.fontSizeMd
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                text: subtitle
                color: AppTheme.textSecondary
                font.pixelSize: AppTheme.fontSizeSm
                wrapMode: Text.WordWrap
            }
        }

        Slider {
            Layout.preferredWidth: 220
            from: fromValue
            to: toValue
            stepSize: 1
            snapMode: Slider.SnapAlways
            live: true
            value: parent.value
            onMoved: parent.movedTo(Math.round(value))
        }

        Label {
            Layout.preferredWidth: 42
            text: parent.value + "%"
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            horizontalAlignment: Text.AlignRight
        }
    }
}

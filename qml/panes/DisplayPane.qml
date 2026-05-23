import QtQuick 2.15
import CodexBarX 1.0
import ".."
import "../components"
import "../components/display" as DisplayComponents

SettingsPage {
    title: qsTr("Display")
    subtitle: qsTr("Tune how usage, glass, motion, and tray state are presented.")

    SettingsSectionHeader {
        text: qsTr("Appearance")
    }

    DisplayComponents.ThemeSelectorCard {}

    DisplayComponents.GlassEffectCard {}

    SettingsSectionHeader {
        text: qsTr("Tray Presentation")
    }

    DisplayComponents.TrayDisplayCard {}

    SettingsSectionHeader {
        text: qsTr("Usage Display")
    }

    DisplayComponents.UsageDisplayCard {}

    SettingsSectionHeader {
        text: qsTr("Live Preview")
    }

    DisplayComponents.DisplayPreviewCard {}
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import "components" as Components

ApplicationWindow {
    id: root
    visible: false
    flags: Qt.FramelessWindowHint

    TrayPanel {
        id: trayPanel
    }

    Shortcut {
        sequence: "Ctrl+K"
        onActivated: commandPalette.openPalette()
    }

    Components.CommandPalette {
        id: commandPalette
        anchors.centerIn: parent
        width: 460
        commands: [
            { id: "open-settings", title: qsTr("Open Settings"), subtitle: qsTr("Configure providers and display"), keywords: "settings provider display" },
            { id: "open-usage", title: qsTr("Open Usage Observatory"), subtitle: qsTr("Inspect cost and token trends"), keywords: "usage cost token observatory" },
            { id: "refresh", title: qsTr("Refresh Providers"), subtitle: qsTr("Refresh cached provider snapshots"), keywords: "refresh providers" }
        ]
        onCommandTriggered: function(commandId) {
            if (commandId === "open-settings") AppController.toggleSettings()
            else if (commandId === "open-usage") AppController.openUsage()
            else if (commandId === "refresh") TrayViewModel.refresh()
        }
    }
}

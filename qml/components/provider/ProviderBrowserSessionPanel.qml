import QtQuick 2.15
import CodexBarX 1.0
import ".." as Components

Loader {
    id: root

    property string providerId: ""

    active: SettingsStore.browserSessionBridgeEnabled
        && BridgeViewModel.isProviderSupported(providerId)
    visible: active

    sourceComponent: Components.BrowserSessionCard {
        providerId: root.providerId
    }
}

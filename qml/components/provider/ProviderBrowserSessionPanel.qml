import QtQuick 2.15
import CodexBarX 1.0
import ".." as Components

Loader {
    id: root

    property string providerId: ""
    property string diagnosticText: ""

    active: SettingsStore.browserSessionBridgeEnabled
        && (BridgeViewModel.shouldShowProviderPanel(providerId)
            || BridgeViewModel.shouldSuggestFallback(providerId, diagnosticText))
    visible: active

    sourceComponent: Components.BrowserSessionCard {
        providerId: root.providerId
    }
}

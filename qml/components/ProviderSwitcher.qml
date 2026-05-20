import QtQuick 2.15

TrayProviderDock {
    id: root

    // Compatibility wrapper: existing TrayPanel call sites still import
    // ProviderSwitcher, while the visual system lives in TrayProviderDock.
    implicitHeight: 56
    avatarSize: 34
}

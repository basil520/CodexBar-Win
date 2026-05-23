import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import CodexBarX 1.0
import "components" as Components
import "panes"

Rectangle {
    id: settingsWindow
    width: 960
    height: 640
    color: windowBackgroundColor

    property int rev: LanguageManager.translationRevision
    property bool providersPaneLoaded: false
    readonly property bool glassEffectActive: SettingsStore.glassEffectEnabled
    readonly property color windowBackgroundColor: AppTheme.surfaceWindow
    readonly property color sidebarBackgroundColor: AppTheme.surfaceSidebar
    property var tabs: {
        settingsWindow.rev
        var items = [
            { label: qsTr("General"), icon: "general" },
            { label: qsTr("Providers"), icon: "providers" },
            { label: qsTr("Display"), icon: "display" },
            { label: qsTr("Advanced"), icon: "advanced" },
            { label: qsTr("About"), icon: "about" }
        ]
        if (SettingsStore.debugMenuEnabled) items.push({ label: qsTr("Debug"), icon: "debug" })
        return items
    }

    Components.AcrylicBackdrop {
        anchors.fill: parent
        tint: AppTheme.bgPrimary
    }

    Components.AmbientFluidAurora {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: AppTheme.surfaceBorder
        z: 20
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Components.WindowTitleBar {
            title: qsTr("CodexBar Settings")
            iconText: "C"
            windowKind: "settings"
            showMaximize: true
            maximized: AppController.settingsMaximized
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.preferredWidth: 240
                Layout.fillHeight: true
                color: settingsWindow.sidebarBackgroundColor

                Rectangle {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 1
                    color: AppTheme.surfaceBorder
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 16

                    Label {
                        text: qsTr("Settings")
                        color: AppTheme.textPrimary
                        font.pixelSize: AppTheme.fontSizeXl
                        font.bold: true
                        Layout.topMargin: 8
                    }

                    Label {
                        text: qsTr("Configure providers and app behavior")
                        color: AppTheme.textSecondary
                        font.pixelSize: AppTheme.fontSizeSm
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: AppTheme.surfaceBorder
                    }

                    ListView {
                        id: tabList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: 0
                        spacing: 8
                        clip: true
                        model: settingsWindow.tabs
                        onCurrentIndexChanged: {
                            if (currentIndex === 1) settingsWindow.providersPaneLoaded = true
                        }

                        Rectangle {
                            id: activeSlideBar
                            width: 3
                            height: 22
                            radius: 2
                            color: AppTheme.accentColor
                            z: 10
                            x: 0
                            y: tabList.currentItem ? tabList.currentItem.y + (tabList.currentItem.height - height) / 2 : 0

                            Behavior on y {
                                NumberAnimation {
                                    duration: AppTheme.duration(AppTheme.motionSlow)
                                    easing.type: Easing.OutBack
                                }
                            }
                        }

                        delegate: Rectangle {
                            id: tabButton

                            width: ListView.view.width
                            height: 38
                            radius: 6
                            color: tabList.currentIndex === index
                                ? AppTheme.surfaceSelected
                                : ((tabMouse.containsMouse || activeFocus) ? AppTheme.surfaceHover : "transparent")
                            activeFocusOnTab: true

                            Accessible.role: Accessible.Button
                            Accessible.name: modelData.label

                            function activate() {
                                tabList.currentIndex = index
                            }

                            Keys.onReturnPressed: activate()
                            Keys.onEnterPressed: activate()
                            Keys.onSpacePressed: activate()

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 10
                                spacing: 10

                                Rectangle {
                                    Layout.preferredWidth: 22
                                    Layout.preferredHeight: 22
                                    radius: 5
                                    color: tabList.currentIndex === index
                                        ? AppTheme.accentColor
                                        : AppTheme.surfaceControl

                                    SettingsNavIcon {
                                        anchors.centerIn: parent
                                        width: 14
                                        height: 14
                                        iconName: modelData.icon
                                        strokeColor: tabList.currentIndex === index ? AppTheme.textOnAccent : AppTheme.textSecondary
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: modelData.label
                                    color: tabList.currentIndex === index ? AppTheme.textPrimary : AppTheme.textSecondary
                                    font.pixelSize: AppTheme.fontSizeMd
                                    elide: Text.ElideRight
                                }
                            }

                            MouseArea {
                                id: tabMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: tabButton.activate()
                            }

                            Components.FocusRing {
                                anchors.fill: parent
                                active: tabButton.activeFocus
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"

                StackLayout {
                    id: contentStack
                    anchors.fill: parent
                    currentIndex: tabList.currentIndex

                    PageWrapper {
                        GeneralPane {
                            anchors.fill: parent
                        }
                    }

                    PageWrapper {
                        Loader {
                            id: providersPaneLoader
                            anchors.fill: parent
                            active: settingsWindow.providersPaneLoaded
                            asynchronous: true

                            sourceComponent: ProvidersPane {
                            id: providersPane
                            providers: SettingsProvidersModel.providers
                            providerCount: SettingsProvidersModel.providerCount
                            selectedProvider: SettingsProvidersModel.selectedProvider
                            selectedDescriptor: SettingsProvidersModel.selectedDescriptor
                            detailState: SettingsProvidersModel.detailState
                            selectedConnectionTest: SettingsProvidersModel.selectedConnectionTest
                            selectedProviderStatus: SettingsProvidersModel.selectedProviderStatus
                            selectedProviderError: SettingsProvidersModel.selectedProviderError
                            selectedUsageSnapshot: SettingsProvidersModel.selectedUsageSnapshot
                            tokenAccounts: SettingsProvidersModel.selectedTokenAccounts
                            defaultTokenAccountId: SettingsProvidersModel.selectedDefaultTokenAccountId
                            tokenAccountOperationState: SettingsProvidersModel.tokenAccountOperationState
                            codexAccountState: SettingsProvidersModel.codexAccountState
                            codexProjection: SettingsProvidersModel.codexProjection

                            Component.onCompleted: SettingsProvidersModel.requestOpenProvidersTab()

                            onProviderSelected: function(providerId) {
                                SettingsProvidersModel.selectProvider(providerId)
                            }
                            onProviderEnabled: function(providerId, enabled) {
                                SettingsProvidersModel.setProviderEnabled(providerId, enabled)
                            }
                            onTestConnection: function(providerId) {
                                SettingsProvidersModel.testConnection(providerId)
                            }
                            onRefreshProvider: function(providerId) {
                                SettingsProvidersModel.refreshProvider(providerId)
                            }
                            onSettingChanged: function(providerId, key, value) {
                                SettingsProvidersModel.setProviderSetting(providerId, key, value)
                            }
                            onSecretSaveRequested: function(providerId, key, value) {
                                SettingsProvidersModel.setProviderSecret(providerId, key, value)
                            }
                            onSecretClearRequested: function(providerId, key) {
                                SettingsProvidersModel.clearProviderSecret(providerId, key)
                            }
                            onMoveProvider: function(fromIndex, toIndex) {
                                SettingsProvidersModel.moveProvider(fromIndex, toIndex)
                            }
                            onAddTokenAccount: function(providerId, displayName, sourceMode, apiKey) {
                                if (apiKey && apiKey.trim() !== "") {
                                    SettingsProvidersModel.requestAddTokenAccountWithApiKey(providerId, displayName, sourceMode, apiKey)
                                } else {
                                    SettingsProvidersModel.requestAddTokenAccount(providerId, displayName, sourceMode)
                                }
                            }
                            onRemoveTokenAccount: function(accountId) {
                                SettingsProvidersModel.requestRemoveTokenAccount(accountId)
                            }
                            onSetDefaultTokenAccount: function(providerId, accountId) {
                                SettingsProvidersModel.requestSetDefaultTokenAccount(providerId, accountId)
                            }
                            onSetTokenAccountSourceMode: function(accountId, sourceMode) {
                                SettingsProvidersModel.requestSetTokenAccountSourceMode(accountId, sourceMode)
                            }
                            onSetTokenAccountVisibility: function(accountId, visibility) {
                                SettingsProvidersModel.requestSetTokenAccountVisibility(accountId, visibility)
                            }
                            onSetCodexActiveAccount: function(accountId) {
                                SettingsProvidersModel.setCodexActiveAccount(accountId)
                            }
                            onAddCodexAccount: SettingsProvidersModel.addCodexAccount()
                            onCancelCodexAuthentication: SettingsProvidersModel.cancelCodexAuthentication()
                            onRemoveCodexAccount: function(accountId) {
                                SettingsProvidersModel.removeCodexAccount(accountId)
                            }
                            onReauthenticateCodexAccount: function(accountId) {
                                SettingsProvidersModel.reauthenticateCodexAccount(accountId)
                            }
                            onPromoteCodexAccount: function(accountId) {
                                SettingsProvidersModel.promoteCodexAccount(accountId)
                            }
                        }
                    }
                }

                    PageWrapper { DisplayPane { anchors.fill: parent } }
                    PageWrapper { AdvancedPane { anchors.fill: parent } }
                    PageWrapper { AboutPane { anchors.fill: parent } }
                    PageWrapper { DebugPane { anchors.fill: parent } }
                }
            }
        }
    }

    ResizeHandle { anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom; edge: Qt.LeftEdge }
    ResizeHandle { anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom; edge: Qt.RightEdge }
    ResizeHandle { anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right; edge: Qt.TopEdge }
    ResizeHandle { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; edge: Qt.BottomEdge }
    ResizeHandle { anchors.left: parent.left; anchors.top: parent.top; width: 10; height: 10; edge: Qt.LeftEdge | Qt.TopEdge }
    ResizeHandle { anchors.right: parent.right; anchors.top: parent.top; width: 10; height: 10; edge: Qt.RightEdge | Qt.TopEdge }
    ResizeHandle { anchors.left: parent.left; anchors.bottom: parent.bottom; width: 10; height: 10; edge: Qt.LeftEdge | Qt.BottomEdge }
    ResizeHandle { anchors.right: parent.right; anchors.bottom: parent.bottom; width: 10; height: 10; edge: Qt.RightEdge | Qt.BottomEdge }

    component SettingsNavIcon: Canvas {
        id: navIcon

        property string iconName: ""
        property color strokeColor: AppTheme.textSecondary

        antialiasing: true

        onIconNameChanged: requestPaint()
        onStrokeColorChanged: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = strokeColor
            ctx.fillStyle = strokeColor
            ctx.lineWidth = 1.55
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            var w = width
            var h = height
            var cx = w / 2
            var cy = h / 2

            if (iconName === "general") {
                ctx.beginPath()
                ctx.arc(cx, cy, 2.5, 0, Math.PI * 2)
                ctx.stroke()
                for (var i = 0; i < 8; ++i) {
                    var a = Math.PI * 2 * i / 8
                    ctx.beginPath()
                    ctx.moveTo(cx + Math.cos(a) * 4.2, cy + Math.sin(a) * 4.2)
                    ctx.lineTo(cx + Math.cos(a) * 5.7, cy + Math.sin(a) * 5.7)
                    ctx.stroke()
                }
            } else if (iconName === "providers") {
                ctx.beginPath()
                ctx.moveTo(3.2, 9.8)
                ctx.lineTo(10.7, 9.8)
                ctx.arc(10.3, 8.2, 1.7, Math.PI * 0.45, Math.PI * 1.45, true)
                ctx.arc(7.5, 6.3, 2.5, Math.PI * 1.1, Math.PI * 1.9, true)
                ctx.arc(4.6, 8.1, 1.9, Math.PI * 1.3, Math.PI * 0.55, true)
                ctx.stroke()
            } else if (iconName === "display") {
                ctx.strokeRect(2.2, 3.2, 9.6, 6.5)
                ctx.beginPath()
                ctx.moveTo(cx, 9.7)
                ctx.lineTo(cx, 11.4)
                ctx.moveTo(4.7, 11.5)
                ctx.lineTo(9.3, 11.5)
                ctx.stroke()
            } else if (iconName === "advanced") {
                ctx.beginPath()
                ctx.moveTo(3.0, 10.8)
                ctx.lineTo(8.9, 4.9)
                ctx.moveTo(8.3, 3.3)
                ctx.lineTo(10.7, 5.7)
                ctx.moveTo(9.8, 6.6)
                ctx.lineTo(11.0, 7.8)
                ctx.moveTo(2.8, 4.1)
                ctx.lineTo(5.3, 6.6)
                ctx.stroke()
            } else if (iconName === "about") {
                ctx.beginPath()
                ctx.arc(cx, cy, 5.1, 0, Math.PI * 2)
                ctx.stroke()
                ctx.beginPath()
                ctx.arc(cx, 4.5, 0.65, 0, Math.PI * 2)
                ctx.fill()
                ctx.beginPath()
                ctx.moveTo(cx, 6.6)
                ctx.lineTo(cx, 10.0)
                ctx.stroke()
            } else {
                ctx.beginPath()
                ctx.ellipse(cx, cy + 0.8, 3.4, 4.0, 0, 0, Math.PI * 2)
                ctx.stroke()
                ctx.beginPath()
                ctx.moveTo(3.1, 6.1)
                ctx.lineTo(1.9, 5.0)
                ctx.moveTo(10.9, 6.1)
                ctx.lineTo(12.1, 5.0)
                ctx.moveTo(3.0, 9.0)
                ctx.lineTo(1.8, 9.9)
                ctx.moveTo(11.0, 9.0)
                ctx.lineTo(12.2, 9.9)
                ctx.moveTo(cx, 4.4)
                ctx.lineTo(cx, 11.2)
                ctx.stroke()
            }
        }
    }

    component ResizeHandle: MouseArea {
        property int edge: Qt.LeftEdge
        width: 6
        height: 6
        hoverEnabled: true
        z: 30
        onPressed: AppController.startSettingsResize(edge)
    }

    component PageWrapper: Item {
        id: wrapper
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        
        opacity: visible ? 1.0 : 0.0
        x: visible ? 0 : 8

        Behavior on opacity {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: Easing.OutQuad }
        }
        Behavior on x {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: Easing.OutQuad }
        }
    }
}

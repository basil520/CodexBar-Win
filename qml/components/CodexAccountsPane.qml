import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import CodexBarX 1.0
import ".."

Rectangle {
    id: root
    color: AppTheme.surfacePane
    radius: AppTheme.radiusLg

    property var accounts: []
    property string activeAccountID: ""
    property bool isAuthenticating: false
    property bool isRemoving: false
    property bool isPromoting: false
    property string authenticatingAccountID: ""
    property string removingAccountID: ""
    property string promotingAccountID: ""
    property bool hasUnreadableStore: false
    property string authState: "idle"
    property string authMessage: ""
    property string authError: ""
    property string verificationUri: ""
    property string userCode: ""

    signal setActiveAccount(string accountID)
    signal addAccount()
    signal cancelAuthentication()
    signal openVerificationUrl(string url)
    signal copyText(string text)
    signal removeAccount(string accountID)
    signal reauthenticateAccount(string accountID)
    signal promoteAccount(string accountID)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                text: qsTr("Accounts")
                color: AppTheme.textPrimary
                font.pixelSize: 16
                font.bold: true
            }

            ActionButton {
                text: root.isAuthenticating && root.authenticatingAccountID === ""
                    ? qsTr("Adding...")
                    : qsTr("Add Account")
                variant: "primary"
                compact: true
                enabled: !root.hasUnreadableStore && !root.isAuthenticating && !root.isRemoving
                onClicked: root.addAccount()
            }
        }

        FeedbackBanner {
            Layout.fillWidth: true
            visible: root.isAuthenticating || root.authError !== "" || root.userCode !== ""
            status: root.authError !== "" ? "error" : "warning"
            title: root.authError !== "" ? qsTr("Authorization failed") : qsTr("Codex authorization")
            message: root.authError !== ""
                ? root.authError
                : (root.authMessage !== "" ? root.authMessage : qsTr("Waiting for Codex authorization..."))
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            visible: root.userCode !== "" || root.verificationUri !== ""

            Rectangle {
                Layout.preferredWidth: Math.max(110, codeLabel.implicitWidth + 18)
                Layout.preferredHeight: 30
                radius: 5
                color: AppTheme.surfaceControl
                border.width: 1
                border.color: AppTheme.surfaceBorder
                visible: root.userCode !== ""

                Label {
                    id: codeLabel
                    anchors.centerIn: parent
                    text: root.userCode
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    font.bold: true
                }
            }

            ActionButton {
                text: qsTr("Open")
                compact: true
                variant: "secondary"
                enabled: root.verificationUri !== ""
                onClicked: root.openVerificationUrl(root.verificationUri)
            }

            ActionButton {
                text: qsTr("Copy")
                compact: true
                variant: "secondary"
                enabled: root.userCode !== "" || root.verificationUri !== ""
                onClicked: root.copyText(root.userCode !== "" ? root.userCode : root.verificationUri)
            }

            ActionButton {
                text: qsTr("Cancel")
                compact: true
                variant: "ghost"
                enabled: root.isAuthenticating
                onClicked: root.cancelAuthentication()
            }

            Item { Layout.fillWidth: true }
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("Select the active Codex account for usage tracking.")
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: AppTheme.surfaceBorder
        }

        ListView {
            id: accountList
            Layout.fillWidth: true
            Layout.fillHeight: true
            maximumFlickVelocity: AppTheme.maxScrollVelocity
            flickDeceleration: AppTheme.scrollDeceleration
            boundsBehavior: Flickable.DragAndOvershootBounds
            clip: true
            model: root.accounts
            spacing: 8

            delegate: Rectangle {
                width: accountList.width
                height: 72
                color: modelData.isActive
                    ? AppTheme.surfaceSelected
                    : "transparent"
                radius: 6
                border.color: modelData.isActive ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
                border.width: modelData.isActive ? 2 : 1

                MouseArea {
                    anchors.fill: parent
                    z: 0
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (!modelData.isActive) {
                            root.setActiveAccount(modelData.id)
                        }
                    }
                }

                RowLayout {
                    z: 1
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignTop
                        radius: 20
                        color: modelData.isLive ? AppTheme.statusOk : AppTheme.accentColor

                        Label {
                            anchors.centerIn: parent
                            text: modelData.isLive ? "S" : (modelData.email ? modelData.email.charAt(0).toUpperCase() : "A")
                            color: AppTheme.textOnAccent
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        spacing: 2

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Label {
                                Layout.fillWidth: true
                                text: modelData.displayName
                                color: AppTheme.textPrimary
                                font.pixelSize: 14
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                visible: modelData.isLive
                                Layout.preferredWidth: liveLabel.implicitWidth + 12
                                Layout.preferredHeight: 20
                                Layout.alignment: Qt.AlignVCenter
                                radius: 4
                                color: AppTheme.statusOk

                                Label {
                                    id: liveLabel
                                    anchors.centerIn: parent
                                    text: qsTr("System")
                                    color: AppTheme.textOnAccent
                                    font.pixelSize: 10
                                }
                            }

                            Rectangle {
                                visible: modelData.isActive
                                Layout.preferredWidth: activeLabel.implicitWidth + 12
                                Layout.preferredHeight: 20
                                Layout.alignment: Qt.AlignVCenter
                                radius: 4
                                color: AppTheme.accentColor

                                Label {
                                    id: activeLabel
                                    anchors.centerIn: parent
                                    text: qsTr("Active")
                                    color: AppTheme.textOnAccent
                                    font.pixelSize: 10
                                }
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: modelData.email || qsTr("No email")
                            color: AppTheme.textSecondary
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            visible: modelData.workspaceLabel && modelData.workspaceLabel !== ""
                            text: modelData.workspaceLabel || ""
                            color: AppTheme.textTertiary
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignTop
                        spacing: 4

                        ActionButton {
                            Layout.preferredWidth: 46
                            Layout.preferredHeight: 32
                            width: 46
                            height: 32
                            visible: !modelData.isActive
                            enabled: !root.isAuthenticating && !root.isRemoving
                            text: qsTr("Use")
                            compact: true
                            variant: "ghost"
                            onClicked: root.setActiveAccount(modelData.id)

                            ToolTip.text: qsTr("Set as active")
                            ToolTip.visible: visible && activeFocus
                        }

                        ActionButton {
                            Layout.preferredWidth: 46
                            Layout.preferredHeight: 32
                            width: 46
                            height: 32
                            visible: modelData.canReauthenticate
                            enabled: !root.isAuthenticating && !root.isRemoving
                            text: root.isAuthenticating && root.authenticatingAccountID === modelData.id
                                ? qsTr("Auth...")
                                : qsTr("Auth")
                            compact: true
                            variant: "ghost"
                            onClicked: root.reauthenticateAccount(modelData.id)

                            ToolTip.text: root.isAuthenticating && root.authenticatingAccountID === modelData.id
                                ? qsTr("Re-authenticating...")
                                : qsTr("Re-authenticate")
                            ToolTip.visible: visible && activeFocus
                        }

                        ActionButton {
                            Layout.preferredWidth: 58
                            Layout.preferredHeight: 32
                            width: 58
                            height: 32
                            visible: modelData.canReauthenticate
                            enabled: !root.isAuthenticating && !root.isRemoving && !root.isPromoting
                            text: root.isPromoting && root.promotingAccountID === modelData.id
                                ? qsTr("...")
                                : qsTr("Promote")
                            compact: true
                            variant: "ghost"
                            onClicked: root.promoteAccount(modelData.id)

                            ToolTip.text: root.isPromoting && root.promotingAccountID === modelData.id
                                ? qsTr("Promoting...")
                                : qsTr("Promote to system account")
                            ToolTip.visible: visible && activeFocus
                        }

                        ActionButton {
                            Layout.preferredWidth: 58
                            Layout.preferredHeight: 32
                            width: 58
                            height: 32
                            visible: modelData.canRemove
                            enabled: !root.isAuthenticating && !root.isRemoving
                            text: qsTr("Remove")
                            compact: true
                            variant: "danger"
                            onClicked: {
                                removeDialog.accountEmail = modelData.email || modelData.displayName || ""
                                removeDialog.accountWorkspace = modelData.workspaceLabel || ""
                                removeDialog.pendingAccountID = modelData.id
                                removeDialog.open()
                            }

                            ToolTip.text: root.isRemoving && root.removingAccountID === modelData.id
                                ? qsTr("Removing...")
                                : qsTr("Remove account")
                            ToolTip.visible: visible && activeFocus
                        }
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.accounts.length === 0
            text: qsTr("No accounts configured. Click 'Add Account' to add a Codex account.")
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }

    DeleteConfirmationDialog {
        id: removeDialog
        property string pendingAccountID: ""
        property string accountEmail: ""
        property string accountWorkspace: ""

        title: qsTr("Remove Account?")
        description: qsTr("This will remove the account from CodexBarX. Your data on the server will not be affected.")
        itemLabel: accountEmail
        itemSublabel: accountWorkspace

        onConfirmed: {
            if (pendingAccountID !== "") {
                root.removeAccount(pendingAccountID)
            }
            close()
        }
        onCancelled: {
            pendingAccountID = ""
            close()
        }
    }
}

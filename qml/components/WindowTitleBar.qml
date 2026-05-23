import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import CodexBarX 1.0
import ".."

Rectangle {
    id: root

    property string title: ""
    property string iconText: ""
    property string windowKind: "settings"
    property bool showMaximize: false
    property bool maximized: false

    Layout.fillWidth: true
    Layout.preferredHeight: 44
    color: SettingsStore.glassEffectEnabled ? "transparent" : AppTheme.surfaceTitleBar

    function startMove() {
        if (windowKind === "usage") {
            AppController.startUsageMove()
        } else {
            AppController.startSettingsMove()
        }
    }

    function minimizeWindow() {
        if (windowKind === "usage") {
            AppController.minimizeUsage()
        } else {
            AppController.minimizeSettings()
        }
    }

    function toggleMaximized() {
        if (showMaximize && windowKind === "settings") {
            AppController.toggleSettingsMaximized()
        }
    }

    function closeWindow() {
        if (windowKind === "usage") {
            AppController.closeUsage()
        } else {
            AppController.closeSettings()
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: AppTheme.surfaceBorder
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 8
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 20
            radius: 5
            color: AppTheme.accentColor

            Label {
                anchors.centerIn: parent
                text: root.iconText
                color: AppTheme.textOnAccent
                font.pixelSize: 11
                font.bold: true
            }
        }

        Label {
            text: root.title
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            elide: Text.ElideRight
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onPressed: root.startMove()
                onDoubleClicked: root.toggleMaximized()
            }
        }

        TitleActionButton {
            symbol: "minimize"
            accessibleName: qsTr("Minimize")
            onClicked: root.minimizeWindow()
        }

        TitleActionButton {
            visible: root.showMaximize
            symbol: root.maximized ? "restore" : "maximize"
            accessibleName: root.maximized ? qsTr("Restore") : qsTr("Maximize")
            onClicked: root.toggleMaximized()
        }

        TitleActionButton {
            symbol: "close"
            accessibleName: qsTr("Close")
            danger: true
            onClicked: root.closeWindow()
        }
    }

    component TitleActionButton: Rectangle {
        id: action

        property string symbol: ""
        property string accessibleName: ""
        property bool danger: false
        readonly property bool hovered: mouseArea.containsMouse || activeFocus
        readonly property color glyphColor: hovered && danger ? AppTheme.textOnDanger : AppTheme.textPrimary
        signal clicked()

        Layout.preferredWidth: 36
        Layout.preferredHeight: 30
        radius: 5
        color: hovered
            ? (danger ? AppTheme.statusOutage : AppTheme.surfaceHover)
            : "transparent"
        activeFocusOnTab: true

        Accessible.role: Accessible.Button
        Accessible.name: accessibleName

        function activate() {
            clicked()
        }

        Keys.onReturnPressed: activate()
        Keys.onEnterPressed: activate()
        Keys.onSpacePressed: activate()

        IconGlyph {
            anchors.centerIn: parent
            width: 13
            height: 13
            glyphName: action.symbol
            strokeColor: action.glyphColor
        }

        FocusRing {
            anchors.fill: parent
            active: action.activeFocus
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: action.activate()
        }
    }
}

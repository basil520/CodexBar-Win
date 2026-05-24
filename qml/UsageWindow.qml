import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import CodexBarX 1.0
import "components" as Components
import "panes"

Rectangle {
    id: usageWindow
    width: 800
    height: 600
    color: windowBackgroundColor

    property int rev: LanguageManager.translationRevision
    readonly property bool contentActive: Window.window !== null && Window.window.visible
    readonly property bool glassEffectActive: SettingsStore.glassEffectEnabled
    readonly property color windowBackgroundColor: AppTheme.surfaceWindow

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
            title: qsTr("Usage Details")
            windowKind: "usage"
            showMaximize: false
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            Loader {
                anchors.fill: parent
                active: usageWindow.contentActive
                asynchronous: true
                source: "panes/TokenUsagePane.qml"
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

    component ResizeHandle: MouseArea {
        property int edge: Qt.LeftEdge
        width: 6
        height: 6
        hoverEnabled: true
        z: 30
        onPressed: AppController.startUsageResize(edge)
    }
}

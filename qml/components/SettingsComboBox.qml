import QtQuick 2.15
import QtQuick.Controls 2.15
import ".."

ComboBox {
    id: root
    property var selectedValue: ""
    signal valueActivated(var value)

    implicitHeight: 32
    textRole: "label"
    valueRole: "value"
    font.pixelSize: AppTheme.fontSizeSm

    function syncIndex() {
        var count = model && model.length !== undefined ? model.length : 0
        var idx = -1
        for (var i = 0; i < count; i++) {
            if (model[i].value === root.selectedValue) {
                idx = i
                break
            }
        }
        currentIndex = idx >= 0 ? idx : (count > 0 ? 0 : -1)
    }

    onSelectedValueChanged: syncIndex()
    onModelChanged: syncIndex()
    Component.onCompleted: syncIndex()
    onActivated: function() {
        var count = model && model.length !== undefined ? model.length : 0
        if (currentIndex >= 0 && currentIndex < count) {
            root.valueActivated(model[currentIndex].value)
        }
    }

    contentItem: Text {
        leftPadding: 12
        rightPadding: 28
        text: root.displayText
        font: root.font
        color: root.enabled ? AppTheme.textPrimary : AppTheme.textDisabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        id: arrowText
        x: root.width - width - 10
        y: (root.height - height) / 2
        text: "▾"
        color: AppTheme.textSecondary
        font.pixelSize: AppTheme.fontSizeSm

        rotation: root.popup.visible ? 180 : 0
        transformOrigin: Item.Center
        Behavior on rotation {
            NumberAnimation { duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
        }
    }

    background: Rectangle {
        radius: 6
        color: AppTheme.surfaceControl
        border.width: 1
        border.color: root.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder
    }

    delegate: ItemDelegate {
        width: root.width
        height: 32
        text: modelData.label || modelData

        contentItem: Text {
            text: modelData.label || modelData
            color: highlighted ? AppTheme.textPrimary : AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            color: highlighted ? AppTheme.surfaceHover : "transparent"
            Behavior on color {
                ColorAnimation { duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
            }
        }
    }

    popup: Popup {
        id: popupContainer
        y: root.height + 4
        width: root.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 240)
        padding: 1

        enter: Transition {
            NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: AppTheme.duration(AppTheme.motionNormal); easing.type: AppTheme.easeStandard }
        }
        exit: Transition {
            NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: AppTheme.duration(AppTheme.motionFast); easing.type: AppTheme.easeStandard }
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
        }

        background: Rectangle {
            radius: 6
            color: AppTheme.surfacePopup
            border.width: 1
            border.color: AppTheme.surfaceBorder
        }
    }
}

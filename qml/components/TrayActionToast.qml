import QtQuick 2.15
import QtQuick.Layouts 1.15
import ".."

Rectangle {
    id: root

    property string message: ""
    property int toastType: 0

    readonly property int typeInfo: 0
    readonly property int typeSuccess: 1
    readonly property int typeError: 2
    readonly property int typeWarning: 3
    readonly property color toneColor: toastType === typeError ? AppTheme.statusOutage
        : toastType === typeSuccess ? AppTheme.statusOk
        : toastType === typeWarning ? AppTheme.statusDegraded
        : AppTheme.textSecondary
    readonly property string glyphName: toastType === typeSuccess ? "check"
        : toastType === typeError || toastType === typeWarning ? "warning"
        : "check"

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 52
    width: Math.min(parent.width - 24, messageText.implicitWidth + 44)
    height: messageText.implicitHeight + 16
    radius: AppTheme.radiusMd
    color: AppTheme.surfacePopup
    border.width: 1
    border.color: toastType === typeError ? AppTheme.statusOutage : AppTheme.surfaceBorder
    z: 100
    opacity: 0
    y: anchors.bottomMargin + 20

    function show(msg, type) {
        message = msg
        toastType = type !== undefined ? type : typeInfo
        hideTimer.stop()
        showAnim.start()
        hideTimer.restart()
    }

    ParallelAnimation {
        id: showAnim
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: AppTheme.duration(AppTheme.motionNormal); easing.type: Easing.OutCubic }
        NumberAnimation { target: root; property: "y"; to: anchors.bottomMargin; duration: AppTheme.duration(AppTheme.motionNormal); easing.type: Easing.OutCubic }
    }

    SequentialAnimation {
        id: hideAnim
        NumberAnimation { target: root; property: "opacity"; to: 0; duration: AppTheme.duration(AppTheme.motionFast); easing.type: Easing.InCubic }
        NumberAnimation { target: root; property: "y"; to: anchors.bottomMargin + 20; duration: AppTheme.duration(AppTheme.motionFast); easing.type: Easing.InCubic }
    }

    Timer {
        id: hideTimer
        interval: toastType === typeError ? 4000 : 2500
        onTriggered: hideAnim.start()
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: AppTheme.spacingSm

        IconGlyph {
            Layout.preferredWidth: 14
            Layout.preferredHeight: 14
            glyphName: root.glyphName
            strokeColor: root.toneColor
            fillColor: root.toneColor
        }

        Text {
            id: messageText
            text: root.message
            color: root.toastType === root.typeError ? AppTheme.statusOutage : AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }
}

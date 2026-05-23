import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import ".."

RowLayout {
    id: root
    property string title: ""
    property string subtitle: ""
    property bool checked: false
    signal toggled(bool checked)

    Layout.fillWidth: true
    Layout.preferredHeight: textColumn.implicitHeight + 24
    spacing: 16

    ColumnLayout {
        id: textColumn
        Layout.fillWidth: true
        spacing: 4

        Label {
            text: root.title
            color: AppTheme.textPrimary
            font.pixelSize: AppTheme.fontSizeMd
            font.bold: true
            Layout.fillWidth: true
        }

        Label {
            text: root.subtitle
            color: AppTheme.textSecondary
            font.pixelSize: AppTheme.fontSizeSm
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: root.subtitle !== ""
        }
    }

    SettingsSwitch {
        id: toggleSwitch
        Layout.alignment: Qt.AlignVCenter
        accessibleName: root.title
        checked: root.checked
        onToggled: function(checked) {
            root.toggled(checked)
        }
    }
}

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import ".."

RowLayout {
    id: root
    property string title: ""
    property string subtitle: ""
    property alias model: comboBox.model
    property alias selectedValue: comboBox.selectedValue
    property int comboBoxWidth: 180 // Use standard property instead of attached alias
    signal valueActivated(var value)

    Layout.fillWidth: true
    Layout.preferredHeight: textColumn.implicitHeight + 24
    spacing: 16

    ColumnLayout {
        id: textColumn
        Layout.fillWidth: true
        spacing: 4 // Strict 4px respiratory spacing between Title and Subtitle

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

    SettingsComboBox {
        id: comboBox
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: root.comboBoxWidth // Bind to standard property
        onValueActivated: function(value) {
            root.valueActivated(value)
        }
    }
}

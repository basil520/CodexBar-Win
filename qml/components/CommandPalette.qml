import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ".."

Item {
    id: root

    property bool open: false
    property var commands: []
    property string query: searchField.text

    signal commandTriggered(string commandId)

    visible: open
    z: 1000
    implicitWidth: 460
    implicitHeight: paletteCard.implicitHeight
    focus: open

    function openPalette() {
        open = true
        searchField.forceActiveFocus()
    }

    function closePalette() {
        open = false
        searchField.text = ""
    }

    function filteredCommands() {
        var q = query.toLowerCase()
        if (q === "") return commands
        var result = []
        for (var i = 0; i < commands.length; ++i) {
            var command = commands[i]
            var haystack = ((command.title || "") + " " + (command.subtitle || "") + " " + (command.keywords || "")).toLowerCase()
            if (haystack.indexOf(q) !== -1) result.push(command)
        }
        return result
    }

    Keys.onEscapePressed: function(event) {
        event.accepted = true
        root.closePalette()
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: -12
        radius: AppTheme.radiusLg + 8
        color: AppTheme.withAlpha(AppTheme.bgPrimary, AppTheme.glassActive ? 0.46 : 0.72)
        visible: root.open
    }

    SurfaceCard {
        id: paletteCard
        anchors.fill: parent
        radius: AppTheme.radiusLg
        color: AppTheme.surfacePopup
        border.color: AppTheme.surfaceAccentBorder
        implicitHeight: Math.min(420, 74 + commandList.contentHeight)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: AppTheme.spacingSm

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 38
                radius: AppTheme.radiusMd
                color: AppTheme.surfaceControl
                border.width: 1
                border.color: searchField.activeFocus ? AppTheme.surfaceAccentBorder : AppTheme.surfaceBorder

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    text: qsTr("Search commands, providers, and actions")
                    color: AppTheme.textTertiary
                    font.pixelSize: AppTheme.fontSizeMd
                    elide: Text.ElideRight
                    visible: searchField.text === ""
                }

                TextInput {
                    id: searchField
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    verticalAlignment: TextInput.AlignVCenter
                    color: AppTheme.textPrimary
                    font.pixelSize: AppTheme.fontSizeMd
                    selectByMouse: true
                    clip: true
                }
            }

            ListView {
                id: commandList
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(320, Math.max(46, contentHeight))
                clip: true
                model: root.filteredCommands()
                spacing: 6

                delegate: SurfaceCard {
                    width: commandList.width
                    implicitHeight: 48
                    interactive: true

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: AppTheme.spacingSm

                        IconGlyph {
                            Layout.preferredWidth: 18
                            Layout.preferredHeight: 18
                            glyphName: "search"
                            strokeColor: AppTheme.accentColor
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1

                            Label {
                                Layout.fillWidth: true
                                text: modelData.title || ""
                                color: AppTheme.textPrimary
                                font.pixelSize: AppTheme.fontSizeSm
                                font.bold: true
                                elide: Text.ElideRight
                            }

                            Label {
                                Layout.fillWidth: true
                                text: modelData.subtitle || ""
                                color: AppTheme.textSecondary
                                font.pixelSize: AppTheme.fontSizeXs
                                visible: text !== ""
                                elide: Text.ElideRight
                            }
                        }
                    }

                    TapHandler {
                        onTapped: {
                            root.commandTriggered(modelData.id || modelData.title || "")
                            root.closePalette()
                        }
                    }
                }
            }
        }
    }
}

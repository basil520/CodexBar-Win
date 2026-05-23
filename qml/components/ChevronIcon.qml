import QtQuick 2.15

IconGlyph {
    property bool expanded: false

    glyphName: expanded ? "chevronDown" : "chevronRight"
}

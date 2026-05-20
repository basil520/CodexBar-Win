import QtQuick 2.15

ActionButton {
    id: root

    property bool danger: false
    property bool primary: false

    variant: primary ? "primary" : (danger ? "danger" : "secondary")
    compact: false
}

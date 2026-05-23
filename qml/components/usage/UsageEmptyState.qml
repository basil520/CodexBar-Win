import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.FeedbackBanner {
    property string emptyTitle: qsTr("No usage data")
    property string emptyMessage: qsTr("Enable a provider or refresh usage to populate this view.")

    status: "info"
    title: emptyTitle
    message: emptyMessage
}

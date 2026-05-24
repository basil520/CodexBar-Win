import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../.."
import ".." as Components

Components.FeedbackBanner {
    id: root

    property string forecastText: qsTr("Forecast will appear after usage data refreshes.")
    property string riskLevel: "info"

    status: riskLevel === "error" ? "error"
        : riskLevel === "warning" ? "warning"
        : "info"
    title: qsTr("Forecast & Risk")
    message: forecastText
    compact: false
}

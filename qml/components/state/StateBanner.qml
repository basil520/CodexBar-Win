import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../.." 
import ".." as Components

Components.FeedbackBanner {
    id: root

    property string state: "idle"
    property string reason: ""
    property string nextStep: ""
    property string lifecycle: "persistent"
    property string scope: "global"

    status: {
        if (state === "error" || state === "blocked") return "error"
        if (state === "warning" || state === "stale" || state === "busy") return "warning"
        if (state === "success" || state === "ready") return "success"
        return "info"
    }
    message: {
        var parts = []
        if (reason !== "") parts.push(reason)
        if (nextStep !== "") parts.push(nextStep)
        return parts.join(" ")
    }
}

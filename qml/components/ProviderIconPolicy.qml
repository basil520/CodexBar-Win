import QtQuick 2.15

QtObject {
    id: root

    readonly property var darkGlyphProviders: ({
        "alibaba": true,
        "deepseek": true,
        "kimi": true,
        "kilo": true,
        "opencode": true,
        "opencodego": true,
        "openrouter": true
    })

    readonly property var preserveBackgroundProviders: ({
        "kimik2": true,
        "xfxinchen": true
    })

    readonly property var lightGlyphProviders: ({
        "claude": true,
        "codex": true,
        "codebuff": true,
        "copilot": true,
        "cursor": true,
        "gemini": true,
        "perplexity": true,
        "windsurf": true,
        "zai": true
    })

    function normalize(providerId) {
        return (providerId || "").toString().toLowerCase()
    }

    function iconPolicy(providerId) {
        var id = normalize(providerId)
        if (preserveBackgroundProviders[id] === true) {
            return {
                "tone": "fullColor",
                "surfaceMode": "none",
                "preserveBackground": true,
                "paddingRatio": 0.05
            }
        }
        if (darkGlyphProviders[id] === true) {
            return {
                "tone": "darkGlyph",
                "surfaceMode": "light",
                "preserveBackground": false,
                "paddingRatio": 0.18
            }
        }
        if (lightGlyphProviders[id] === true) {
            return {
                "tone": "lightGlyph",
                "surfaceMode": "brandSoft",
                "preserveBackground": false,
                "paddingRatio": 0.18
            }
        }
        return {
            "tone": "unknown",
            "surfaceMode": "brandSoft",
            "preserveBackground": false,
            "paddingRatio": 0.16
        }
    }
}

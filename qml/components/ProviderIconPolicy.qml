import QtQuick 2.15

QtObject {
    id: root

    property QtObject identityRegistry: ProviderIdentityRegistry {
        id: identityRegistry
    }

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
        var identity = identityRegistry.identityFor(id)
        if (identity.preserveBackground === true || preserveBackgroundProviders[id] === true) {
            return {
                "tone": "fullColor",
                "shape": identity.vesselShape || "native",
                "background": "native",
                "imageMode": "containNative",
                "selectedTreatment": "ring",
                "smallSizeFallback": "native",
                "surfaceMode": "none",
                "preserveBackground": true,
                "paddingRatio": 0.05
            }
        }
        if (identity.iconMode === "darkGlyph" || darkGlyphProviders[id] === true) {
            return {
                "tone": "darkGlyph",
                "shape": identity.vesselShape || "squircle",
                "background": "neutralLight",
                "imageMode": "glyph",
                "selectedTreatment": "ring",
                "smallSizeFallback": "glyph",
                "surfaceMode": "light",
                "preserveBackground": false,
                "paddingRatio": 0.18
            }
        }
        if (identity.iconMode === "lightGlyph" || lightGlyphProviders[id] === true) {
            return {
                "tone": "lightGlyph",
                "shape": identity.vesselShape || "squircle",
                "background": "brandTint",
                "imageMode": "glyph",
                "selectedTreatment": "ring",
                "smallSizeFallback": "glyph",
                "surfaceMode": "brandSoft",
                "preserveBackground": false,
                "paddingRatio": 0.18
            }
        }
        return {
            "tone": "unknown",
            "shape": "squircle",
            "background": "brandTint",
            "imageMode": "glyph",
            "selectedTreatment": "ring",
            "smallSizeFallback": "initial",
            "surfaceMode": "brandSoft",
            "preserveBackground": false,
            "paddingRatio": 0.16
        }
    }
}

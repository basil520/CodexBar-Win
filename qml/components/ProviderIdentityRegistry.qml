import QtQuick 2.15
import ".."

QtObject {
    id: root

    readonly property var identities: ({
        "kimik2": {
            "iconMode": "preserve",
            "vesselShape": "native",
            "preserveBackground": true,
            "needsStroke": true
        },
        "xfxinchen": {
            "iconMode": "preserve",
            "vesselShape": "native",
            "preserveBackground": true,
            "needsStroke": true
        },
        "alibaba": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "deepseek": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "kimi": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "kilo": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "opencode": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "opencodego": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "openrouter": { "iconMode": "darkGlyph", "vesselShape": "squircle" },
        "claude": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "codex": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "codebuff": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "copilot": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "cursor": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "gemini": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "perplexity": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "windsurf": { "iconMode": "lightGlyph", "vesselShape": "squircle" },
        "zai": { "iconMode": "lightGlyph", "vesselShape": "squircle" }
    })

    function normalize(providerId) {
        return (providerId || "").toString().toLowerCase()
    }

    function identityFor(providerId) {
        var id = normalize(providerId)
        var identity = identities[id] || ({})
        return {
            "providerId": id,
            "brandColor": AppTheme.providerBrandColor(id),
            "accentColor": AppTheme.providerBrandColor(id),
            "iconMode": identity.iconMode || "auto",
            "vesselShape": identity.vesselShape || "squircle",
            "preserveBackground": identity.preserveBackground === true,
            "needsStroke": identity.needsStroke === true,
            "preferredBadge": identity.preferredBadge || "status",
            "supportsUsageRing": identity.supportsUsageRing !== false,
            "supportsAccountBadge": identity.supportsAccountBadge !== false,
            "shortName": identity.shortName || (id.length > 0 ? id.charAt(0).toUpperCase() : "?"),
            "tooltipName": identity.tooltipName || id,
            "prefersDarkBackground": identity.iconMode === "lightGlyph"
        }
    }
}

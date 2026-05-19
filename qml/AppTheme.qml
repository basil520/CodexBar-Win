import QtQuick 2.15
import CodexBarX 1.0

pragma Singleton

QtObject {
    property color bgPrimary: AppThemeCpp.bgPrimary
    property color bgSecondary: AppThemeCpp.bgSecondary
    property color bgTertiary: AppThemeCpp.bgTertiary
    property color bgCard: AppThemeCpp.bgCard
    property color bgHover: AppThemeCpp.bgHover
    property color bgSelected: AppThemeCpp.bgSelected
    property color bgPressed: AppThemeCpp.bgPressed
    property color bgTitleBar: AppThemeCpp.bgTitleBar
    property color bgChart: AppThemeCpp.bgChart
    property color bgTrack: AppThemeCpp.bgTrack

    property color borderColor: AppThemeCpp.borderColor
    property color borderAccent: AppThemeCpp.borderAccent

    property color textPrimary: AppThemeCpp.textPrimary
    property color textSecondary: AppThemeCpp.textSecondary
    property color textTertiary: AppThemeCpp.textTertiary
    property color textDisabled: AppThemeCpp.textDisabled
    property color textInverse: AppThemeCpp.textInverse
    property color textOnAccent: "#ffffff"

    property color statusOk: AppThemeCpp.statusOk
    property color statusDegraded: AppThemeCpp.statusDegraded
    property color statusOutage: AppThemeCpp.statusOutage
    property color statusUnknown: AppThemeCpp.statusUnknown

    property color accentColor: AppThemeCpp.accentColor
    property color accentHover: AppThemeCpp.accentHover

    readonly property bool glassActive: SettingsStore.glassEffectEnabled
    readonly property real glassMaterialOpacity: Math.min(0.95, Math.max(0.05, SettingsStore.glassEffectOpacity / 100))

    property color surfaceWindow: glassActive ? withAlpha(bgPrimary, glassMaterialOpacity) : bgPrimary
    property color surfaceTitleBar: glassActive ? withAlpha(bgTitleBar, Math.min(0.84, Math.max(0.34, glassMaterialOpacity + 0.12))) : bgTitleBar
    property color surfacePane: glassActive ? withAlpha(bgSecondary, Math.min(0.84, Math.max(0.38, glassMaterialOpacity + 0.18))) : bgSecondary
    property color surfaceCard: glassActive ? withAlpha(bgCard, Math.min(0.88, Math.max(0.42, glassMaterialOpacity + 0.20))) : bgCard
    property color surfaceElevated: glassActive ? withAlpha(bgCard, Math.min(0.94, Math.max(0.52, glassMaterialOpacity + 0.30))) : bgCard
    property color surfaceControl: glassActive ? withAlpha(bgPrimary, Math.min(0.88, Math.max(0.48, glassMaterialOpacity + 0.24))) : bgPrimary
    property color surfacePopup: glassActive ? withAlpha(bgCard, Math.min(0.96, Math.max(0.62, glassMaterialOpacity + 0.32))) : bgCard
    property color surfaceChart: glassActive ? withAlpha(bgChart, Math.min(0.90, Math.max(0.46, glassMaterialOpacity + 0.24))) : bgChart
    property color surfaceHover: glassActive ? withAlpha(bgHover, Math.min(0.76, Math.max(0.36, glassMaterialOpacity + 0.18))) : bgHover
    property color surfacePressed: glassActive ? withAlpha(bgPressed, Math.min(0.82, Math.max(0.42, glassMaterialOpacity + 0.22))) : bgPressed
    property color surfaceSelected: glassActive ? withAlpha(bgSelected, Math.min(0.86, Math.max(0.44, glassMaterialOpacity + 0.24))) : bgSelected
    property color surfaceTrack: glassActive ? withAlpha(bgTrack, Math.min(0.84, Math.max(0.36, glassMaterialOpacity + 0.16))) : bgTrack
    property color surfaceBorder: glassActive ? withAlpha(borderColor, Math.min(0.74, Math.max(0.38, glassMaterialOpacity + 0.12))) : borderColor
    property color surfaceAccentBorder: glassActive ? withAlpha(borderAccent, Math.min(0.86, Math.max(0.58, glassMaterialOpacity + 0.18))) : borderAccent

    property var providerBrandColors: ({
        "codex": "#49A3B0", "claude": "#CC7C5E", "cursor": "#5B8DFA",
        "gemini": "#8860D0", "copilot": "#2DA44E", "zai": "#E85A6A",
        "opencode": "#E44D26", "warp": "#00BCD4", "mistral": "#F77F00",
        "openrouter": "#FF6B6B", "ollama": "#E6EF6C", "kilo": "#7C3AED",
        "kiro": "#F59E0B", "kimik2": "#06B6D4", "minimax": "#EC4899",
        "perplexity": "#22C55E", "kimi": "#8B5CF6", "abacus": "#6366F1",
        "alibaba": "#F97316", "augment": "#14B8A6", "amp": "#D946EF",
        "factory": "#84CC16", "jetbrains": "#F000F0", "vertexai": "#4285F4",
        "deepseek": "#4D6BFE", "codebuff": "#44FF00", "windsurf": "#34E8BB",
        "antigravity": "#10B981", "synthetic": "#6366F1", "opencodego": "#3B82F6",
        "qianfan": "#2932E1"
    })

    function withAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    function providerBrandColor(providerId) {
        return providerBrandColors[providerId] || accentColor
    }

    property int spacingXs: 4
    property int spacingSm: 8
    property int spacingMd: 12
    property int spacingLg: 16
    property int spacingXl: 24

    property int radiusSm: 4
    property int radiusMd: 8
    property int radiusLg: 12

    property int fontSizeXs: 10
    property int fontSizeSm: 11
    property int fontSizeMd: 13
    property int fontSizeLg: 16
    property int fontSizeXl: 20

    property int sidebarWidth: 240
    property int listItemHeight: 48
    property int iconSizeSm: 18
    property int iconSizeMd: 24
    property int iconSizeLg: 28
    property int statusDotSize: 6
    property int progressBarHeight: 6
}

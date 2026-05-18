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

    property color statusOk: AppThemeCpp.statusOk
    property color statusDegraded: AppThemeCpp.statusDegraded
    property color statusOutage: AppThemeCpp.statusOutage
    property color statusUnknown: AppThemeCpp.statusUnknown

    property color accentColor: AppThemeCpp.accentColor
    property color accentHover: AppThemeCpp.accentHover

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

#include "AppTheme.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQml>

AppThemeManager::AppThemeManager(QObject* parent)
    : QObject(parent)
{}

QColor AppThemeManager::bgPrimary() const { return currentColorSet().bgPrimary; }
QColor AppThemeManager::bgSecondary() const { return currentColorSet().bgSecondary; }
QColor AppThemeManager::bgTertiary() const { return currentColorSet().bgTertiary; }
QColor AppThemeManager::bgCard() const { return currentColorSet().bgCard; }
QColor AppThemeManager::bgHover() const { return currentColorSet().bgHover; }
QColor AppThemeManager::bgSelected() const { return currentColorSet().bgSelected; }
QColor AppThemeManager::bgPressed() const { return currentColorSet().bgPressed; }
QColor AppThemeManager::bgTitleBar() const { return currentColorSet().bgTitleBar; }
QColor AppThemeManager::bgChart() const { return currentColorSet().bgChart; }
QColor AppThemeManager::bgTrack() const { return currentColorSet().bgTrack; }

QColor AppThemeManager::borderColor() const { return currentColorSet().borderColor; }
QColor AppThemeManager::borderAccent() const { return currentColorSet().borderAccent; }

QColor AppThemeManager::textPrimary() const { return currentColorSet().textPrimary; }
QColor AppThemeManager::textSecondary() const { return currentColorSet().textSecondary; }
QColor AppThemeManager::textTertiary() const { return currentColorSet().textTertiary; }
QColor AppThemeManager::textDisabled() const { return currentColorSet().textDisabled; }
QColor AppThemeManager::textInverse() const { return currentColorSet().textInverse; }

QColor AppThemeManager::statusOk() const { return currentColorSet().statusOk; }
QColor AppThemeManager::statusDegraded() const { return currentColorSet().statusDegraded; }
QColor AppThemeManager::statusOutage() const { return currentColorSet().statusOutage; }
QColor AppThemeManager::statusUnknown() const { return currentColorSet().statusUnknown; }

QColor AppThemeManager::accentColor() const { return currentColorSet().accentColor; }
QColor AppThemeManager::accentHover() const { return currentColorSet().accentHover; }

int AppThemeManager::spacingXs() const { return 4; }
int AppThemeManager::spacingSm() const { return 8; }
int AppThemeManager::spacingMd() const { return 12; }
int AppThemeManager::spacingLg() const { return 16; }
int AppThemeManager::spacingXl() const { return 24; }

int AppThemeManager::radiusSm() const { return 4; }
int AppThemeManager::radiusMd() const { return 8; }
int AppThemeManager::radiusLg() const { return 12; }

int AppThemeManager::fontSizeXs() const { return 10; }
int AppThemeManager::fontSizeSm() const { return 11; }
int AppThemeManager::fontSizeMd() const { return 13; }
int AppThemeManager::fontSizeLg() const { return 16; }
int AppThemeManager::fontSizeXl() const { return 20; }

int AppThemeManager::sidebarWidth() const { return 240; }
int AppThemeManager::listItemHeight() const { return 48; }
int AppThemeManager::iconSizeSm() const { return 18; }
int AppThemeManager::iconSizeMd() const { return 24; }
int AppThemeManager::iconSizeLg() const { return 28; }
int AppThemeManager::statusDotSize() const { return 6; }
int AppThemeManager::progressBarHeight() const { return 6; }

int AppThemeManager::currentTheme() const {
    return m_currentTheme;
}

void AppThemeManager::setCurrentTheme(int theme) {
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        m_cacheDirty = true;
        emit themeChanged();
    }
}

const AppThemeManager::ColorSet& AppThemeManager::currentColorSet() const {
    if (m_cacheDirty) {
        m_cachedColorSet = colorSet(static_cast<Theme>(m_currentTheme));
        m_cacheDirty = false;
    }
    return m_cachedColorSet;
}

AppThemeManager::ColorSet AppThemeManager::colorSet(Theme theme) const {
    ColorSet c;
    switch (theme) {
    case MidnightBlue:
        c.bgPrimary      = QColor(0x0f, 0x17, 0x2a);
        c.bgSecondary    = QColor(0x0b, 0x12, 0x21);
        c.bgTertiary     = QColor(0x1e, 0x29, 0x3b);
        c.bgCard         = QColor(0x1e, 0x29, 0x3b);
        c.bgHover        = QColor(0x33, 0x41, 0x55);
        c.bgSelected     = QColor(0x47, 0x55, 0x69);
        c.bgPressed      = QColor(0x64, 0x74, 0x8b);
        c.bgTitleBar     = QColor(0x08, 0x0c, 0x16);
        c.bgChart        = QColor(0x16, 0x20, 0x32);
        c.bgTrack        = QColor(0x33, 0x41, 0x55);
        c.borderColor    = QColor(0x33, 0x41, 0x55);
        c.borderAccent   = QColor(0x47, 0x55, 0x69);
        c.textPrimary    = QColor(0xf1, 0xf5, 0xf9);
        c.textSecondary  = QColor(0x94, 0xa3, 0xb8);
        c.textTertiary   = QColor(0x64, 0x74, 0x8b);
        c.textDisabled   = QColor(0x47, 0x55, 0x69);
        c.textInverse    = QColor(0x64, 0x74, 0x8b);
        c.statusOk       = QColor(0x4C, 0xAF, 0x50);
        c.statusDegraded = QColor(0xFF, 0xC1, 0x07);
        c.statusOutage   = QColor(0xF4, 0x43, 0x36);
        c.statusUnknown  = QColor(0x88, 0x88, 0x88);
        c.accentColor    = QColor(0x3b, 0x82, 0xf6);
        c.accentHover    = QColor(0x60, 0xa5, 0xfa);
        break;

    case Amethyst:
        c.bgPrimary      = QColor(0x1a, 0x10, 0x25);
        c.bgSecondary    = QColor(0x14, 0x0d, 0x1f);
        c.bgTertiary     = QColor(0x2a, 0x1f, 0x3a);
        c.bgCard         = QColor(0x25, 0x1a, 0x35);
        c.bgHover        = QColor(0x3a, 0x2d, 0x4d);
        c.bgSelected     = QColor(0x4d, 0x3d, 0x66);
        c.bgPressed      = QColor(0x66, 0x52, 0x80);
        c.bgTitleBar     = QColor(0x0f, 0x0a, 0x18);
        c.bgChart        = QColor(0x1a, 0x14, 0x25);
        c.bgTrack        = QColor(0x3a, 0x2d, 0x4d);
        c.borderColor    = QColor(0x3a, 0x2d, 0x4d);
        c.borderAccent   = QColor(0x5d, 0x4e, 0x75);
        c.textPrimary    = QColor(0xf3, 0xe8, 0xff);
        c.textSecondary  = QColor(0xc4, 0xb5, 0xe0);
        c.textTertiary   = QColor(0x9e, 0x8e, 0xc1);
        c.textDisabled   = QColor(0x6d, 0x5f, 0x8a);
        c.textInverse    = QColor(0x6d, 0x5f, 0x8a);
        c.statusOk       = QColor(0x4C, 0xAF, 0x50);
        c.statusDegraded = QColor(0xFF, 0xC1, 0x07);
        c.statusOutage   = QColor(0xF4, 0x43, 0x36);
        c.statusUnknown  = QColor(0x88, 0x88, 0x88);
        c.accentColor    = QColor(0xa8, 0x55, 0xf7);
        c.accentHover    = QColor(0xc0, 0x84, 0xfc);
        break;

    case Dark:
    default:
        c.bgPrimary      = QColor(0x1a, 0x1a, 0x2e);
        c.bgSecondary    = QColor(0x15, 0x15, 0x2a);
        c.bgTertiary     = QColor(0x25, 0x25, 0x40);
        c.bgCard         = QColor(0x1f, 0x1f, 0x38);
        c.bgHover        = QColor(0x2a, 0x2a, 0x4a);
        c.bgSelected     = QColor(0x3a, 0x3a, 0x5c);
        c.bgPressed      = QColor(0x4a, 0x4a, 0x7c);
        c.bgTitleBar     = QColor(0x10, 0x10, 0x1f);
        c.bgChart        = QColor(0x1c, 0x1c, 0x32);
        c.bgTrack        = QColor(0x2a, 0x2a, 0x4a);
        c.borderColor    = QColor(0x2a, 0x2a, 0x4a);
        c.borderAccent   = QColor(0x4a, 0x4a, 0x7c);
        c.textPrimary    = QColor(0xff, 0xff, 0xff);
        c.textSecondary  = QColor(0xaa, 0xaa, 0xaa);
        c.textTertiary   = QColor(0x88, 0x88, 0x88);
        c.textDisabled   = QColor(0x55, 0x55, 0x55);
        c.textInverse    = QColor(0x66, 0x66, 0x66);
        c.statusOk       = QColor(0x4C, 0xAF, 0x50);
        c.statusDegraded = QColor(0xFF, 0xC1, 0x07);
        c.statusOutage   = QColor(0xF4, 0x43, 0x36);
        c.statusUnknown  = QColor(0x88, 0x88, 0x88);
        c.accentColor    = QColor(0x6b, 0x6b, 0xff);
        c.accentHover    = QColor(0x8a, 0x8a, 0xff);
        break;
    }
    return c;
}

void registerAppThemeTypes(AppThemeManager* themeManager) {
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "AppThemeCpp", themeManager);
}

void installAppTheme(QQmlEngine& engine, AppThemeManager* themeManager) {
    engine.rootContext()->setContextProperty(QStringLiteral("AppTheme"), themeManager);
    engine.rootContext()->setContextProperty(QStringLiteral("AppThemeCpp"), themeManager);
}

#pragma once

#include <QColor>
#include <QObject>

class QQmlEngine;

class AppThemeManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QColor bgPrimary READ bgPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor bgSecondary READ bgSecondary NOTIFY themeChanged)
    Q_PROPERTY(QColor bgTertiary READ bgTertiary NOTIFY themeChanged)
    Q_PROPERTY(QColor bgCard READ bgCard NOTIFY themeChanged)
    Q_PROPERTY(QColor bgHover READ bgHover NOTIFY themeChanged)
    Q_PROPERTY(QColor bgSelected READ bgSelected NOTIFY themeChanged)
    Q_PROPERTY(QColor bgPressed READ bgPressed NOTIFY themeChanged)
    Q_PROPERTY(QColor bgTitleBar READ bgTitleBar NOTIFY themeChanged)
    Q_PROPERTY(QColor bgChart READ bgChart NOTIFY themeChanged)
    Q_PROPERTY(QColor bgTrack READ bgTrack NOTIFY themeChanged)

    Q_PROPERTY(QColor borderColor READ borderColor NOTIFY themeChanged)
    Q_PROPERTY(QColor borderAccent READ borderAccent NOTIFY themeChanged)

    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY themeChanged)
    Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY themeChanged)
    Q_PROPERTY(QColor textDisabled READ textDisabled NOTIFY themeChanged)
    Q_PROPERTY(QColor textInverse READ textInverse NOTIFY themeChanged)

    Q_PROPERTY(QColor statusOk READ statusOk NOTIFY themeChanged)
    Q_PROPERTY(QColor statusDegraded READ statusDegraded NOTIFY themeChanged)
    Q_PROPERTY(QColor statusOutage READ statusOutage NOTIFY themeChanged)
    Q_PROPERTY(QColor statusUnknown READ statusUnknown NOTIFY themeChanged)

    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentHover READ accentHover NOTIFY themeChanged)

    Q_PROPERTY(int spacingXs READ spacingXs CONSTANT)
    Q_PROPERTY(int spacingSm READ spacingSm CONSTANT)
    Q_PROPERTY(int spacingMd READ spacingMd CONSTANT)
    Q_PROPERTY(int spacingLg READ spacingLg CONSTANT)
    Q_PROPERTY(int spacingXl READ spacingXl CONSTANT)

    Q_PROPERTY(int radiusSm READ radiusSm CONSTANT)
    Q_PROPERTY(int radiusMd READ radiusMd CONSTANT)
    Q_PROPERTY(int radiusLg READ radiusLg CONSTANT)

    Q_PROPERTY(int fontSizeXs READ fontSizeXs CONSTANT)
    Q_PROPERTY(int fontSizeSm READ fontSizeSm CONSTANT)
    Q_PROPERTY(int fontSizeMd READ fontSizeMd CONSTANT)
    Q_PROPERTY(int fontSizeLg READ fontSizeLg CONSTANT)
    Q_PROPERTY(int fontSizeXl READ fontSizeXl CONSTANT)

    Q_PROPERTY(int sidebarWidth READ sidebarWidth CONSTANT)
    Q_PROPERTY(int listItemHeight READ listItemHeight CONSTANT)
    Q_PROPERTY(int iconSizeSm READ iconSizeSm CONSTANT)
    Q_PROPERTY(int iconSizeMd READ iconSizeMd CONSTANT)
    Q_PROPERTY(int iconSizeLg READ iconSizeLg CONSTANT)
    Q_PROPERTY(int statusDotSize READ statusDotSize CONSTANT)
    Q_PROPERTY(int progressBarHeight READ progressBarHeight CONSTANT)

    Q_PROPERTY(int currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY themeChanged)

public:
    enum Theme {
        Dark = 0,
        MidnightBlue = 1,
        Amethyst = 2,
        Light = 3
    };
    Q_ENUM(Theme)

    explicit AppThemeManager(QObject* parent = nullptr);

    QColor bgPrimary() const;
    QColor bgSecondary() const;
    QColor bgTertiary() const;
    QColor bgCard() const;
    QColor bgHover() const;
    QColor bgSelected() const;
    QColor bgPressed() const;
    QColor bgTitleBar() const;
    QColor bgChart() const;
    QColor bgTrack() const;

    QColor borderColor() const;
    QColor borderAccent() const;

    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textTertiary() const;
    QColor textDisabled() const;
    QColor textInverse() const;

    QColor statusOk() const;
    QColor statusDegraded() const;
    QColor statusOutage() const;
    QColor statusUnknown() const;

    QColor accentColor() const;
    QColor accentHover() const;

    int spacingXs() const;
    int spacingSm() const;
    int spacingMd() const;
    int spacingLg() const;
    int spacingXl() const;

    int radiusSm() const;
    int radiusMd() const;
    int radiusLg() const;

    int fontSizeXs() const;
    int fontSizeSm() const;
    int fontSizeMd() const;
    int fontSizeLg() const;
    int fontSizeXl() const;

    int sidebarWidth() const;
    int listItemHeight() const;
    int iconSizeSm() const;
    int iconSizeMd() const;
    int iconSizeLg() const;
    int statusDotSize() const;
    int progressBarHeight() const;

    int currentTheme() const;
    void setCurrentTheme(int theme);

signals:
    void themeChanged();

private:
    struct ColorSet {
        QColor bgPrimary;
        QColor bgSecondary;
        QColor bgTertiary;
        QColor bgCard;
        QColor bgHover;
        QColor bgSelected;
        QColor bgPressed;
        QColor bgTitleBar;
        QColor bgChart;
        QColor bgTrack;
        QColor borderColor;
        QColor borderAccent;
        QColor textPrimary;
        QColor textSecondary;
        QColor textTertiary;
        QColor textDisabled;
        QColor textInverse;
        QColor statusOk;
        QColor statusDegraded;
        QColor statusOutage;
        QColor statusUnknown;
        QColor accentColor;
        QColor accentHover;
    };

    ColorSet colorSet(Theme theme) const;
    const ColorSet& currentColorSet() const;

    int m_currentTheme = Dark;
    mutable ColorSet m_cachedColorSet;
    mutable bool m_cacheDirty = true;
};

void registerAppThemeTypes(AppThemeManager* themeManager);
void installAppTheme(QQmlEngine& engine, AppThemeManager* themeManager);

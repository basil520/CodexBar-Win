#include "app/AppTheme.h"

#include <QColor>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtTest/QtTest>
#include <QVariant>
#include <QVariantMap>

#include <algorithm>
#include <cmath>
#include <memory>

namespace {

double srgbToLinear(double channel)
{
    return channel <= 0.03928 ? channel / 12.92 : std::pow((channel + 0.055) / 1.055, 2.4);
}

double relativeLuminance(const QColor& color)
{
    return 0.2126 * srgbToLinear(color.redF())
        + 0.7152 * srgbToLinear(color.greenF())
        + 0.0722 * srgbToLinear(color.blueF());
}

double contrastRatio(const QColor& a, const QColor& b)
{
    const double lighter = std::max(relativeLuminance(a), relativeLuminance(b));
    const double darker = std::min(relativeLuminance(a), relativeLuminance(b));
    return (lighter + 0.05) / (darker + 0.05);
}

} // namespace

class tst_AppTheme : public QObject {
    Q_OBJECT

private slots:
    void runtimeColorsAreOpaque();
    void lightThemeProvidesReadableBrightPalette();
    void qmlThemeFileUsesInstalledCppTheme();
    void installedContextThemeExposesMetricsAndColors();
};

void tst_AppTheme::runtimeColorsAreOpaque() {
    AppThemeManager theme;
    const QVariantMap colors = {
        {"bgPrimary", theme.bgPrimary()},
        {"bgSecondary", theme.bgSecondary()},
        {"bgCard", theme.bgCard()},
        {"bgHover", theme.bgHover()},
        {"bgSelected", theme.bgSelected()},
        {"bgPressed", theme.bgPressed()},
        {"borderColor", theme.borderColor()},
        {"borderAccent", theme.borderAccent()},
        {"textPrimary", theme.textPrimary()},
        {"textSecondary", theme.textSecondary()},
        {"textTertiary", theme.textTertiary()},
        {"textDisabled", theme.textDisabled()},
        {"statusOk", theme.statusOk()},
        {"statusDegraded", theme.statusDegraded()},
        {"statusOutage", theme.statusOutage()},
        {"statusUnknown", theme.statusUnknown()},
        {"accentColor", theme.accentColor()},
        {"accentHover", theme.accentHover()},
    };
    const QStringList colorKeys = {
        "bgPrimary",
        "bgSecondary",
        "bgCard",
        "bgHover",
        "bgSelected",
        "bgPressed",
        "borderColor",
        "borderAccent",
        "textPrimary",
        "textSecondary",
        "textTertiary",
        "textDisabled",
        "statusOk",
        "statusDegraded",
        "statusOutage",
        "statusUnknown",
        "accentColor",
        "accentHover",
    };

    for (const QString& key : colorKeys) {
        QVERIFY2(colors.contains(key), qPrintable(QString("missing theme key %1").arg(key)));

        const QColor color = colors.value(key).value<QColor>();
        QVERIFY2(color.isValid(), qPrintable(QString("invalid theme color %1").arg(key)));
        QCOMPARE(color.alpha(), 255);
    }
}

void tst_AppTheme::lightThemeProvidesReadableBrightPalette() {
    AppThemeManager theme;
    theme.setCurrentTheme(AppThemeManager::Light);

    QVERIFY2(theme.bgPrimary().lightnessF() > 0.88,
             "Light theme primary background must read as a bright application surface.");
    QVERIFY2(theme.bgCard().lightnessF() > 0.92,
             "Light theme cards must stay airy instead of becoming gray panels.");
    QVERIFY2(theme.bgChart().lightnessF() > 0.90,
             "Light theme charts need a bright canvas for dense chart marks.");
    QVERIFY2(theme.textPrimary().lightnessF() < 0.24,
             "Light theme primary text must be dark enough for small tray labels.");
    QVERIFY2(theme.textSecondary().lightnessF() < 0.45,
             "Light theme secondary text must remain readable on light cards.");
    QVERIFY2(contrastRatio(theme.bgPrimary(), theme.textPrimary()) >= 10.0,
             "Light theme primary text contrast must be comfortably above WCAG AA.");
    QVERIFY2(contrastRatio(theme.bgCard(), theme.textSecondary()) >= 4.5,
             "Light theme secondary text contrast must meet WCAG AA on cards.");
    QVERIFY2(contrastRatio(theme.accentColor(), QColor(Qt::white)) >= 4.5,
             "Light theme accent must support white text for existing accent buttons.");
}

void tst_AppTheme::qmlThemeFileUsesInstalledCppTheme() {
    AppThemeManager theme;
    theme.setCurrentTheme(AppThemeManager::Dark);
    registerAppThemeTypes(&theme);

    QQmlEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml"));
    installAppTheme(engine, &theme);

    QQmlComponent component(&engine);
    component.setData(R"QML(
        import QtQuick 2.15
        import "." as Theme

        QtObject {
            property color bgPrimary: Theme.AppTheme.bgPrimary
            property color textPrimary: Theme.AppTheme.textPrimary
            property color accentColor: Theme.AppTheme.accentColor
            property int fontSizeMd: Theme.AppTheme.fontSizeMd
        }
    )QML", QUrl(QStringLiteral("qrc:/qml/AppThemeSingletonProbe.qml")));

    QVERIFY2(component.status() == QQmlComponent::Ready,
             qPrintable(component.errorString()));

    std::unique_ptr<QObject> root(component.create());
    QVERIFY(root != nullptr);
    QCOMPARE(root->property("bgPrimary").value<QColor>(), theme.bgPrimary());
    QCOMPARE(root->property("textPrimary").value<QColor>(), theme.textPrimary());
    QCOMPARE(root->property("accentColor").value<QColor>(), theme.accentColor());
    QCOMPARE(root->property("fontSizeMd").toInt(), 13);
}

void tst_AppTheme::installedContextThemeExposesMetricsAndColors() {
    QQmlEngine engine;
    AppThemeManager theme;
    theme.setCurrentTheme(AppThemeManager::Dark);
    installAppTheme(engine, &theme);

    QQmlComponent component(&engine);
    component.setData(R"QML(
        import QtQuick 2.15

        QtObject {
            property color bgPrimary: AppTheme.bgPrimary
            property color textPrimary: AppTheme.textPrimary
            property color accentColor: AppTheme.accentColor
            property int fontSizeMd: AppTheme.fontSizeMd
            property int spacingMd: AppTheme.spacingMd
            property int radiusMd: AppTheme.radiusMd
        }
    )QML", QUrl(QStringLiteral("qrc:/tests/AppThemeContextProbe.qml")));

    QVERIFY2(component.status() == QQmlComponent::Ready,
             qPrintable(component.errorString()));

    std::unique_ptr<QObject> root(component.create());
    QVERIFY(root != nullptr);
    QCOMPARE(root->property("bgPrimary").value<QColor>(), theme.bgPrimary());
    QCOMPARE(root->property("textPrimary").value<QColor>(), theme.textPrimary());
    QCOMPARE(root->property("accentColor").value<QColor>(), theme.accentColor());
    QCOMPARE(root->property("fontSizeMd").toInt(), 13);
    QCOMPARE(root->property("spacingMd").toInt(), 12);
    QCOMPARE(root->property("radiusMd").toInt(), 8);
}

QTEST_MAIN(tst_AppTheme)
#include "tst_AppTheme.moc"

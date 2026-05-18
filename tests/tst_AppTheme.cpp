#include "app/AppTheme.h"

#include <QColor>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtTest/QtTest>
#include <QVariant>
#include <QVariantMap>

#include <memory>

class tst_AppTheme : public QObject {
    Q_OBJECT

private slots:
    void runtimeColorsAreOpaque();
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

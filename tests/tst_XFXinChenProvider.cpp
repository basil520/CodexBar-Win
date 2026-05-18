#include <QtTest/QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include "providers/xfxinchen/XFXinChenProvider.h"
#include "providers/ProviderFetchContext.h"

class tst_XFXinChenProvider : public QObject {
    Q_OBJECT

private slots:
    void labelsAreCorrect();
    void sourceModes();
    void dashboardURL();
    void brandColor();
    void manualCookieMakesStrategyAvailable();
    void envVarMakesStrategyAvailable();
    void importedBrowserSessionCookieMakesStrategyAvailable();
    void noCookieIsNotAvailable();
    void parseResponseFull();
    void parseResponseNotSucceed();
    void parseResponseEmptyRows();
    void parseResponseNoActivePlan();
    void parseResponseMissingUsageDTO();
};

void tst_XFXinChenProvider::labelsAreCorrect() {
    XFXinChenProvider provider;
    QCOMPARE(provider.id(), QString("xfxinchen"));
    QCOMPARE(provider.displayName(), QString("XFXinChen"));
    QCOMPARE(provider.sessionLabel(), QString("5h Usage"));
    QCOMPARE(provider.weeklyLabel(), QString("Weekly"));
    QCOMPARE(provider.opusLabel(), QString("Package"));
    QVERIFY(!provider.supportsCredits());
    QVERIFY(!provider.defaultEnabled());
}

void tst_XFXinChenProvider::sourceModes() {
    XFXinChenProvider provider;
    auto modes = provider.supportedSourceModes();
    QCOMPARE(modes.size(), 1);
    QVERIFY(modes.contains("web"));
}

void tst_XFXinChenProvider::dashboardURL() {
    XFXinChenProvider provider;
    QCOMPARE(provider.dashboardURL(), QString("https://maas.xfyun.cn"));
}

void tst_XFXinChenProvider::brandColor() {
    XFXinChenProvider provider;
    QCOMPARE(provider.brandColor(), QString("#0066FF"));
}

void tst_XFXinChenProvider::manualCookieMakesStrategyAvailable() {
    ProviderFetchContext ctx;
    ctx.manualCookieHeader = QStringLiteral("sessionId=abc123");
    XFXinChenWebStrategy strategy;
    QVERIFY(strategy.isAvailable(ctx));
}

void tst_XFXinChenProvider::envVarMakesStrategyAvailable() {
    ProviderFetchContext ctx;
    ctx.env[QStringLiteral("XFYUN_COOKIE")] = QStringLiteral("sessionId=abc123");
    XFXinChenWebStrategy strategy;
    QVERIFY(strategy.isAvailable(ctx));
}

void tst_XFXinChenProvider::importedBrowserSessionCookieMakesStrategyAvailable() {
    ProviderFetchContext ctx;
    ImportedBrowserSession session;
    session.providerId = QStringLiteral("xfxinchen");
    session.cookieHeader = QStringLiteral("sessionId=abc123");
    ctx.importedBrowserSession = session;
    XFXinChenWebStrategy strategy;
    QVERIFY(strategy.isAvailable(ctx));
}

void tst_XFXinChenProvider::noCookieIsNotAvailable() {
    ProviderFetchContext ctx;
    ctx.disableLegacyCookieImport = true;
    XFXinChenWebStrategy strategy;
    QVERIFY(!strategy.isAvailable(ctx));
}

void tst_XFXinChenProvider::parseResponseFull() {
    QJsonObject usageDTO;
    usageDTO["rp5hUsage"] = 100;
    usageDTO["rp5hLimit"] = 1200;
    usageDTO["rpwUsage"] = 500;
    usageDTO["rpwLimit"] = 9000;
    usageDTO["packageUsage"] = 2000;
    usageDTO["packageLimit"] = 18000;
    usageDTO["packageLeft"] = 16000;

    QJsonObject row;
    row["appId"] = "mc05be2a";
    row["name"] = QStringLiteral("专业版");
    row["status"] = 1;
    row["expiresAt"] = "2026-06-18 10:15:09";
    row["codingPlanUsageDTO"] = usageDTO;

    QJsonArray rows;
    rows.append(row);

    QJsonObject data;
    data["page"] = 1;
    data["rows"] = rows;
    data["total"] = 1;

    QJsonObject json;
    json["code"] = 0;
    json["data"] = data;
    json["succeed"] = true;
    json["failed"] = false;
    json["message"] = "OK";

    auto result = XFXinChenWebStrategy::parseResponse(json);
    QVERIFY(result.success);

    // Primary = 5h: 100/1200 = 8.33%
    QVERIFY(result.usage.primary.has_value());
    QVERIFY(qAbs(result.usage.primary->usedPercent - 100.0 / 1200.0 * 100.0) < 0.01);

    // Secondary = weekly: 500/9000 = 5.56%
    QVERIFY(result.usage.secondary.has_value());
    QVERIFY(qAbs(result.usage.secondary->usedPercent - 500.0 / 9000.0 * 100.0) < 0.01);

    // Tertiary = package: 2000/18000 = 11.11%
    QVERIFY(result.usage.tertiary.has_value());
    QVERIFY(qAbs(result.usage.tertiary->usedPercent - 2000.0 / 18000.0 * 100.0) < 0.01);
    QVERIFY(result.usage.tertiary->resetsAt.has_value());

    // Identity
    QVERIFY(result.usage.identity.has_value());
    QCOMPARE(result.usage.identity->providerID.value(), UsageProvider::xfxinchen);
    QCOMPARE(result.usage.identity->loginMethod, QString("专业版"));
}

void tst_XFXinChenProvider::parseResponseNotSucceed() {
    QJsonObject json;
    json["code"] = 4001;
    json["succeed"] = false;
    json["failed"] = true;
    json["message"] = QStringLiteral("用户未登录");
    json["messageEn"] = "User not logged in";

    auto result = XFXinChenWebStrategy::parseResponse(json);
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.contains("not logged in"));
}

void tst_XFXinChenProvider::parseResponseEmptyRows() {
    QJsonObject data;
    data["rows"] = QJsonArray();
    data["total"] = 0;

    QJsonObject json;
    json["code"] = 0;
    json["data"] = data;
    json["succeed"] = true;

    auto result = XFXinChenWebStrategy::parseResponse(json);
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.contains("No coding plan"));
}

void tst_XFXinChenProvider::parseResponseNoActivePlan() {
    QJsonObject row;
    row["status"] = 0; // inactive
    row["name"] = "Expired Plan";

    QJsonArray rows;
    rows.append(row);

    QJsonObject data;
    data["rows"] = rows;

    QJsonObject json;
    json["code"] = 0;
    json["data"] = data;
    json["succeed"] = true;

    auto result = XFXinChenWebStrategy::parseResponse(json);
    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.contains("No active"));
}

void tst_XFXinChenProvider::parseResponseMissingUsageDTO() {
    QJsonObject row;
    row["status"] = 1;
    row["name"] = "Pro";
    // No codingPlanUsageDTO

    QJsonArray rows;
    rows.append(row);

    QJsonObject data;
    data["rows"] = rows;

    QJsonObject json;
    json["code"] = 0;
    json["data"] = data;
    json["succeed"] = true;

    auto result = XFXinChenWebStrategy::parseResponse(json);
    // Should not crash; no usage data means not successful
    QVERIFY(!result.success);
}

QTEST_MAIN(tst_XFXinChenProvider)
#include "tst_XFXinChenProvider.moc"
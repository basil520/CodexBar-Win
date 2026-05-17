#include <QtTest/QtTest>
#include "../src/providers/qianfan/QianFanProvider.h"
#include "../src/models/UsageSnapshot.h"

class tst_QianFanProvider : public QObject {
    Q_OBJECT

private slots:
    void parsesFullQuotaResponse();
    void apiErrorReturnsFailure();
    void emptyItemsReturnsFailure();
    void partialQuotaOnlyPrimary();
    void zeroLimitSkipsWindow();
    void nonRunningResourceSkipped();
};

void tst_QianFanProvider::parsesFullQuotaResponse()
{
    QJsonObject fiveHour;
    fiveHour["used"] = 147;
    fiveHour["limit"] = 1200;
    fiveHour["resetAt"] = QStringLiteral("2026-05-18T00:10:00+08:00");

    QJsonObject week;
    week["used"] = 8049;
    week["limit"] = 9000;
    week["resetAt"] = QStringLiteral("2026-05-18T00:00:00+08:00");

    QJsonObject month;
    month["used"] = 13051;
    month["limit"] = 18000;
    month["resetAt"] = QStringLiteral("2026-05-23T00:21:06+08:00");

    QJsonObject quota;
    quota["fiveHour"] = fiveHour;
    quota["week"] = week;
    quota["month"] = month;

    QJsonObject item;
    item["resourceId"] = QStringLiteral("cp-2tN5o0S7");
    item["planType"] = QStringLiteral("LITE");
    item["resourceStatus"] = QStringLiteral("Running");
    item["quota"] = quota;

    QJsonArray items;
    items.append(item);

    QJsonObject resultObj;
    resultObj["totalCount"] = 1;
    resultObj["items"] = items;

    QJsonObject json;
    json["success"] = true;
    json["result"] = resultObj;

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(r.success);
    QVERIFY(r.usage.primary.has_value());
    QVERIFY(r.usage.secondary.has_value());
    QVERIFY(r.usage.tertiary.has_value());

    // fiveHour: 147/1200 = 12.25%
    QVERIFY(qAbs(r.usage.primary->usedPercent - 12.25) < 0.01);
    QCOMPARE(r.usage.primary->resetDescription.value(), QStringLiteral("147 / 1,200 Credits"));
    QVERIFY(r.usage.primary->resetsAt.has_value());

    // week: 8049/9000 = 89.433...%
    QVERIFY(qAbs(r.usage.secondary->usedPercent - 89.4333) < 0.01);
    QCOMPARE(r.usage.secondary->resetDescription.value(), QStringLiteral("8,049 / 9,000 Credits"));

    // month: 13051/18000 = 72.505...%
    QVERIFY(qAbs(r.usage.tertiary->usedPercent - 72.5056) < 0.01);
    QCOMPARE(r.usage.tertiary->resetDescription.value(), QStringLiteral("13,051 / 18,000 Credits"));

    QVERIFY(r.usage.identity.has_value());
    QCOMPARE(r.usage.identity->loginMethod.value(), QStringLiteral("LITE"));
}

void tst_QianFanProvider::apiErrorReturnsFailure()
{
    QJsonObject json;
    json["success"] = false;
    json["message"] = QStringLiteral("login required");

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(!r.success);
    QVERIFY(r.errorMessage.contains(QStringLiteral("login required")));
}

void tst_QianFanProvider::emptyItemsReturnsFailure()
{
    QJsonObject resultObj;
    resultObj["totalCount"] = 0;
    resultObj["items"] = QJsonArray();

    QJsonObject json;
    json["success"] = true;
    json["result"] = resultObj;

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(!r.success);
    QVERIFY(r.errorMessage.contains(QStringLiteral("No coding plan")));
}

void tst_QianFanProvider::partialQuotaOnlyPrimary()
{
    QJsonObject fiveHour;
    fiveHour["used"] = 100;
    fiveHour["limit"] = 500;

    QJsonObject quota;
    quota["fiveHour"] = fiveHour;

    QJsonObject item;
    item["planType"] = QStringLiteral("PRO");
    item["resourceStatus"] = QStringLiteral("Running");
    item["quota"] = quota;

    QJsonArray items;
    items.append(item);

    QJsonObject resultObj;
    resultObj["items"] = items;

    QJsonObject json;
    json["success"] = true;
    json["result"] = resultObj;

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(r.success);
    QVERIFY(r.usage.primary.has_value());
    QVERIFY(qAbs(r.usage.primary->usedPercent - 20.0) < 0.01);
    QVERIFY(!r.usage.secondary.has_value());
    QVERIFY(!r.usage.tertiary.has_value());
    QCOMPARE(r.usage.identity->loginMethod.value(), QStringLiteral("PRO"));
}

void tst_QianFanProvider::zeroLimitSkipsWindow()
{
    QJsonObject fiveHour;
    fiveHour["used"] = 50;
    fiveHour["limit"] = 200;

    QJsonObject week;
    week["used"] = 100;
    week["limit"] = 0;

    QJsonObject quota;
    quota["fiveHour"] = fiveHour;
    quota["week"] = week;

    QJsonObject item;
    item["planType"] = QStringLiteral("LITE");
    item["resourceStatus"] = QStringLiteral("Running");
    item["quota"] = quota;

    QJsonArray items;
    items.append(item);

    QJsonObject resultObj;
    resultObj["items"] = items;

    QJsonObject json;
    json["success"] = true;
    json["result"] = resultObj;

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(r.success);
    QVERIFY(r.usage.primary.has_value());
    QVERIFY(!r.usage.secondary.has_value());
}

void tst_QianFanProvider::nonRunningResourceSkipped()
{
    QJsonObject item;
    item["planType"] = QStringLiteral("LITE");
    item["resourceStatus"] = QStringLiteral("Expired");
    item["quota"] = QJsonObject();

    QJsonArray items;
    items.append(item);

    QJsonObject resultObj;
    resultObj["items"] = items;

    QJsonObject json;
    json["success"] = true;
    json["result"] = resultObj;

    const ProviderFetchResult r = QianFanWebStrategy::parseResponse(json);

    QVERIFY(!r.success);
    QVERIFY(r.errorMessage.contains(QStringLiteral("No running")));
}

QTEST_MAIN(tst_QianFanProvider)
#include "tst_QianFanProvider.moc"

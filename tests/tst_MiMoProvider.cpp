#include <QtTest/QtTest>
#include "../src/providers/mimo/MiMoProvider.h"
#include "../src/models/UsageSnapshot.h"

class tst_MiMoProvider : public QObject {
    Q_OBJECT

private slots:
    void parsesWrappedBalanceAndTokenPlanPayloads();
    void balanceOnlyStillCarriesVisibleIdentity();
    void nonZeroApiCodeFailsInsteadOfEmptySuccess();
    void missingBalanceValueFailsInsteadOfZeroBalanceSuccess();
};

void tst_MiMoProvider::parsesWrappedBalanceAndTokenPlanPayloads()
{
    QJsonObject balanceData;
    balanceData["balance"] = QStringLiteral("25.51");
    balanceData["currency"] = QStringLiteral("USD");
    QJsonObject balance;
    balance["code"] = 0;
    balance["message"] = QString();
    balance["data"] = balanceData;

    QJsonObject detailData;
    detailData["planCode"] = QStringLiteral("standard");
    detailData["currentPeriodEnd"] = QStringLiteral("2026-05-04 23:59:59");
    detailData["expired"] = false;
    QJsonObject detail;
    detail["code"] = 0;
    detail["message"] = QString();
    detail["data"] = detailData;

    QJsonObject usageItem;
    usageItem["name"] = QStringLiteral("month_total_token");
    usageItem["used"] = 10100158;
    usageItem["limit"] = 200000000;
    usageItem["percent"] = 0.0505;
    QJsonArray items;
    items.append(usageItem);
    QJsonObject monthUsage;
    monthUsage["percent"] = 0.0505;
    monthUsage["items"] = items;
    QJsonObject usageData;
    usageData["monthUsage"] = monthUsage;
    QJsonObject usage;
    usage["code"] = 0;
    usage["message"] = QString();
    usage["data"] = usageData;

    const ProviderFetchResult result = MiMoWebStrategy::parseResponse(balance, detail, usage);

    QVERIFY(result.success);
    QVERIFY(result.usage.primary.has_value());
    QVERIFY(qAbs(result.usage.primary->usedPercent - 5.05) < 0.0001);
    QVERIFY(result.usage.primary->resetDescription.has_value());
    QCOMPARE(result.usage.primary->resetDescription.value(), QStringLiteral("10,100,158 / 200,000,000 Credits"));
    QVERIFY(result.usage.primary->resetsAt.has_value());
    QVERIFY(result.usage.identity.has_value());
    QVERIFY(result.usage.identity->loginMethod.has_value());
    QCOMPARE(result.usage.identity->loginMethod.value(), QStringLiteral("Standard"));
}

void tst_MiMoProvider::balanceOnlyStillCarriesVisibleIdentity()
{
    QJsonObject balanceData;
    balanceData["balance"] = QStringLiteral("25.51");
    balanceData["currency"] = QStringLiteral("USD");
    QJsonObject balance;
    balance["code"] = 0;
    balance["data"] = balanceData;

    const ProviderFetchResult result = MiMoWebStrategy::parseResponse(balance, {}, {});

    QVERIFY(result.success);
    QVERIFY(!result.usage.primary.has_value());
    QVERIFY(result.usage.identity.has_value());
    QVERIFY(result.usage.identity->loginMethod.has_value());
    QCOMPARE(result.usage.identity->loginMethod.value(), QStringLiteral("Balance: $25.51"));
}

void tst_MiMoProvider::nonZeroApiCodeFailsInsteadOfEmptySuccess()
{
    QJsonObject balance;
    balance["code"] = 401;
    balance["message"] = QStringLiteral("login required");

    const ProviderFetchResult result = MiMoWebStrategy::parseResponse(balance, {}, {});

    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.contains(QStringLiteral("login required")));
}

void tst_MiMoProvider::missingBalanceValueFailsInsteadOfZeroBalanceSuccess()
{
    QJsonObject balanceData;
    balanceData["currency"] = QStringLiteral("USD");
    QJsonObject balance;
    balance["code"] = 0;
    balance["data"] = balanceData;

    const ProviderFetchResult result = MiMoWebStrategy::parseResponse(balance, {}, {});

    QVERIFY(!result.success);
    QVERIFY(result.errorMessage.contains(QStringLiteral("balance"), Qt::CaseInsensitive));
}

QTEST_MAIN(tst_MiMoProvider)
#include "tst_MiMoProvider.moc"

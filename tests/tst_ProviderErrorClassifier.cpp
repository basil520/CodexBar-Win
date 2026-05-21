#include <QtTest/QtTest>

#include "app/ProviderErrorClassifier.h"

class ProviderErrorClassifierTest : public QObject {
    Q_OBJECT

private slots:
    void classifiesCommonProviderErrors_data()
    {
        QTest::addColumn<QString>("message");
        QTest::addColumn<QString>("category");
        QTest::addColumn<QString>("severity");

        QTest::newRow("network timeout")
            << QStringLiteral("Kimi network error: API unreachable or request timed out")
            << QStringLiteral("network")
            << QStringLiteral("error");
        QTest::newRow("auth")
            << QStringLiteral("HTTP 401 Unauthorized")
            << QStringLiteral("auth")
            << QStringLiteral("error");
        QTest::newRow("quota")
            << QStringLiteral("HTTP 429 rate limit exceeded")
            << QStringLiteral("quota")
            << QStringLiteral("warning");
        QTest::newRow("cli")
            << QStringLiteral("claude binary not found")
            << QStringLiteral("cli")
            << QStringLiteral("error");
        QTest::newRow("parse")
            << QStringLiteral("Could not parse Xiaomi MiMo balance: invalid balance value")
            << QStringLiteral("parse")
            << QStringLiteral("warning");
    }

    void classifiesCommonProviderErrors()
    {
        QFETCH(QString, message);
        QFETCH(QString, category);
        QFETCH(QString, severity);

        ProviderErrorClassifier classifier;
        const QVariantMap view = classifier.classify(QStringLiteral("codex"), message);

        QCOMPARE(view.value(QStringLiteral("category")).toString(), category);
        QCOMPARE(view.value(QStringLiteral("severity")).toString(), severity);
        QVERIFY(!view.value(QStringLiteral("title")).toString().isEmpty());
        QVERIFY(!view.value(QStringLiteral("summary")).toString().isEmpty());
        QVERIFY(!view.value(QStringLiteral("copyText")).toString().isEmpty());
    }

    void copyTextRedactsSecrets()
    {
        ProviderErrorClassifier classifier;
        const QString raw = QStringLiteral("Authorization: Bearer sk-secret\ncookie=sessionid=abc123; token=xyz\napi_key: test-key");
        const QString copyText = classifier.copyText(QStringLiteral("codex"), raw);

        QVERIFY(copyText.contains(QStringLiteral("[redacted]")));
        QVERIFY(!copyText.contains(QStringLiteral("sk-secret")));
        QVERIFY(!copyText.contains(QStringLiteral("abc123")));
        QVERIFY(!copyText.contains(QStringLiteral("xyz")));
        QVERIFY(!copyText.contains(QStringLiteral("test-key")));
    }
};

QTEST_MAIN(ProviderErrorClassifierTest)

#include "tst_ProviderErrorClassifier.moc"

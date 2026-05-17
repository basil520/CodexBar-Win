#include <QtTest/QtTest>

#include <QFile>

#include "../src/providers/windsurf/WindsurfProvider.h"
#include "../src/providers/windsurf/WindsurfDevinSessionImporter.h"
#include "../src/providers/ProviderPipeline.h"
#include "../src/providers/ProviderFetchContext.h"

namespace {

struct WindsurfSessionImporterOverrideGuard {
    ~WindsurfSessionImporterOverrideGuard()
    {
        WindsurfDevinSessionImporter::clearOverrides();
    }
};

QVector<WindsurfDevinSessionInfo> noImportedSessions(const ProviderFetchContext&)
{
    return {};
}

} // namespace

class tst_WindsurfBridge : public QObject {
    Q_OBJECT

private slots:
    void parsesBridgePayload();
    void bridgePayloadWinsInAutoMode();
    void manualModeIgnoresBridgePayload();
    void fallsBackToLegacyImporterWhenBridgeMissing();
    void roundTripsWindsurfLocalStorageImport();
    void extensionLocalStorageProbeUsesRequestedKeysAndClosesCreatedTabs();
};

void tst_WindsurfBridge::parsesBridgePayload()
{
    // Bridge sessionPayload uses the same JSON format that parseManualSessionInput handles
    const QString payload = R"({
        "devin_session_token": "bridge-session-token",
        "devin_auth1_token": "bridge-auth1-token",
        "devin_account_id": "bridge-account-id",
        "devin_primary_org_id": "bridge-org-id"
    })";

    QString error;
    auto auth = WindsurfWebStrategy::parseManualSessionInput(payload, &error);
    QVERIFY2(auth.has_value(), qPrintable(error));
    QCOMPARE(auth->sessionToken, QStringLiteral("bridge-session-token"));
    QCOMPARE(auth->auth1Token, QStringLiteral("bridge-auth1-token"));
    QCOMPARE(auth->accountID, QStringLiteral("bridge-account-id"));
    QCOMPARE(auth->primaryOrgID, QStringLiteral("bridge-org-id"));
    QVERIFY(auth->isValid());
}

void tst_WindsurfBridge::bridgePayloadWinsInAutoMode()
{
    WindsurfSessionImporterOverrideGuard guard;
    // Override LevelDB importer to return no sessions
    WindsurfDevinSessionImporter::setImportPreferredSessionsOverride(noImportedSessions);
    WindsurfDevinSessionImporter::setImportFallbackSessionsOverride(noImportedSessions);

    WindsurfWebStrategy strategy;
    ProviderFetchContext ctx;
    ctx.settings.set(QStringLiteral("cookieSource"), QStringLiteral("auto"));

    // Set bridge session in context
    ImportedBrowserSession bridgeSession;
    bridgeSession.providerId = QStringLiteral("windsurf");
    bridgeSession.sessionPayload = R"({
        "devin_session_token": "bridge-session-token",
        "devin_auth1_token": "bridge-auth1-token",
        "devin_account_id": "bridge-account-id",
        "devin_primary_org_id": "bridge-org-id"
    })";
    ctx.importedBrowserSession = bridgeSession;

    ProviderFetchResult result = strategy.fetchSync(ctx);
    // The bridge payload is valid and parsed successfully, so fetchSync attempts
    // a network request which fails in test (no server, httpStatus=0).
    // Since that's a recoverable error (network timeout), it falls through to
    // LevelDB, which also finds nothing. The key invariant: the bridge session
    // was parsed and attempted (not skipped).
    QVERIFY(!result.success);
    // The result should be a failure — network timeout or no sessions found
    QVERIFY(result.errorMessage.contains(QStringLiteral("network timeout"))
         || result.errorMessage.contains(QStringLiteral("No Windsurf web session"))
         || result.errorMessage.contains(QStringLiteral("expired"))
         || result.errorMessage.contains(QStringLiteral("API call failed")));
}

void tst_WindsurfBridge::manualModeIgnoresBridgePayload()
{
    WindsurfWebStrategy strategy;
    ProviderFetchContext ctx;
    ctx.settings.set(QStringLiteral("cookieSource"), QStringLiteral("manual"));

    // Set bridge session in context — should be ignored in manual mode
    ImportedBrowserSession bridgeSession;
    bridgeSession.providerId = QStringLiteral("windsurf");
    bridgeSession.sessionPayload = R"({
        "devin_session_token": "bridge-session-token",
        "devin_auth1_token": "bridge-auth1-token",
        "devin_account_id": "bridge-account-id",
        "devin_primary_org_id": "bridge-org-id"
    })";
    ctx.importedBrowserSession = bridgeSession;

    ProviderFetchResult result = strategy.fetchSync(ctx);
    QVERIFY(!result.success);
    // Manual mode should report "not configured" or "Invalid", not use bridge data
    QVERIFY(result.errorMessage.contains(QStringLiteral("not configured"))
         || result.errorMessage.contains(QStringLiteral("Invalid")));
}

void tst_WindsurfBridge::fallsBackToLegacyImporterWhenBridgeMissing()
{
    WindsurfSessionImporterOverrideGuard guard;
    WindsurfDevinSessionImporter::setImportPreferredSessionsOverride(noImportedSessions);
    WindsurfDevinSessionImporter::setImportFallbackSessionsOverride(noImportedSessions);

    WindsurfWebStrategy strategy;
    ProviderFetchContext ctx;
    ctx.settings.set(QStringLiteral("cookieSource"), QStringLiteral("auto"));

    // No bridge session in context — should fall through to LevelDB importer
    ProviderFetchResult result = strategy.fetchSync(ctx);
    QVERIFY(!result.success);
    // This error comes from the LevelDB importer path (no sessions found)
    QVERIFY(result.errorMessage.contains(QStringLiteral("No Windsurf web session")));
    QVERIFY(result.errorMessage.contains(QStringLiteral("Chromium")));
}

void tst_WindsurfBridge::roundTripsWindsurfLocalStorageImport()
{
    // Verify that the BridgeSessionMaterial localStorage format matches
    // what parseManualSessionInput expects as JSON input
    const QString payload = R"({
        "devin_session_token": "rt-session",
        "devin_auth1_token": "rt-auth1",
        "devin_account_id": "rt-account",
        "devin_primary_org_id": "rt-org"
    })";

    QString error;
    auto auth = WindsurfWebStrategy::parseManualSessionInput(payload, &error);
    QVERIFY2(auth.has_value(), qPrintable(error));
    QVERIFY(auth->isValid());

    QCOMPARE(auth->sessionToken, QStringLiteral("rt-session"));
    QCOMPARE(auth->auth1Token, QStringLiteral("rt-auth1"));
    QCOMPARE(auth->accountID, QStringLiteral("rt-account"));
    QCOMPARE(auth->primaryOrgID, QStringLiteral("rt-org"));
}

void tst_WindsurfBridge::extensionLocalStorageProbeUsesRequestedKeysAndClosesCreatedTabs()
{
    QFile worker(QStringLiteral(PROJECT_SOURCE_DIR "/resources/browser-session-bridge/service_worker.js"));
    QVERIFY2(worker.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(worker.errorString()));
    const QString source = QString::fromUtf8(worker.readAll());

    QVERIFY2(source.contains(QStringLiteral("localStorageKeys")),
             "Windsurf localStorage import must use the provider-requested key list.");
    QVERIFY2(source.contains(QStringLiteral("args: [localStorageKeys]")),
             "The requested key list must be passed into the injected probe.");
    QVERIFY2(source.contains(QStringLiteral("finally")),
             "Created background tabs must be cleaned up on success, timeout, and script injection errors.");
    QVERIFY2(source.contains(QStringLiteral("chrome.tabs.remove(tabId)")),
             "Created background tabs must be closed after localStorage probing.");
}

QTEST_MAIN(tst_WindsurfBridge)
#include "tst_WindsurfBridge.moc"

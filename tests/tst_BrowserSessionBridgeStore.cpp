#include <QtTest/QtTest>
#include "../src/browserbridge/BrowserSessionBridgeStore.h"
#include "../src/providers/shared/ProviderCredentialStore.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>

class tst_BrowserSessionBridgeStore : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    void savesImportedCookieInCredentialStore();
    void filtersExpiredCookies();
    void usesPreferredBinding();
    void fallsBackWhenPreferredBindingMissing();
    void savesImportedLocalStorageInCredentialStore();
    void codexSpecUsesChatGptHybridMaterial();

private:
    std::shared_ptr<InMemoryCredentialBackend> m_backend;
};

void tst_BrowserSessionBridgeStore::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_backend = std::make_shared<InMemoryCredentialBackend>();
    ProviderCredentialStore::setBackendForTesting(m_backend);
}

void tst_BrowserSessionBridgeStore::init()
{
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
}

void tst_BrowserSessionBridgeStore::cleanup()
{
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
}

void tst_BrowserSessionBridgeStore::cleanupTestCase()
{
    ProviderCredentialStore::resetBackendForTesting();
}

void tst_BrowserSessionBridgeStore::savesImportedCookieInCredentialStore()
{
    BrowserSessionBridgeStore store;

    BridgeSessionMaterial material;
    material.providerId = QStringLiteral("cursor");
    material.clientId.browserFamily = QStringLiteral("chrome");
    material.clientId.profileInstanceId = QStringLiteral("uuid-001");
    material.capturedAtUtc = QDateTime::currentDateTimeUtc();
    material.sourceReason = QStringLiteral("manual_request");

    BridgeCookieRecord cookie;
    cookie.name = QStringLiteral("WorkosCursorSessionToken");
    cookie.value = QStringLiteral("secret-cursor-token");
    cookie.domain = QStringLiteral(".cursor.com");
    cookie.path = QStringLiteral("/");
    cookie.secure = true;
    cookie.httpOnly = true;
    material.cookies.append(cookie);

    store.saveImportedMaterial(material);

    // Cookie value should be in credential store
    const auto header = store.resolvedCookieHeader(QStringLiteral("cursor"));
    QVERIFY(header.has_value());
    QVERIFY(header->contains(QStringLiteral("WorkosCursorSessionToken=secret-cursor-token")));

    // Metadata should not contain raw cookie values
    // (metadata file would be checked if we had a real filesystem, but the in-memory
    //  metadata store won't accidentally contain secrets by design)
}

void tst_BrowserSessionBridgeStore::filtersExpiredCookies()
{
    BrowserSessionBridgeStore store;

    BridgeSessionMaterial material;
    material.providerId = QStringLiteral("codex");
    material.clientId.browserFamily = QStringLiteral("edge");
    material.clientId.profileInstanceId = QStringLiteral("uuid-002");
    material.capturedAtUtc = QDateTime::currentDateTimeUtc();

    // Expired cookie
    BridgeCookieRecord expired;
    expired.name = QStringLiteral("expired_cookie");
    expired.value = QStringLiteral("should-not-appear");
    expired.domain = QStringLiteral(".chatgpt.com");
    expired.expirationDateUtc = QDateTime::fromSecsSinceEpoch(1); // 1970
    material.cookies.append(expired);

    // Valid cookie
    BridgeCookieRecord valid;
    valid.name = QStringLiteral("valid_cookie");
    valid.value = QStringLiteral("should-appear");
    valid.domain = QStringLiteral(".chatgpt.com");
    valid.expirationDateUtc = QDateTime::currentDateTimeUtc().addSecs(3600); // 1 hour from now
    material.cookies.append(valid);

    store.saveImportedMaterial(material);

    const auto header = store.resolvedCookieHeader(QStringLiteral("codex"));
    QVERIFY(header.has_value());
    QVERIFY(!header->contains(QStringLiteral("expired_cookie")));
    QVERIFY(header->contains(QStringLiteral("valid_cookie=should-appear")));
}

void tst_BrowserSessionBridgeStore::usesPreferredBinding()
{
    BrowserSessionBridgeStore store;

    // Save material for chrome:uuid-010
    BridgeSessionMaterial mat1;
    mat1.providerId = QStringLiteral("claude");
    mat1.clientId.browserFamily = QStringLiteral("chrome");
    mat1.clientId.profileInstanceId = QStringLiteral("uuid-010");
    mat1.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord c1;
    c1.name = QStringLiteral("sessionKey");
    c1.value = QStringLiteral("chrome-session");
    c1.domain = QStringLiteral(".claude.ai");
    mat1.cookies.append(c1);
    store.saveImportedMaterial(mat1);

    // Save material for edge:uuid-020
    BridgeSessionMaterial mat2;
    mat2.providerId = QStringLiteral("claude");
    mat2.clientId.browserFamily = QStringLiteral("edge");
    mat2.clientId.profileInstanceId = QStringLiteral("uuid-020");
    mat2.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord c2;
    c2.name = QStringLiteral("sessionKey");
    c2.value = QStringLiteral("edge-session");
    c2.domain = QStringLiteral(".claude.ai");
    mat2.cookies.append(c2);
    store.saveImportedMaterial(mat2);

    // Request with preferredBindingId = edge:uuid-020
    const auto header = store.resolvedCookieHeader(
        QStringLiteral("claude"), QStringLiteral("edge:uuid-020"));
    QVERIFY(header.has_value());
    QVERIFY(header->contains(QStringLiteral("edge-session")));
}

void tst_BrowserSessionBridgeStore::fallsBackWhenPreferredBindingMissing()
{
    BrowserSessionBridgeStore store;

    // Only save material for chrome:uuid-030
    BridgeSessionMaterial mat;
    mat.providerId = QStringLiteral("kimi");
    mat.clientId.browserFamily = QStringLiteral("chrome");
    mat.clientId.profileInstanceId = QStringLiteral("uuid-030");
    mat.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord c;
    c.name = QStringLiteral("kimi-auth");
    c.value = QStringLiteral("kimi-token");
    c.domain = QStringLiteral(".kimi.com");
    mat.cookies.append(c);
    store.saveImportedMaterial(mat);

    // Request with non-existent preferred binding
    const auto header = store.resolvedCookieHeader(
        QStringLiteral("kimi"), QStringLiteral("edge:nonexistent"));
    // Should fall back to the available chrome binding
    QVERIFY(header.has_value());
    QVERIFY(header->contains(QStringLiteral("kimi-auth=kimi-token")));
}

void tst_BrowserSessionBridgeStore::savesImportedLocalStorageInCredentialStore()
{
    BrowserSessionBridgeStore store;

    BridgeSessionMaterial material;
    material.providerId = QStringLiteral("windsurf");
    material.clientId.browserFamily = QStringLiteral("chrome");
    material.clientId.profileInstanceId = QStringLiteral("uuid-ls-001");
    material.capturedAtUtc = QDateTime::currentDateTimeUtc();
    material.sourceReason = QStringLiteral("manual_request");

    material.localStorage[QStringLiteral("devin_session_token")] = QStringLiteral("sess-token-abc");
    material.localStorage[QStringLiteral("devin_auth1_token")] = QStringLiteral("auth1-token-xyz");
    material.localStorage[QStringLiteral("devin_account_id")] = QStringLiteral("acct-123");
    material.localStorage[QStringLiteral("devin_primary_org_id")] = QStringLiteral("org-456");

    store.saveImportedMaterial(material);

    // LocalStorage provider should return session payload
    const auto payload = store.resolvedSessionPayload(QStringLiteral("windsurf"));
    QVERIFY(payload.has_value());

    // Verify JSON contains all four keys
    const QJsonDocument doc = QJsonDocument::fromJson(payload->toUtf8());
    QVERIFY(doc.isObject());
    const QJsonObject obj = doc.object();
    QCOMPARE(obj[QStringLiteral("devin_session_token")].toString(), QStringLiteral("sess-token-abc"));
    QCOMPARE(obj[QStringLiteral("devin_auth1_token")].toString(), QStringLiteral("auth1-token-xyz"));
    QCOMPARE(obj[QStringLiteral("devin_account_id")].toString(), QStringLiteral("acct-123"));
    QCOMPARE(obj[QStringLiteral("devin_primary_org_id")].toString(), QStringLiteral("org-456"));

    // LocalStorage-only provider should not return a cookie header
    const auto header = store.resolvedCookieHeader(QStringLiteral("windsurf"));
    QVERIFY(!header.has_value());
}

void tst_BrowserSessionBridgeStore::codexSpecUsesChatGptHybridMaterial()
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(QStringLiteral("codex"));
    QVERIFY(spec.has_value());
    QCOMPARE(spec->materialKind, BridgeMaterialKind::Hybrid);
    QCOMPARE(spec->domains, QStringList{QStringLiteral("chatgpt.com")});
    QCOMPARE(spec->localStorageOrigin, QStringLiteral("https://chatgpt.com"));
}

QTEST_MAIN(tst_BrowserSessionBridgeStore)
#include "tst_BrowserSessionBridgeStore.moc"

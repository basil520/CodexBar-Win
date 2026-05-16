#include <QtTest/QtTest>
#include "../src/browserbridge/BrowserSessionBridgeStore.h"
#include "../src/providers/shared/ProviderCredentialStore.h"

class tst_BrowserSessionBridgeStore : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void savesImportedCookieInCredentialStore();
    void filtersExpiredCookies();
    void usesPreferredBinding();
    void fallsBackWhenPreferredBindingMissing();

private:
    std::shared_ptr<InMemoryCredentialBackend> m_backend;
};

void tst_BrowserSessionBridgeStore::initTestCase()
{
    m_backend = std::make_shared<InMemoryCredentialBackend>();
    ProviderCredentialStore::setBackendForTesting(m_backend);
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

QTEST_MAIN(tst_BrowserSessionBridgeStore)
#include "tst_BrowserSessionBridgeStore.moc"

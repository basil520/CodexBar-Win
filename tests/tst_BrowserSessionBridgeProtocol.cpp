#include <QtTest/QtTest>
#include "../src/browserbridge/BrowserSessionBridgeProtocol.h"

class tst_BrowserSessionBridgeProtocol : public QObject {
    Q_OBJECT

private slots:
    void roundTripsRegisterClient();
    void rejectsUnknownProtocolVersion();
    void roundTripsImportResultWithCookies();
    void roundTripsLocalStoragePayload();
};

void tst_BrowserSessionBridgeProtocol::roundTripsRegisterClient()
{
    RegisterClientPayload original;
    original.protocolVersion = 1;
    original.extensionId = QStringLiteral("ext-abc123");
    original.browserFamily = QStringLiteral("chrome");
    original.browserVersion = QStringLiteral("136.0.0.0");
    original.profileInstanceId = QStringLiteral("8f9c6d2d-1234-5678-abcd-ef0123456789");
    original.profileAlias = QStringLiteral("Work Chrome");
    original.incognito = false;
    original.supportsCookies = true;
    original.supportsLocalStorage = true;

    const auto json = BridgeProtocol::serializeRegisterClient(original);
    QCOMPARE(json[QStringLiteral("type")].toString(), QStringLiteral("register_client"));

    const auto restored = BridgeProtocol::parseRegisterClient(json);
    QCOMPARE(restored.protocolVersion, original.protocolVersion);
    QCOMPARE(restored.extensionId, original.extensionId);
    QCOMPARE(restored.browserFamily, original.browserFamily);
    QCOMPARE(restored.browserVersion, original.browserVersion);
    QCOMPARE(restored.profileInstanceId, original.profileInstanceId);
    QCOMPARE(restored.profileAlias, original.profileAlias);
    QCOMPARE(restored.incognito, original.incognito);
    QCOMPARE(restored.supportsCookies, original.supportsCookies);
    QCOMPARE(restored.supportsLocalStorage, original.supportsLocalStorage);
}

void tst_BrowserSessionBridgeProtocol::rejectsUnknownProtocolVersion()
{
    RegisterClientPayload original;
    original.protocolVersion = 99; // unknown version
    original.extensionId = QStringLiteral("ext-abc123");
    original.browserFamily = QStringLiteral("chrome");
    original.profileInstanceId = QStringLiteral("uuid-123");

    const auto json = BridgeProtocol::serializeRegisterClient(original);
    const auto restored = BridgeProtocol::parseRegisterClient(json);
    QCOMPARE(restored.protocolVersion, 99);

    // Server should respond with error for mismatched version
    RegisterAckPayload ack;
    ack.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    ack.accepted = false;
    ack.errorMessage = QStringLiteral("Unsupported protocol version");

    const auto ackJson = BridgeProtocol::serializeRegisterAck(ack);
    const auto ackRestored = BridgeProtocol::parseRegisterAck(ackJson);
    QVERIFY(!ackRestored.accepted);
    QCOMPARE(ackRestored.errorMessage, QStringLiteral("Unsupported protocol version"));
}

void tst_BrowserSessionBridgeProtocol::roundTripsImportResultWithCookies()
{
    ImportResultPayload original;
    original.requestId = QStringLiteral("req-001");
    original.providerId = QStringLiteral("cursor");
    original.capturedAtUtc = QDateTime::fromString(
        QStringLiteral("2026-05-16T10:20:30Z"), Qt::ISODate);

    BridgeCookieRecord cookie;
    cookie.name = QStringLiteral("WorkosCursorSessionToken");
    cookie.value = QStringLiteral("secret-value-123");
    cookie.domain = QStringLiteral(".cursor.com");
    cookie.path = QStringLiteral("/");
    cookie.sameSite = QStringLiteral("lax");
    cookie.storeId = QStringLiteral("0");
    cookie.secure = true;
    cookie.httpOnly = true;
    cookie.hostOnly = false;
    cookie.session = false;
    cookie.expirationDateUtc = QDateTime::fromSecsSinceEpoch(1780000000);
    original.cookies.append(cookie);

    const auto json = BridgeProtocol::serializeImportResult(original);
    QCOMPARE(json[QStringLiteral("type")].toString(), QStringLiteral("import_result"));

    const auto restored = BridgeProtocol::parseImportResult(json);
    QCOMPARE(restored.requestId, original.requestId);
    QCOMPARE(restored.providerId, original.providerId);
    QCOMPARE(restored.cookies.size(), 1);
    QCOMPARE(restored.cookies[0].name, cookie.name);
    QCOMPARE(restored.cookies[0].value, cookie.value);
    QCOMPARE(restored.cookies[0].domain, cookie.domain);
    QCOMPARE(restored.cookies[0].secure, cookie.secure);
    QCOMPARE(restored.cookies[0].httpOnly, cookie.httpOnly);
    QVERIFY(restored.cookies[0].expirationDateUtc.has_value());
    QCOMPARE(restored.cookies[0].expirationDateUtc->toSecsSinceEpoch(),
             cookie.expirationDateUtc->toSecsSinceEpoch());
}

void tst_BrowserSessionBridgeProtocol::roundTripsLocalStoragePayload()
{
    ImportResultPayload original;
    original.requestId = QStringLiteral("req-002");
    original.providerId = QStringLiteral("windsurf");
    original.capturedAtUtc = QDateTime::currentDateTimeUtc();
    original.localStorage[QStringLiteral("devin_session_token")] = QStringLiteral("tok-abc");
    original.localStorage[QStringLiteral("devin_auth1_token")] = QStringLiteral("auth-xyz");
    original.localStorage[QStringLiteral("devin_account_id")] = QStringLiteral("acc-123");

    const auto json = BridgeProtocol::serializeImportResult(original);
    const auto restored = BridgeProtocol::parseImportResult(json);

    QCOMPARE(restored.providerId, QStringLiteral("windsurf"));
    QCOMPARE(restored.localStorage.size(), 3);
    QCOMPARE(restored.localStorage[QStringLiteral("devin_session_token")], QStringLiteral("tok-abc"));
    QCOMPARE(restored.localStorage[QStringLiteral("devin_auth1_token")], QStringLiteral("auth-xyz"));
    QCOMPARE(restored.localStorage[QStringLiteral("devin_account_id")], QStringLiteral("acc-123"));
}

QTEST_MAIN(tst_BrowserSessionBridgeProtocol)
#include "tst_BrowserSessionBridgeProtocol.moc"

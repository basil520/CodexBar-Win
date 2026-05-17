#include <QtTest/QtTest>
#include "../src/browserbridge/BrowserSessionBridgeProtocol.h"

class tst_BrowserSessionBridgeProtocol : public QObject {
    Q_OBJECT

private slots:
    void roundTripsRegisterClient();
    void parsesExtensionRegisterClientSample();
    void parsesExtensionSessionCookieWithoutExpiry();
    void parsesExtensionCookieWithFractionalExpiry();
    void serializesRequestImportWithCanonicalMaterialKind();
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
    original.supportsCodexUsageSnapshot = true;
    original.supportsCookieUrlQuery = true;
    original.supportsAllUrlsCookiePermission = true;
    original.extensionBuild = QStringLiteral("2026.05.17");

    const auto json = BridgeProtocol::serializeRegisterClient(original);
    QCOMPARE(json[QStringLiteral("type")].toString(), QStringLiteral("register_client"));
    QVERIFY(json[QStringLiteral("capabilities")].toObject()[QStringLiteral("cookieUrlQuery")].toBool());
    QVERIFY(json[QStringLiteral("capabilities")].toObject()[QStringLiteral("allUrlsCookiePermission")].toBool());

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
    QCOMPARE(restored.supportsCodexUsageSnapshot, original.supportsCodexUsageSnapshot);
    QCOMPARE(restored.supportsCookieUrlQuery, original.supportsCookieUrlQuery);
    QCOMPARE(restored.supportsAllUrlsCookiePermission, original.supportsAllUrlsCookiePermission);
    QCOMPARE(restored.extensionBuild, original.extensionBuild);
}

void tst_BrowserSessionBridgeProtocol::parsesExtensionRegisterClientSample()
{
    const QByteArray sample = R"json({
        "type": "register_client",
        "protocolVersion": 1,
        "extensionId": "cnanalhpjiclhljkpnlbgiaclpbncidk",
        "browserFamily": "chrome",
        "browserVersion": "127.0.0.0",
        "profileInstanceId": "profile-uuid-001",
        "profileAlias": "Default",
        "incognito": false,
        "extensionBuild": "2026.05.17",
        "capabilities": {
            "cookies": true,
            "localStorage": true,
            "codexUsageSnapshot": true,
            "cookieUrlQuery": true,
            "allUrlsCookiePermission": true
        }
    })json";

    const auto msg = BridgeProtocol::parseMessage(sample);
    QVERIFY(msg.has_value());
    QCOMPARE(msg->type, BridgeMessageType::RegisterClient);

    const auto payload = BridgeProtocol::parseRegisterClient(msg->payload);
    QCOMPARE(payload.extensionId, QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk"));
    QCOMPARE(payload.browserFamily, QStringLiteral("chrome"));
    QCOMPARE(payload.profileInstanceId, QStringLiteral("profile-uuid-001"));
    QVERIFY(payload.supportsCookies);
    QVERIFY(payload.supportsLocalStorage);
    QVERIFY(payload.supportsCodexUsageSnapshot);
    QVERIFY(payload.supportsCookieUrlQuery);
    QVERIFY(payload.supportsAllUrlsCookiePermission);
    QCOMPARE(payload.extensionBuild, QStringLiteral("2026.05.17"));
}

void tst_BrowserSessionBridgeProtocol::parsesExtensionSessionCookieWithoutExpiry()
{
    const QByteArray sample = R"json({
        "type": "import_result",
        "requestId": "req-session-cookie",
        "providerId": "codex",
        "success": true,
        "cookies": [
            {
                "name": "__Secure-next-auth.session-token",
                "value": "session-cookie-value",
                "domain": ".chatgpt.com",
                "path": "/",
                "secure": true,
                "httpOnly": true,
                "session": true,
                "expirationDate": null
            }
        ],
        "localStorage": {},
        "capturedAtUtc": "2026-05-16T10:20:30Z"
    })json";

    const auto msg = BridgeProtocol::parseMessage(sample);
    QVERIFY(msg.has_value());
    QCOMPARE(msg->type, BridgeMessageType::ImportResult);

    const auto payload = BridgeProtocol::parseImportResult(msg->payload);
    QCOMPARE(payload.providerId, QStringLiteral("codex"));
    QCOMPARE(payload.cookies.size(), 1);
    QCOMPARE(payload.cookies[0].name, QStringLiteral("__Secure-next-auth.session-token"));
    QVERIFY(payload.cookies[0].session);
    QVERIFY2(!payload.cookies[0].expirationDateUtc.has_value(),
             "Session cookies with expirationDate:null must not be treated as expired 1970 cookies.");
}

void tst_BrowserSessionBridgeProtocol::parsesExtensionCookieWithFractionalExpiry()
{
    const QByteArray sample = R"json({
        "type": "import_result",
        "requestId": "req-fractional-expiry",
        "providerId": "opencodego",
        "success": true,
        "cookies": [
            {
                "name": "auth",
                "value": "opencode-auth-cookie",
                "domain": "opencode.ai",
                "path": "/",
                "secure": true,
                "httpOnly": true,
                "session": false,
                "expirationDate": 1800000000.123
            }
        ],
        "localStorage": {},
        "capturedAtUtc": "2026-05-17T07:20:30Z"
    })json";

    const auto msg = BridgeProtocol::parseMessage(sample);
    QVERIFY(msg.has_value());
    const auto payload = BridgeProtocol::parseImportResult(msg->payload);

    QCOMPARE(payload.cookies.size(), 1);
    QVERIFY(payload.cookies[0].expirationDateUtc.has_value());
    QCOMPARE(payload.cookies[0].expirationDateUtc->toSecsSinceEpoch(), qint64(1800000000));
}

void tst_BrowserSessionBridgeProtocol::serializesRequestImportWithCanonicalMaterialKind()
{
    RequestImportPayload cookies;
    cookies.requestId = QStringLiteral("req-cookies");
    cookies.providerId = QStringLiteral("cursor");
    cookies.materialKind = BridgeMaterialKind::Cookies;
    const auto cookiesJson = BridgeProtocol::serializeRequestImport(cookies);
    QCOMPARE(cookiesJson[QStringLiteral("type")].toString(), QStringLiteral("request_import"));
    QCOMPARE(cookiesJson[QStringLiteral("materialKind")].toString(), QStringLiteral("cookies"));

    RequestImportPayload storage;
    storage.requestId = QStringLiteral("req-storage");
    storage.providerId = QStringLiteral("windsurf");
    storage.materialKind = BridgeMaterialKind::LocalStorage;
    const auto storageJson = BridgeProtocol::serializeRequestImport(storage);
    QCOMPARE(storageJson[QStringLiteral("materialKind")].toString(), QStringLiteral("localStorage"));
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

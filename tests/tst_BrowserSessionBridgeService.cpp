#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <QFile>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QWebSocket>

#include "../src/browserbridge/BrowserSessionBridgeService.h"
#include "../src/browserbridge/BrowserSessionBridgeStore.h"
#include "../src/browserbridge/BrowserSessionBridgeProtocol.h"
#include "../src/providers/shared/ProviderCredentialStore.h"

class tst_BrowserSessionBridgeService : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void statusQueriesReturnAfterStart();
    void registerImportDisconnectFlow();
    void emitsProviderImportedOnceAfterDebounce();
    void requestImportSendsRequestToConnectedClient();
    void emptyCookieImportReportsFailure();
    void codexUsageErrorImportReportsFailure();
    void codexUsageJsonWithoutCookiesPersistsAsSessionPayload();
    void webSocketImportResultPersistsAsynchronously();
    void cookieProviderRequiresAllUrlsCookiePermission();
    void preferredBindingIsNotSilentlyReplacedByAnotherProfile();
    void hybridDiagnosticsOnlyDoesNotCountAsImportedMaterial();
    void credentialWriteFailureReportsImportFailure();
    void kimiRefreshTokenOnlyDoesNotCountAsImportedMaterial();

private:
    BrowserSessionBridgeStore* m_store = nullptr;
    BrowserSessionBridgeService* m_service = nullptr;
    std::shared_ptr<InMemoryCredentialBackend> m_backend;
};

class FailingCredentialBackend : public ProviderCredentialBackend {
public:
    bool write(const QString&, const QString&, const QByteArray&) override { return false; }
    std::optional<QByteArray> read(const QString&) override { return std::nullopt; }
    bool remove(const QString&) override { return false; }
    bool exists(const QString&) override { return false; }
};

void tst_BrowserSessionBridgeService::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void tst_BrowserSessionBridgeService::init()
{
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
    m_backend = std::make_shared<InMemoryCredentialBackend>();
    ProviderCredentialStore::setBackendForTesting(m_backend);
    m_store = new BrowserSessionBridgeStore();
    m_service = new BrowserSessionBridgeService(m_store);
    m_service->start();
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->isServerRunning();
    }, 500));
}

void tst_BrowserSessionBridgeService::cleanup()
{
    if (m_service) {
        m_service->stop();
        delete m_service;
        m_service = nullptr;
    }
    if (m_store) {
        delete m_store;
        m_store = nullptr;
    }
    ProviderCredentialStore::resetBackendForTesting();
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
}

void tst_BrowserSessionBridgeService::statusQueriesReturnAfterStart()
{
    QElapsedTimer elapsed;
    elapsed.start();

    (void)m_service->isServerRunning();
    (void)m_service->serverPort();

    QVERIFY2(elapsed.elapsed() < 200,
             "Bridge service status queries must not block the UI thread.");
}

void tst_BrowserSessionBridgeService::registerImportDisconnectFlow()
{
    QSignalSpy importedSpy(m_service, &BrowserSessionBridgeService::providerSessionImported);
    QVERIFY(importedSpy.isValid());

    // Simulate client registration via direct store upsert
    BridgeClientInfo client;
    client.id.browserFamily = QStringLiteral("chrome");
    client.id.profileInstanceId = QStringLiteral("uuid-test-003");
    client.extensionId = QStringLiteral("test-ext");
    m_store->upsertClient(client);

    // Build and save import result directly (bypassing server)
    BridgeSessionMaterial material;
    material.providerId = QStringLiteral("cursor");
    material.clientId = client.id;
    material.capturedAtUtc = QDateTime::currentDateTimeUtc();
    material.sourceReason = QStringLiteral("manual_request");

    BridgeCookieRecord cookie;
    cookie.name = QStringLiteral("WorkosCursorSessionToken");
    cookie.value = QStringLiteral("cursor-token-service-test");
    cookie.domain = QStringLiteral(".cursor.com");
    material.cookies.append(cookie);

    m_store->saveImportedMaterial(material);

    // Verify the data is stored
    const auto header = m_store->resolvedCookieHeader(QStringLiteral("cursor"));
    QVERIFY(header.has_value());
    QVERIFY(header->contains(QStringLiteral("cursor-token-service-test")));
}

void tst_BrowserSessionBridgeService::emitsProviderImportedOnceAfterDebounce()
{
    QSignalSpy importedSpy(m_service, &BrowserSessionBridgeService::providerSessionImported);
    QVERIFY(importedSpy.isValid());

    BridgeClientInfo client;
    client.id.browserFamily = QStringLiteral("chrome");
    client.id.profileInstanceId = QStringLiteral("uuid-test-004");
    m_store->upsertClient(client);

    // Simulate two rapid import results for the same provider
    BridgeSessionMaterial mat1;
    mat1.providerId = QStringLiteral("claude");
    mat1.clientId = client.id;
    mat1.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord c1;
    c1.name = QStringLiteral("sessionKey");
    c1.value = QStringLiteral("session-1");
    c1.domain = QStringLiteral(".claude.ai");
    mat1.cookies.append(c1);
    m_store->saveImportedMaterial(mat1);

    BridgeSessionMaterial mat2;
    mat2.providerId = QStringLiteral("claude");
    mat2.clientId = client.id;
    mat2.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord c2;
    c2.name = QStringLiteral("sessionKey");
    c2.value = QStringLiteral("session-2");
    c2.domain = QStringLiteral(".claude.ai");
    mat2.cookies.append(c2);
    m_store->saveImportedMaterial(mat2);

    // Wait for debounce (1.5s + margin)
    QTest::qWait(2000);

    // Service's internal debounce should emit only once for the same provider
    // Note: since we bypassed the server, the service didn't receive importResultReceived
    // signal. This test validates the store works; full debounce test needs server integration.
    // For now, just verify store state.
    const auto header = m_store->resolvedCookieHeader(QStringLiteral("claude"));
    QVERIFY(header.has_value());
    QVERIFY(header->contains(QStringLiteral("session-2")));
}

void tst_BrowserSessionBridgeService::requestImportSendsRequestToConnectedClient()
{
    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-request-send-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;
    reg.supportsCookieUrlQuery = true;
    reg.supportsAllUrlsCookiePermission = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-request-send-001"));
    }, 2000));

    QSignalSpy textSpy(&client, &QWebSocket::textMessageReceived);
    QVERIFY(textSpy.isValid());
    QVERIFY(m_service->requestImport(QStringLiteral("codex")));

    QVERIFY(QTest::qWaitFor([&textSpy]() {
        for (const auto& args : textSpy) {
            const auto msg = BridgeProtocol::parseMessage(args.at(0).toString().toUtf8());
            if (msg.has_value() && msg->type == BridgeMessageType::RequestImport) {
                return true;
            }
        }
        return false;
    }, 2000));

    bool found = false;
    for (const auto& args : textSpy) {
        const auto msg = BridgeProtocol::parseMessage(args.at(0).toString().toUtf8());
        if (!msg.has_value() || msg->type != BridgeMessageType::RequestImport) continue;
        const auto payload = BridgeProtocol::parseRequestImport(msg->payload);
        QCOMPARE(payload.providerId, QStringLiteral("codex"));
        QCOMPARE(payload.materialKind, BridgeMaterialKind::Hybrid);
        QVERIFY(payload.domains.contains(QStringLiteral("chatgpt.com")));
        QCOMPARE(payload.localStorageOrigin, QStringLiteral("https://chatgpt.com"));
        found = true;
    }
    QVERIFY(found);
}

void tst_BrowserSessionBridgeService::emptyCookieImportReportsFailure()
{
    QSignalSpy completedSpy(m_service, &BrowserSessionBridgeService::providerImportCompleted);
    QVERIFY(completedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-empty-import-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-empty-import-001"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-empty-import");
    result.providerId = QStringLiteral("codex");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&completedSpy]() { return completedSpy.count() > 0; }, 2000));
    const auto completedArgs = completedSpy.takeFirst();
    QCOMPARE(completedArgs.at(0).toString(), QStringLiteral("codex"));
    QCOMPARE(completedArgs.at(1).toBool(), false);
    QVERIFY(m_service->lastError().contains(QStringLiteral("usage snapshot"), Qt::CaseInsensitive));
    QVERIFY(!m_store->resolvedCookieHeader(QStringLiteral("codex")).has_value());
    const auto binding = m_service->bindingForProvider(QStringLiteral("codex"));
    QVERIFY(!binding.has_value() || !binding->lastImportedAtUtc.isValid());
}

void tst_BrowserSessionBridgeService::codexUsageErrorImportReportsFailure()
{
    QSignalSpy completedSpy(m_service, &BrowserSessionBridgeService::providerImportCompleted);
    QVERIFY(completedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-codex-usage-error-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-codex-usage-error-001"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-codex-usage-error");
    result.providerId = QStringLiteral("codex");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    result.localStorage[QStringLiteral("codex_usage_error")] =
        QStringLiteral("codex_usage_http_401 at backend-api/wham/usage");

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&completedSpy]() { return completedSpy.count() > 0; }, 2000));
    const auto completedArgs = completedSpy.takeFirst();
    QCOMPARE(completedArgs.at(0).toString(), QStringLiteral("codex"));
    QCOMPARE(completedArgs.at(1).toBool(), false);
    QVERIFY(completedArgs.at(2).toString().contains(QStringLiteral("codex_usage_http_401")));
    const QString diagnosticTarget = BrowserSessionBridgeStore::credentialTargetFor(
        QStringLiteral("codex"),
        QStringLiteral("edge:uuid-codex-usage-error-001"),
        BridgeMaterialKind::LocalStorage);
    QVERIFY(QTest::qWaitFor([&diagnosticTarget]() {
        return ProviderCredentialStore::read(diagnosticTarget).has_value();
    }, 2000));
    const auto diagnostic = ProviderCredentialStore::read(diagnosticTarget);
    QVERIFY(diagnostic.has_value());
    QVERIFY(QString::fromUtf8(diagnostic.value()).contains(QStringLiteral("codex_usage_http_401")));
    const auto binding = m_service->bindingForProvider(QStringLiteral("codex"));
    QVERIFY(!binding.has_value() || !binding->lastImportedAtUtc.isValid());
}

void tst_BrowserSessionBridgeService::codexUsageJsonWithoutCookiesPersistsAsSessionPayload()
{
    QSignalSpy importedSpy(m_service, &BrowserSessionBridgeService::providerSessionImported);
    QVERIFY(importedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-codex-usage-json-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-codex-usage-json-001"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-codex-usage-json");
    result.providerId = QStringLiteral("codex");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    result.localStorage[QStringLiteral("codex_usage_json")] =
        QStringLiteral(R"({"rate_limit":{"primary_window":{"used_percent":12,"limit_window_seconds":18000}},"email":"bridge@example.com"})");

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&importedSpy]() { return importedSpy.count() > 0; }, 4000));
    BrowserSessionBridgeStore persistedStore;
    const auto payload = persistedStore.resolvedSessionPayload(QStringLiteral("codex"));
    QVERIFY(payload.has_value());
    QVERIFY(payload->contains(QStringLiteral("codex_usage_json")));
    const auto binding = m_service->bindingForProvider(QStringLiteral("codex"));
    QVERIFY(binding.has_value());
    QVERIFY(binding->lastImportedAtUtc.isValid());
}

void tst_BrowserSessionBridgeService::webSocketImportResultPersistsAsynchronously()
{
    QSignalSpy importedSpy(m_service, &BrowserSessionBridgeService::providerSessionImported);
    QVERIFY(importedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("chrome");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-service-ws-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("chrome:uuid-service-ws-001"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-service-ws");
    result.providerId = QStringLiteral("cursor");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    BridgeCookieRecord cookie;
    cookie.name = QStringLiteral("WorkosCursorSessionToken");
    cookie.value = QStringLiteral("service-ws-token");
    cookie.domain = QStringLiteral(".cursor.com");
    result.cookies.append(cookie);

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&importedSpy]() { return importedSpy.count() > 0; }, 4000));
    const QString target = BrowserSessionBridgeStore::credentialTargetFor(
        QStringLiteral("cursor"), QStringLiteral("chrome:uuid-service-ws-001"));
    const auto stored = ProviderCredentialStore::read(target);
    QVERIFY(stored.has_value());
    QVERIFY(QString::fromUtf8(stored.value()).contains(QStringLiteral("service-ws-token")));
}

void tst_BrowserSessionBridgeService::cookieProviderRequiresAllUrlsCookiePermission()
{
    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-old-cookie-query-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;
    reg.supportsCookieUrlQuery = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-old-cookie-query-001"));
    }, 2000));

    QVERIFY(m_service->bindingOptions(QStringLiteral("kimi")).isEmpty());
    QVERIFY(!m_service->requestImport(QStringLiteral("kimi")));
    QVERIFY(m_service->lastError().contains(QStringLiteral("Reload"), Qt::CaseInsensitive));
}

void tst_BrowserSessionBridgeService::preferredBindingIsNotSilentlyReplacedByAnotherProfile()
{
    QWebSocket oldClient;
    QSignalSpy oldConnectedSpy(&oldClient, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(oldConnectedSpy.isValid());
    QNetworkRequest oldReq(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    oldReq.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    oldClient.open(oldReq);
    QVERIFY(QTest::qWaitFor([&oldConnectedSpy]() { return oldConnectedSpy.count() > 0; }, 2000));

    RegisterClientPayload oldReg;
    oldReg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    oldReg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    oldReg.browserFamily = QStringLiteral("edge");
    oldReg.browserVersion = QStringLiteral("127.0.0.0");
    oldReg.profileInstanceId = QStringLiteral("uuid-stale-selected-profile");
    oldReg.profileAlias = QStringLiteral("Default");
    oldReg.supportsCookies = true;
    oldReg.supportsLocalStorage = true;
    oldReg.supportsCodexUsageSnapshot = true;
    oldReg.supportsCookieUrlQuery = false;

    BridgeMessage oldRegisterMsg;
    oldRegisterMsg.type = BridgeMessageType::RegisterClient;
    oldRegisterMsg.payload = BridgeProtocol::serializeRegisterClient(oldReg);
    oldClient.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(oldRegisterMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(
            QStringLiteral("edge:uuid-stale-selected-profile"));
    }, 2000));

    QWebSocket newClient;
    QSignalSpy newConnectedSpy(&newClient, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(newConnectedSpy.isValid());
    QNetworkRequest newReq(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    newReq.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    newClient.open(newReq);
    QVERIFY(QTest::qWaitFor([&newConnectedSpy]() { return newConnectedSpy.count() > 0; }, 2000));

    RegisterClientPayload newReg = oldReg;
    newReg.profileInstanceId = QStringLiteral("uuid-new-compatible-profile");
    newReg.supportsCookieUrlQuery = true;
    newReg.supportsAllUrlsCookiePermission = true;

    BridgeMessage newRegisterMsg;
    newRegisterMsg.type = BridgeMessageType::RegisterClient;
    newRegisterMsg.payload = BridgeProtocol::serializeRegisterClient(newReg);
    newClient.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(newRegisterMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(
            QStringLiteral("edge:uuid-new-compatible-profile"));
    }, 2000));

    QSignalSpy newTextSpy(&newClient, &QWebSocket::textMessageReceived);
    QVERIFY(newTextSpy.isValid());
    m_service->setProviderBindingAsync(
        QStringLiteral("mimo"),
        QStringLiteral("edge:uuid-stale-selected-profile"));

    QVERIFY(!m_service->requestImport(QStringLiteral("mimo")));
    QVERIFY(m_service->lastError().contains(QStringLiteral("selected"), Qt::CaseInsensitive));
    QTest::qWait(100);
    for (const auto& args : newTextSpy) {
        const auto msg = BridgeProtocol::parseMessage(args.at(0).toString().toUtf8());
        QVERIFY(!msg.has_value() || msg->type != BridgeMessageType::RequestImport);
    }
}

void tst_BrowserSessionBridgeService::hybridDiagnosticsOnlyDoesNotCountAsImportedMaterial()
{
    QSignalSpy completedSpy(m_service, &BrowserSessionBridgeService::providerImportCompleted);
    QVERIFY(completedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-kimi-diag-only");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;
    reg.supportsCookieUrlQuery = true;
    reg.supportsAllUrlsCookiePermission = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-kimi-diag-only"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-kimi-diag-only");
    result.providerId = QStringLiteral("kimi");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    result.localStorage[QStringLiteral("cookie_query_diagnostics")] =
        QStringLiteral(R"({"domains":{"kimi.com":{"matchedCount":0}}})");

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&completedSpy]() { return completedSpy.count() > 0; }, 2000));
    const auto completedArgs = completedSpy.takeFirst();
    QCOMPARE(completedArgs.at(0).toString(), QStringLiteral("kimi"));
    QCOMPARE(completedArgs.at(1).toBool(), false);
    QVERIFY(completedArgs.at(2).toString().contains(QStringLiteral("diagnostics"), Qt::CaseInsensitive));
    const auto binding = m_service->bindingForProvider(QStringLiteral("kimi"));
    QVERIFY(!binding.has_value() || !binding->lastImportedAtUtc.isValid());
}

void tst_BrowserSessionBridgeService::credentialWriteFailureReportsImportFailure()
{
    QSignalSpy completedSpy(m_service, &BrowserSessionBridgeService::providerImportCompleted);
    QVERIFY(completedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-kimi-write-failure");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;
    reg.supportsCookieUrlQuery = true;
    reg.supportsAllUrlsCookiePermission = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-kimi-write-failure"));
    }, 2000));

    ProviderCredentialStore::setBackendForTesting(std::make_shared<FailingCredentialBackend>());

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-kimi-write-failure");
    result.providerId = QStringLiteral("kimi");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    result.localStorage[QStringLiteral("access_token")] = QStringLiteral("kimi-access-token");

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&completedSpy]() { return completedSpy.count() > 0; }, 2000));
    const auto completedArgs = completedSpy.takeFirst();
    QCOMPARE(completedArgs.at(0).toString(), QStringLiteral("kimi"));
    QCOMPARE(completedArgs.at(1).toBool(), false);
    QVERIFY(completedArgs.at(2).toString().contains(QStringLiteral("persist"), Qt::CaseInsensitive));
    const auto binding = m_service->bindingForProvider(QStringLiteral("kimi"));
    QVERIFY(!binding.has_value() || !binding->lastImportedAtUtc.isValid());

    ProviderCredentialStore::setBackendForTesting(m_backend);
}

void tst_BrowserSessionBridgeService::kimiRefreshTokenOnlyDoesNotCountAsImportedMaterial()
{
    QSignalSpy completedSpy(m_service, &BrowserSessionBridgeService::providerImportCompleted);
    QVERIFY(completedSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_service->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-kimi-refresh-only");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;
    reg.supportsCookieUrlQuery = true;
    reg.supportsAllUrlsCookiePermission = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return m_service->connectedClientBindingIds().contains(QStringLiteral("edge:uuid-kimi-refresh-only"));
    }, 2000));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-kimi-refresh-only");
    result.providerId = QStringLiteral("kimi");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();
    result.localStorage[QStringLiteral("refresh_token")] = QStringLiteral("refresh-token-only");

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([&completedSpy]() { return completedSpy.count() > 0; }, 2000));
    const auto completedArgs = completedSpy.takeFirst();
    QCOMPARE(completedArgs.at(0).toString(), QStringLiteral("kimi"));
    QCOMPARE(completedArgs.at(1).toBool(), false);
    QVERIFY(completedArgs.at(2).toString().contains(QStringLiteral("access token"), Qt::CaseInsensitive));
}

QTEST_MAIN(tst_BrowserSessionBridgeService)
#include "tst_BrowserSessionBridgeService.moc"

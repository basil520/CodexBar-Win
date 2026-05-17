#include <QtTest/QtTest>
#include <QFile>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QSignalSpy>
#include <QWebSocket>

#include "app/BridgeViewModel.h"
#include "browserbridge/BrowserSessionBridgeStore.h"
#include "browserbridge/BrowserSessionBridgeService.h"

class tst_BridgeViewModel : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void storeAndServiceOnly();
    void viewModelOnly();
    void fullTestSuite();
    void importFailureClearsBusyAndShowsError();
    void codexOldExtensionCapabilityDoesNotAppearImportable();
    void cookieOldExtensionCapabilityDoesNotAppearImportable();

private:
    void recreate();
    std::unique_ptr<BrowserSessionBridgeStore> m_store;
    std::unique_ptr<BrowserSessionBridgeService> m_service;
    std::unique_ptr<BridgeViewModel> m_viewModel;
};

void tst_BridgeViewModel::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void tst_BridgeViewModel::init()
{
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
}

void tst_BridgeViewModel::cleanup()
{
    m_viewModel.reset();
    m_service.reset();
    m_store.reset();
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath());
    QFile::remove(BrowserSessionBridgeMetadataStore::metadataFilePath() + QStringLiteral(".tmp"));
}

void tst_BridgeViewModel::storeAndServiceOnly()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    QVERIFY(m_service->connectedClientBindingIds().isEmpty());
    m_service.reset();
    m_store.reset();
}

void tst_BridgeViewModel::viewModelOnly()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    m_viewModel = std::make_unique<BridgeViewModel>(m_service.get(), m_store.get());
    QVERIFY(!m_viewModel->serverRunning());
    m_viewModel.reset();
    m_service.reset();
    m_store.reset();
}

void tst_BridgeViewModel::fullTestSuite()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    m_viewModel = std::make_unique<BridgeViewModel>(m_service.get(), m_store.get());

    QVERIFY(m_viewModel->isProviderSupported(QStringLiteral("windsurf")));
    QVERIFY(!m_viewModel->isProviderSupported(QStringLiteral("nonexistent")));

    const auto providers = m_viewModel->supportedProviders();
    QVERIFY(!providers.isEmpty());

    QVERIFY(!m_viewModel->serverRunning());
    QCOMPARE(m_viewModel->serverPort(), 0);
    QCOMPARE(m_viewModel->connectedClients().size(), 0);

    const auto map = m_viewModel->bindingForProvider(QStringLiteral("windsurf"));
    QVERIFY(map.isEmpty());

    QVERIFY(m_viewModel->autoSync(QStringLiteral("windsurf")));
    QVERIFY(m_viewModel->lastImportTime(QStringLiteral("windsurf")).isEmpty());
    QVERIFY(!m_viewModel->installGuideSeen());

    m_viewModel->setInstallGuideSeen(true);
    QVERIFY(m_viewModel->installGuideSeen());

    m_viewModel->setAutoSync(QStringLiteral("windsurf"), false);
    QVERIFY(!m_viewModel->autoSync(QStringLiteral("windsurf")));

    m_viewModel->setBindingForProvider(QStringLiteral("windsurf"), QStringLiteral("chrome-default"));
    const auto map2 = m_viewModel->bindingForProvider(QStringLiteral("windsurf"));
    QVERIFY(!map2.isEmpty());
    QCOMPARE(map2[QStringLiteral("preferredBindingId")].toString(), QStringLiteral("chrome-default"));

    const auto bindings = m_viewModel->availableBindings(QStringLiteral("windsurf"));
    QCOMPARE(bindings.size(), 0);

    QVERIFY(!m_viewModel->extensionInstallPath().isEmpty());
}

void tst_BridgeViewModel::importFailureClearsBusyAndShowsError()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    m_viewModel = std::make_unique<BridgeViewModel>(m_service.get(), m_store.get());
    m_service->start();
    QVERIFY(QTest::qWaitFor([this]() { return m_viewModel->serverRunning(); }, 1000));

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_viewModel->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-vm-import-fail-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return !m_viewModel->bindingOptions(QStringLiteral("codex")).isEmpty();
    }, 2000));

    QSignalSpy busySpy(m_viewModel.get(), &BridgeViewModel::importBusyChanged);
    QVERIFY(busySpy.isValid());

    m_viewModel->requestImport(QStringLiteral("codex"));
    QVERIFY(m_viewModel->importBusy(QStringLiteral("codex")));

    ImportResultPayload result;
    result.requestId = QStringLiteral("req-vm-fail");
    result.providerId = QStringLiteral("codex");
    result.success = false;
    result.errorCode = QStringLiteral("cookie_fetch_failed");
    result.errorMessage = QStringLiteral("Unable to read cookies from this profile.");
    result.capturedAtUtc = QDateTime::currentDateTimeUtc();

    BridgeMessage importMsg;
    importMsg.type = BridgeMessageType::ImportResult;
    importMsg.payload = BridgeProtocol::serializeImportResult(result);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(importMsg)));

    QVERIFY(QTest::qWaitFor([this]() {
        return !m_viewModel->importBusy(QStringLiteral("codex"));
    }, 2000));
    QVERIFY(m_viewModel->importError(QStringLiteral("codex")).contains(QStringLiteral("Unable to read cookies")));
}

void tst_BridgeViewModel::codexOldExtensionCapabilityDoesNotAppearImportable()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    m_viewModel = std::make_unique<BridgeViewModel>(m_service.get(), m_store.get());
    m_service->start();
    QVERIFY(QTest::qWaitFor([this]() { return m_viewModel->serverRunning(); }, 1000));

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_viewModel->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-vm-old-ext-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = false;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return !m_viewModel->connectedClients().isEmpty();
    }, 2000));

    QVERIFY(m_viewModel->bindingOptions(QStringLiteral("codex")).isEmpty());
    m_viewModel->requestImport(QStringLiteral("codex"));
    QVERIFY(!m_viewModel->importBusy(QStringLiteral("codex")));
    QVERIFY(m_viewModel->importError(QStringLiteral("codex")).contains(QStringLiteral("Reload"), Qt::CaseInsensitive));
}

void tst_BridgeViewModel::cookieOldExtensionCapabilityDoesNotAppearImportable()
{
    m_store = std::make_unique<BrowserSessionBridgeStore>();
    m_service = std::make_unique<BrowserSessionBridgeService>(m_store.get());
    m_viewModel = std::make_unique<BridgeViewModel>(m_service.get(), m_store.get());
    m_service->start();
    QVERIFY(QTest::qWaitFor([this]() { return m_viewModel->serverRunning(); }, 1000));

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_viewModel->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
    reg.browserFamily = QStringLiteral("edge");
    reg.browserVersion = QStringLiteral("127.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-vm-old-cookie-query-001");
    reg.profileAlias = QStringLiteral("Default");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = true;
    reg.supportsCodexUsageSnapshot = true;

    BridgeMessage registerMsg;
    registerMsg.type = BridgeMessageType::RegisterClient;
    registerMsg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(registerMsg)));
    QVERIFY(QTest::qWaitFor([this]() {
        return !m_viewModel->connectedClients().isEmpty();
    }, 2000));

    QVERIFY(m_viewModel->bindingOptions(QStringLiteral("kimi")).isEmpty());
    m_viewModel->requestImport(QStringLiteral("kimi"));
    QVERIFY(!m_viewModel->importBusy(QStringLiteral("kimi")));
    QVERIFY(m_viewModel->importError(QStringLiteral("kimi")).contains(QStringLiteral("Reload"), Qt::CaseInsensitive));
}

QTEST_MAIN(tst_BridgeViewModel)
#include "tst_BridgeViewModel.moc"

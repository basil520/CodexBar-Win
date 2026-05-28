#include <QtTest/QtTest>
#include <QWebSocket>
#include <QSignalSpy>
#include <QThread>
#include <QNetworkRequest>

#include "../src/browserbridge/BrowserSessionBridgeServer.h"
#include "../src/browserbridge/BrowserSessionBridgeProtocol.h"

class tst_BrowserSessionBridgeServer : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void acceptsAllowedExtensionOrigin();
    void rejectsUnknownExtensionOrigin();
    void rejectsUnknownOrigin();
    void keepsSocketWorkOffUiThread();

private:
    BrowserSessionBridgeServer* m_server = nullptr;
};

void tst_BrowserSessionBridgeServer::initTestCase()
{
#if defined(Q_OS_MACOS)
    QSKIP("Browser Session Bridge live WebSocket server is disabled on macOS; native cookie import is the primary path.");
#endif
}

void tst_BrowserSessionBridgeServer::init()
{
    m_server = new BrowserSessionBridgeServer();
    m_server->start();
    QVERIFY(QTest::qWaitFor([this]() { return m_server->isRunning(); }, 2000));
}

void tst_BrowserSessionBridgeServer::cleanup()
{
    if (m_server) {
        m_server->stop();
        delete m_server;
        m_server = nullptr;
    }
}

void tst_BrowserSessionBridgeServer::acceptsAllowedExtensionOrigin()
{
    QSignalSpy registeredSpy(m_server, &BrowserSessionBridgeServer::clientRegistered);
    QVERIFY(registeredSpy.isValid());

    QWebSocket client;
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(connectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("test-extension-id");
    reg.browserFamily = QStringLiteral("chrome");
    reg.browserVersion = QStringLiteral("136.0.0.0");
    reg.profileInstanceId = QStringLiteral("uuid-test-001");
    reg.profileAlias = QStringLiteral("Test Chrome");
    reg.supportsCookies = true;
    reg.supportsLocalStorage = false;

    BridgeMessage msg;
    msg.type = BridgeMessageType::RegisterClient;
    msg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(msg)));

    QVERIFY(QTest::qWaitFor([&registeredSpy]() { return registeredSpy.count() > 0; }, 2000));
    QCOMPARE(registeredSpy.count(), 1);
}

void tst_BrowserSessionBridgeServer::rejectsUnknownExtensionOrigin()
{
    QWebSocket client;
    QSignalSpy disconnectedSpy(&client, &QWebSocket::disconnected);
    QVERIFY(disconnectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&disconnectedSpy]() { return disconnectedSpy.count() > 0; }, 2000));
}

void tst_BrowserSessionBridgeServer::rejectsUnknownOrigin()
{
    QSignalSpy errorSpy(m_server, &BrowserSessionBridgeServer::errorOccurred);
    QVERIFY(errorSpy.isValid());

    QWebSocket client;
    QSignalSpy disconnectedSpy(&client, &QWebSocket::disconnected);
    QVERIFY(disconnectedSpy.isValid());

    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->serverPort())));
    req.setRawHeader("Origin", "127.0.0.1");
    client.open(req);
    QVERIFY(QTest::qWaitFor([&disconnectedSpy]() { return disconnectedSpy.count() > 0; }, 2000));
}

void tst_BrowserSessionBridgeServer::keepsSocketWorkOffUiThread()
{
    Qt::HANDLE uiThreadId = QThread::currentThreadId();
    Qt::HANDLE serverThreadId = m_server->serverThreadId();

    QWebSocket client;
    QNetworkRequest req(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->serverPort())));
    req.setRawHeader("Origin", "chrome-extension://cnanalhpjiclhljkpnlbgiaclpbncidk");
    client.open(req);
    QSignalSpy connectedSpy(&client, QOverload<>::of(&QWebSocket::connected));
    QVERIFY(QTest::qWaitFor([&connectedSpy]() { return connectedSpy.count() > 0; }, 2000));

    RegisterClientPayload reg;
    reg.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    reg.extensionId = QStringLiteral("test-extension-id");
    reg.browserFamily = QStringLiteral("chrome");
    reg.profileInstanceId = QStringLiteral("uuid-test-002");

    BridgeMessage msg;
    msg.type = BridgeMessageType::RegisterClient;
    msg.payload = BridgeProtocol::serializeRegisterClient(reg);
    client.sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(msg)));

    QSignalSpy registeredSpy(m_server, &BrowserSessionBridgeServer::clientRegistered);
    QVERIFY(QTest::qWaitFor([&registeredSpy]() { return registeredSpy.count() > 0; }, 2000));

    QVERIFY(serverThreadId != nullptr);
#if defined(Q_OS_MACOS)
    QCOMPARE(serverThreadId, uiThreadId);
#else
    QVERIFY(serverThreadId != uiThreadId);
#endif
}

QTEST_MAIN(tst_BrowserSessionBridgeServer)
#include "tst_BrowserSessionBridgeServer.moc"

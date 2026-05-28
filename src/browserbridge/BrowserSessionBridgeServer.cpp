#include "BrowserSessionBridgeServer.h"

#include "BrowserSessionBridgeConstants.h"

#include <QWebSocketServer>
#include <QWebSocket>
#include <QTimer>
#include <QHostAddress>
#include <QMetaType>

namespace {

bool useDedicatedServerThread()
{
#if defined(Q_OS_MACOS)
    return false;
#else
    return true;
#endif
}

} // namespace

BrowserSessionBridgeServer::BrowserSessionBridgeServer(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<BridgeClientId>("BridgeClientId");
    qRegisterMetaType<BridgeClientInfo>("BridgeClientInfo");
    qRegisterMetaType<ImportResultPayload>("ImportResultPayload");
    qRegisterMetaType<SessionDirtyPayload>("SessionDirtyPayload");

    m_allowedOrigins = BrowserSessionBridgeConstants::allowedOrigins();
    if (useDedicatedServerThread()) {
        moveToThread(&m_thread);
        m_thread.start();
    }
}

BrowserSessionBridgeServer::~BrowserSessionBridgeServer()
{
    stop();
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait(3000);
    }
}

void BrowserSessionBridgeServer::start()
{
    QMetaObject::invokeMethod(this, &BrowserSessionBridgeServer::doStart, Qt::QueuedConnection);
}

void BrowserSessionBridgeServer::stop()
{
    if (QThread::currentThread() == thread()) {
        doStop();
    } else if (thread() && thread()->isRunning()) {
        QMetaObject::invokeMethod(this, &BrowserSessionBridgeServer::doStop, Qt::BlockingQueuedConnection);
    } else {
        doStop();
    }
}

bool BrowserSessionBridgeServer::isRunning() const
{
    if (QThread::currentThread() == thread()) {
        return m_server && m_server->isListening();
    }

    bool result = false;
    QMetaObject::invokeMethod(
        const_cast<BrowserSessionBridgeServer*>(this),
        [this, &result]() { result = m_server && m_server->isListening(); },
        Qt::BlockingQueuedConnection);
    return result;
}

bool BrowserSessionBridgeServer::heartbeatActive() const
{
    if (QThread::currentThread() == thread()) {
        return m_heartbeatTimer && m_heartbeatTimer->isActive();
    }

    bool result = false;
    QMetaObject::invokeMethod(
        const_cast<BrowserSessionBridgeServer*>(this),
        [this, &result]() { result = m_heartbeatTimer && m_heartbeatTimer->isActive(); },
        Qt::BlockingQueuedConnection);
    return result;
}

quint16 BrowserSessionBridgeServer::serverPort() const
{
    if (QThread::currentThread() == thread()) {
        return m_server ? m_server->serverPort() : 0;
    }

    quint16 result = 0;
    QMetaObject::invokeMethod(
        const_cast<BrowserSessionBridgeServer*>(this),
        [this, &result]() { result = m_server ? m_server->serverPort() : 0; },
        Qt::BlockingQueuedConnection);
    return result;
}

void BrowserSessionBridgeServer::setAllowedOriginsForTesting(const QStringList& origins)
{
    QMetaObject::invokeMethod(this, [this, origins]() {
        m_allowedOrigins = origins;
    }, Qt::QueuedConnection);
}

Qt::HANDLE BrowserSessionBridgeServer::serverThreadId() const
{
    if (QThread::currentThread() == thread()) {
        return QThread::currentThreadId();
    }

    Qt::HANDLE id = nullptr;
    QMetaObject::invokeMethod(
        const_cast<BrowserSessionBridgeServer*>(this),
        [&id]() { id = QThread::currentThreadId(); },
        Qt::BlockingQueuedConnection);
    return id;
}

void BrowserSessionBridgeServer::sendRegisterAck(const RegisterAckPayload& payload,
                                                  const QString& bindingId)
{
    QMetaObject::invokeMethod(this, [this, payload, bindingId]() {
        auto it = m_clients.find(bindingId);
        if (it == m_clients.end()) return;
        BridgeMessage msg;
        msg.type = BridgeMessageType::RegisterAck;
        msg.payload = BridgeProtocol::serializeRegisterAck(payload);
        sendMessage(it.value(), msg);
    }, Qt::QueuedConnection);
}

void BrowserSessionBridgeServer::sendRequestImport(const RequestImportPayload& payload,
                                                    const QString& bindingId)
{
    QMetaObject::invokeMethod(this, [this, payload, bindingId]() {
        auto it = m_clients.find(bindingId);
        if (it == m_clients.end()) return;
        BridgeMessage msg;
        msg.type = BridgeMessageType::RequestImport;
        msg.payload = BridgeProtocol::serializeRequestImport(payload);
        sendMessage(it.value(), msg);
    }, Qt::QueuedConnection);
}

void BrowserSessionBridgeServer::sendPing(const QString& bindingId)
{
    QMetaObject::invokeMethod(this, [this, bindingId]() {
        auto it = m_clients.find(bindingId);
        if (it == m_clients.end()) return;
        BridgeMessage msg;
        msg.type = BridgeMessageType::Ping;
        msg.payload = QJsonObject();
        sendMessage(it.value(), msg);
    }, Qt::QueuedConnection);
}

void BrowserSessionBridgeServer::sendError(const BridgeErrorPayload& payload,
                                            const QString& bindingId)
{
    QMetaObject::invokeMethod(this, [this, payload, bindingId]() {
        auto it = m_clients.find(bindingId);
        if (it == m_clients.end()) return;
        BridgeMessage msg;
        msg.type = BridgeMessageType::Error;
        msg.payload = BridgeProtocol::serializeError(payload);
        sendMessage(it.value(), msg);
    }, Qt::QueuedConnection);
}

void BrowserSessionBridgeServer::doStart()
{
    if (m_server) return;

    m_server = new QWebSocketServer(
        QStringLiteral("CodexBarX Browser Session Bridge"),
        QWebSocketServer::NonSecureMode,
        this);

    connect(m_server, &QWebSocketServer::newConnection,
            this, &BrowserSessionBridgeServer::onNewConnection);

    quint16 port = 0;
    for (quint16 p = 18765; p <= 18770; ++p) {
        if (m_server->listen(QHostAddress::LocalHost, p)) {
            port = p;
            break;
        }
    }

    if (port == 0) {
        emit errorOccurred(QStringLiteral("Failed to bind to any port in range 18765-18770"));
        m_server->deleteLater();
        m_server = nullptr;
        emit serverStateChanged(false, 0);
        return;
    }

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(15000); // 15s
    connect(m_heartbeatTimer, &QTimer::timeout, this, &BrowserSessionBridgeServer::onHeartbeatTimer);
    emit serverStateChanged(true, port);
}

void BrowserSessionBridgeServer::doStop()
{
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        m_heartbeatTimer->deleteLater();
        m_heartbeatTimer = nullptr;
    }

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        it.value()->close(QWebSocketProtocol::CloseCodeNormal);
    }
    m_clients.clear();
    m_socketToClientId.clear();
    m_lastPongTimes.clear();

    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    emit serverStateChanged(false, 0);
}

void BrowserSessionBridgeServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QWebSocket* socket = m_server->nextPendingConnection();
        if (!validateOrigin(socket->origin())) {
            socket->close(QWebSocketProtocol::CloseCodePolicyViolated);
            socket->deleteLater();
            continue;
        }

        connect(socket, &QWebSocket::textMessageReceived,
                this, &BrowserSessionBridgeServer::onTextMessageReceived);
        connect(socket, &QWebSocket::disconnected,
                this, &BrowserSessionBridgeServer::onSocketDisconnected);
        m_lastPongTimes[socket] = QDateTime::currentDateTimeUtc();
    }
}

void BrowserSessionBridgeServer::onTextMessageReceived(const QString& message)
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;

    auto parsed = BridgeProtocol::parseMessage(message.toUtf8());
    if (!parsed.has_value()) {
        BridgeErrorPayload err;
        err.code = QStringLiteral("parse_error");
        err.message = QStringLiteral("Failed to parse message");
        sendError(err, m_socketToClientId.value(socket).toBindingId());
        return;
    }

    const auto& msg = parsed.value();
    switch (msg.type) {
    case BridgeMessageType::RegisterClient:
        handleRegisterClient(socket, BridgeProtocol::parseRegisterClient(msg.payload));
        break;
    case BridgeMessageType::ImportResult:
        handleImportResult(socket, BridgeProtocol::parseImportResult(msg.payload));
        break;
    case BridgeMessageType::SessionDirty:
        handleSessionDirty(socket, BridgeProtocol::parseSessionDirty(msg.payload));
        break;
    case BridgeMessageType::Ping:
        handlePing(socket);
        break;
    case BridgeMessageType::Pong:
        m_lastPongTimes[socket] = QDateTime::currentDateTimeUtc();
        break;
    default:
        BridgeErrorPayload err;
        err.code = QStringLiteral("unexpected_message");
        err.message = QStringLiteral("Unexpected message type for this direction");
        sendError(err, m_socketToClientId.value(socket).toBindingId());
        break;
    }
}

void BrowserSessionBridgeServer::onSocketDisconnected()
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;

    BridgeClientId clientId = m_socketToClientId.value(socket);
    QString bindingId = clientId.toBindingId();

    m_clients.remove(bindingId);
    m_socketToClientId.remove(socket);
    m_lastPongTimes.remove(socket);
    socket->deleteLater();

    if (!bindingId.isEmpty()) {
        emit clientDisconnected(clientId);
    }
    syncHeartbeatTimer();
}

void BrowserSessionBridgeServer::onHeartbeatTimer()
{
    const auto now = QDateTime::currentDateTimeUtc();
    QList<QWebSocket*> toClose;

    for (auto it = m_lastPongTimes.begin(); it != m_lastPongTimes.end(); ++it) {
        if (it.key() && it.value().secsTo(now) > 30) {
            toClose.append(it.key());
        }
    }

    for (auto* socket : toClose) {
        closeClient(socket, QStringLiteral("Heartbeat timeout"));
    }

    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        BridgeMessage msg;
        msg.type = BridgeMessageType::Ping;
        msg.payload = QJsonObject();
        sendMessage(it.value(), msg);
    }
}

void BrowserSessionBridgeServer::handleRegisterClient(QWebSocket* socket,
                                                       const RegisterClientPayload& payload)
{
    if (payload.protocolVersion != BRIDGE_PROTOCOL_VERSION) {
        BridgeErrorPayload err;
        err.code = QStringLiteral("version_mismatch");
        err.message = QStringLiteral("Protocol version mismatch");
        sendError(err, QString());
        socket->close(QWebSocketProtocol::CloseCodePolicyViolated);
        return;
    }
    if (payload.browserFamily.trimmed().isEmpty() || payload.profileInstanceId.trimmed().isEmpty()) {
        emit errorOccurred(QStringLiteral("Rejected Browser Session Bridge client with missing browser/profile identity."));
        socket->close(QWebSocketProtocol::CloseCodePolicyViolated);
        return;
    }

    BridgeClientInfo info;
    info.id.browserFamily = payload.browserFamily;
    info.id.profileInstanceId = payload.profileInstanceId;
    info.id.incognito = payload.incognito;
    info.extensionId = payload.extensionId;
    info.extensionBuild = payload.extensionBuild;
    info.browserVersion = payload.browserVersion;
    info.profileAlias = payload.profileAlias;
    info.supportsCookies = payload.supportsCookies;
    info.supportsLocalStorage = payload.supportsLocalStorage;
    info.supportsCodexUsageSnapshot = payload.supportsCodexUsageSnapshot;
    info.supportsCookieUrlQuery = payload.supportsCookieUrlQuery;
    info.supportsAllUrlsCookiePermission = payload.supportsAllUrlsCookiePermission;
    info.connectedAt = QDateTime::currentDateTimeUtc();
    info.lastSeenAt = info.connectedAt;

    QString bindingId = info.id.toBindingId();

    if (m_clients.contains(bindingId)) {
        auto* oldSocket = m_clients.value(bindingId);
        m_socketToClientId.remove(oldSocket);
        m_lastPongTimes.remove(oldSocket);
        oldSocket->close(QWebSocketProtocol::CloseCodeNormal);
        oldSocket->deleteLater();
    }

    m_clients[bindingId] = socket;
    m_socketToClientId[socket] = info.id;
    m_lastPongTimes[socket] = QDateTime::currentDateTimeUtc();
    syncHeartbeatTimer();

    emit clientRegistered(info);
}

void BrowserSessionBridgeServer::handleImportResult(QWebSocket* socket,
                                                     const ImportResultPayload& payload)
{
    BridgeClientId clientId = m_socketToClientId.value(socket);
    emit importResultReceived(payload, clientId);
}

void BrowserSessionBridgeServer::handleSessionDirty(QWebSocket* socket,
                                                     const SessionDirtyPayload& payload)
{
    Q_UNUSED(socket)
    emit sessionDirtyReceived(payload);
}

void BrowserSessionBridgeServer::handlePing(QWebSocket* socket)
{
    BridgeMessage msg;
    msg.type = BridgeMessageType::Pong;
    msg.payload = QJsonObject();
    sendMessage(socket, msg);
    m_lastPongTimes[socket] = QDateTime::currentDateTimeUtc();
}

void BrowserSessionBridgeServer::sendMessage(QWebSocket* socket, const BridgeMessage& msg)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;
    socket->sendTextMessage(QString::fromUtf8(BridgeProtocol::serializeMessage(msg)));
}

void BrowserSessionBridgeServer::closeClient(QWebSocket* socket, const QString& reason)
{
    Q_UNUSED(reason)
    if (!socket) return;
    socket->close(QWebSocketProtocol::CloseCodeNormal);
}

bool BrowserSessionBridgeServer::validateOrigin(const QString& origin) const
{
    if (origin.isEmpty()) return false;
    for (const auto& allowed : m_allowedOrigins) {
        if (origin == allowed || origin == allowed + QLatin1Char('/')) return true;
    }
    return false;
}

void BrowserSessionBridgeServer::syncHeartbeatTimer()
{
    if (!m_heartbeatTimer) {
        return;
    }
    if (m_clients.isEmpty()) {
        m_heartbeatTimer->stop();
    } else if (!m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start();
    }
}

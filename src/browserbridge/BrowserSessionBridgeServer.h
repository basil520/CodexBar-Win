#pragma once

#include "BrowserSessionBridgeTypes.h"
#include "BrowserSessionBridgeProtocol.h"

#include <QObject>
#include <QThread>
#include <QHash>
#include <QDateTime>

class QWebSocketServer;
class QWebSocket;
class QTimer;

class BrowserSessionBridgeServer : public QObject {
    Q_OBJECT
public:
    explicit BrowserSessionBridgeServer(QObject* parent = nullptr);
    ~BrowserSessionBridgeServer() override;

    void start();
    void stop();
    bool isRunning() const;
    quint16 serverPort() const;

    void setAllowedOriginsForTesting(const QStringList& origins);

    Qt::HANDLE serverThreadId() const;

signals:
    void serverStateChanged(bool running, quint16 port);
    void clientRegistered(const BridgeClientInfo& client);
    void clientDisconnected(const BridgeClientId& clientId);
    void importResultReceived(const ImportResultPayload& payload, const BridgeClientId& clientId);
    void sessionDirtyReceived(const SessionDirtyPayload& payload);
    void errorOccurred(const QString& error);

public slots:
    void sendRegisterAck(const RegisterAckPayload& payload, const QString& bindingId);
    void sendRequestImport(const RequestImportPayload& payload, const QString& bindingId);
    void sendPing(const QString& bindingId);
    void sendError(const BridgeErrorPayload& payload, const QString& bindingId);

private slots:
    void doStart();
    void doStop();
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onSocketDisconnected();
    void onHeartbeatTimer();

private:
    void handleRegisterClient(QWebSocket* socket, const RegisterClientPayload& payload);
    void handleImportResult(QWebSocket* socket, const ImportResultPayload& payload);
    void handleSessionDirty(QWebSocket* socket, const SessionDirtyPayload& payload);
    void handlePing(QWebSocket* socket);
    void sendMessage(QWebSocket* socket, const BridgeMessage& msg);
    void closeClient(QWebSocket* socket, const QString& reason);
    bool validateOrigin(const QString& origin) const;

    QThread m_thread;
    QWebSocketServer* m_server = nullptr;
    QHash<QString, QWebSocket*> m_clients;
    QHash<QWebSocket*, BridgeClientId> m_socketToClientId;
    QHash<QWebSocket*, QDateTime> m_lastPongTimes;
    QTimer* m_heartbeatTimer = nullptr;
    QStringList m_allowedOrigins;
};

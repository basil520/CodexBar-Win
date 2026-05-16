#pragma once

#include "BrowserSessionBridgeStore.h"
#include "BrowserSessionBridgeServer.h"
#include "BrowserSessionBridgeProtocol.h"

#include <QObject>
#include <QSet>
#include <QTimer>

class BrowserSessionBridgeService : public QObject {
    Q_OBJECT
public:
    explicit BrowserSessionBridgeService(BrowserSessionBridgeStore* store, QObject* parent = nullptr);
    ~BrowserSessionBridgeService() override;

    void start();
    void stop();

    BrowserSessionBridgeStore* store() const;

    void requestImport(const QString& providerId, const QString& preferredBindingId = QString());

    bool isClientConnected(const QString& bindingId) const;
    QStringList connectedClientBindingIds() const;

signals:
    void providerSessionImported(const QString& providerId);
    void clientConnectionStateChanged();

private slots:
    void onClientRegistered(const BridgeClientInfo& client);
    void onClientDisconnected(const BridgeClientId& clientId);
    void onImportResultReceived(const ImportResultPayload& payload, const BridgeClientId& clientId);
    void onSessionDirtyReceived(const SessionDirtyPayload& payload);
    void onServerError(const QString& error);
    void onDebounceRefresh();

private:
    void enqueueDebounceRefresh(const QString& providerId);
    QString resolveTargetBindingId(const QString& providerId, const QString& preferredBindingId) const;

    BrowserSessionBridgeStore* m_store;
    BrowserSessionBridgeServer* m_server = nullptr;
    QTimer m_debounceTimer;
    QSet<QString> m_pendingRefreshProviders;
    QSet<QString> m_connectedBindingIds;
};

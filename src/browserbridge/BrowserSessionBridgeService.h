#pragma once

#include "BrowserSessionBridgeStore.h"
#include "BrowserSessionBridgeServer.h"
#include "BrowserSessionBridgeProtocol.h"

#include <QObject>
#include <QSet>
#include <QHash>
#include <QThread>
#include <QTimer>
#include <functional>
#include <optional>

class BrowserSessionBridgeService : public QObject {
    Q_OBJECT
public:
    explicit BrowserSessionBridgeService(BrowserSessionBridgeStore* store, QObject* parent = nullptr);
    ~BrowserSessionBridgeService() override;

    void start();
    void pause();
    void stop();

    BrowserSessionBridgeStore* store() const;

    bool requestImport(const QString& providerId, const QString& preferredBindingId = QString());
    void prepareExtension();
    void setInstallGuideSeenAsync(bool seen);
    void setProviderBindingAsync(const QString& providerId, const QString& bindingId);
    void setAutoSyncAsync(const QString& providerId, bool enabled);

    bool isClientConnected(const QString& bindingId) const;
    QStringList connectedClientBindingIds() const;
    QVector<BridgeBindingOption> bindingOptions(const QString& providerId) const;
    std::optional<BridgeProviderBinding> bindingForProvider(const QString& providerId) const;
    bool autoSyncForProvider(const QString& providerId) const;
    bool installGuideSeen() const;
    bool extensionExported() const;
    bool extensionPreparing() const;
    QString lastError() const;
    std::optional<BridgeSessionLookupInput> sessionLookupForProvider(const QString& providerId) const;

    bool isServerRunning() const;
    quint16 serverPort() const;

signals:
    void providerSessionImported(const QString& providerId);
    void providerImportCompleted(const QString& providerId, bool success, const QString& message);
    void clientConnectionStateChanged();
    void serverStateChanged();
    void providerBindingChanged(const QString& providerId);
    void installGuideSeenChanged();
    void extensionStateChanged();
    void lastErrorChanged();

private slots:
    void onServerStateChanged(bool running, quint16 port);
    void onClientRegistered(const BridgeClientInfo& client);
    void onClientDisconnected(const BridgeClientId& clientId);
    void onImportResultReceived(const ImportResultPayload& payload, const BridgeClientId& clientId);
    void onSessionDirtyReceived(const SessionDirtyPayload& payload);
    void onServerError(const QString& error);
    void onDebounceRefresh();

private:
    void postIoTask(const std::function<void()>& task) const;
    void persistClientAsync(const BridgeClientInfo& client);
    void persistProviderBindingAsync(const QString& providerId, const BridgeProviderBinding& binding);
    void persistCodexImportFailureAsync(const ImportResultPayload& payload,
                                        const BridgeClientId& clientId,
                                        const QString& message);
    void persistInstallGuideSeenAsync(bool seen);
    void refreshExtensionExportStateAsync();
    void enqueueDebounceRefresh(const QString& providerId);
    QString resolveTargetBindingId(const QString& providerId, const QString& preferredBindingId) const;
    bool clientSupportsProvider(const BridgeClientInfo& client, const BridgeProviderSpec& spec) const;
    void setLastError(const QString& message);

    BrowserSessionBridgeStore* m_store;
    BrowserSessionBridgeServer* m_server = nullptr;
    QObject* m_ioWorker = nullptr;
    QThread m_ioThread;
    QTimer m_debounceTimer;
    QSet<QString> m_pendingRefreshProviders;
    QSet<QString> m_connectedBindingIds;
    QHash<QString, BridgeClientInfo> m_knownClients;
    QHash<QString, BridgeProviderBinding> m_providerBindings;
    bool m_installGuideSeen = false;
    bool m_extensionExported = false;
    bool m_extensionPreparing = false;
    QString m_lastError;
    bool m_serverRunning = false;
    quint16 m_serverPort = 0;
};

#pragma once

#include "browserbridge/BrowserSessionBridgeService.h"
#include "browserbridge/BrowserSessionBridgeStore.h"
#include "browserbridge/BrowserSessionBridgeCatalog.h"

#include <QObject>
#include <QHash>
#include <QSet>
#include <QVariantList>
#include <QVariantMap>

class BridgeViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool serverRunning READ serverRunning NOTIFY serverRunningChanged)
    Q_PROPERTY(int serverPort READ serverPort NOTIFY serverRunningChanged)
    Q_PROPERTY(QVariantList connectedClients READ connectedClients NOTIFY connectedClientsChanged)
    Q_PROPERTY(QString extensionInstallPath READ extensionInstallPath CONSTANT)
    Q_PROPERTY(bool extensionInstalled READ extensionInstalled NOTIFY extensionInstalledChanged)
    Q_PROPERTY(bool extensionPreparing READ extensionPreparing NOTIFY extensionInstalledChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool installGuideSeen READ installGuideSeen WRITE setInstallGuideSeen NOTIFY installGuideSeenChanged)

public:
    explicit BridgeViewModel(BrowserSessionBridgeService* service,
                             BrowserSessionBridgeStore* store,
                             QObject* parent = nullptr);

    bool serverRunning() const;
    int serverPort() const;
    QVariantList connectedClients() const;
    QString extensionInstallPath() const;
    bool extensionInstalled() const;
    bool extensionPreparing() const;
    QString lastError() const;
    bool installGuideSeen() const;
    void setInstallGuideSeen(bool seen);

    Q_INVOKABLE bool isProviderSupported(const QString& providerId) const;
    Q_INVOKABLE QStringList supportedProviders() const;
    Q_INVOKABLE QVariantMap bindingForProvider(const QString& providerId) const;
    Q_INVOKABLE QVariantList bindingOptions(const QString& providerId) const;
    Q_INVOKABLE QStringList availableBindings(const QString& providerId) const;
    Q_INVOKABLE bool autoSync(const QString& providerId) const;
    Q_INVOKABLE QString lastImportTime(const QString& providerId) const;
    Q_INVOKABLE bool importBusy(const QString& providerId) const;
    Q_INVOKABLE QString importError(const QString& providerId) const;

    Q_INVOKABLE void prepareExtension();
    Q_INVOKABLE void requestImport(const QString& providerId);
    Q_INVOKABLE void setBindingForProvider(const QString& providerId, const QString& bindingId);
    Q_INVOKABLE void setAutoSync(const QString& providerId, bool enabled);
    Q_INVOKABLE void openExtensionFolder() const;
    Q_INVOKABLE void copyExtensionPath() const;
    Q_INVOKABLE void openChromeExtensionsPage() const;
    Q_INVOKABLE void openEdgeExtensionsPage() const;

signals:
    void serverRunningChanged();
    void connectedClientsChanged();
    void extensionInstalledChanged();
    void installGuideSeenChanged();
    void providerBindingChanged(const QString& providerId);
    void lastErrorChanged();
    void importBusyChanged(const QString& providerId);
    void importFeedbackChanged(const QString& providerId);

private slots:
    void onServerStateChanged();
    void onClientConnectionStateChanged();
    void onProviderSessionImported(const QString& providerId);
    void onProviderImportCompleted(const QString& providerId, bool success, const QString& message);

private:
    void refreshConnectedClients();
    static QString formatRelativeTime(const QDateTime& dateTime);

    BrowserSessionBridgeService* m_service;
    BrowserSessionBridgeStore* m_store;
    QVariantList m_connectedClients;
    QSet<QString> m_importBusyProviders;
    QHash<QString, QString> m_importErrors;
    QHash<QString, int> m_importGenerations;
};

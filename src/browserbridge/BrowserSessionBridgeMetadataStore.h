#pragma once

#include "BrowserSessionBridgeTypes.h"

#include <QHash>
#include <QString>
#include <optional>

struct BridgeProviderBinding {
    QString preferredBindingId;
    bool autoSync = true;
    QDateTime lastImportedAtUtc;
};

class BrowserSessionBridgeMetadataStore {
public:
    bool load();
    bool save() const;

    // Client management
    QHash<QString, BridgeClientInfo> clients() const;
    void upsertClient(const BridgeClientInfo& client);
    void removeClient(const BridgeClientId& clientId);

    // Provider binding
    std::optional<BridgeProviderBinding> bindingForProvider(const QString& providerId) const;
    void setBindingForProvider(const QString& providerId, const BridgeProviderBinding& binding);

    // Auto-sync
    bool autoSyncForProvider(const QString& providerId) const;
    void setAutoSyncForProvider(const QString& providerId, bool enabled);

    // Install guide seen
    bool installGuideSeen() const;
    void setInstallGuideSeen(bool seen);

    static QString metadataFilePath();

private:
    int m_schemaVersion = 1;
    QHash<QString, BridgeClientInfo> m_clients;    // key: bindingId
    QHash<QString, BridgeProviderBinding> m_bindings; // key: providerId
    bool m_installGuideSeen = false;
};

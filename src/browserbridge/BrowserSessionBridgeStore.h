#pragma once

#include "BrowserSessionBridgeTypes.h"
#include "BrowserSessionBridgeCatalog.h"
#include "BrowserSessionBridgeMetadataStore.h"
#include "../providers/shared/ProviderCredentialStore.h"

#include <QHash>
#include <QObject>
#include <optional>

class BrowserSessionBridgeStore : public QObject {
    Q_OBJECT
public:
    explicit BrowserSessionBridgeStore(QObject* parent = nullptr);

    void upsertClient(const BridgeClientInfo& client);
    void removeClient(const BridgeClientId& clientId);

    bool saveImportedMaterial(const BridgeSessionMaterial& material);

    static QString credentialTargetFor(const QString& providerId,
                                       const QString& bindingId,
                                       BridgeMaterialKind kind = BridgeMaterialKind::Cookies);

    std::optional<QString> resolvedCookieHeader(
        const QString& providerId,
        const QString& preferredBindingId = QString()) const;

    std::optional<QString> resolvedSessionPayload(
        const QString& providerId,
        const QString& preferredBindingId = QString()) const;

    QStringList availableBindingIds(const QString& providerId) const;

    BrowserSessionBridgeMetadataStore& metadataStore();
    const BrowserSessionBridgeMetadataStore& metadataStore() const;

private:
    QString secureTargetName(const QString& providerId, const QString& bindingId) const;
    std::optional<QString> preferredOrFirstBinding(const QString& providerId,
                                                    const QString& preferredBindingId) const;

    QHash<QString, BridgeClientInfo> m_clients; // key: bindingId
    BrowserSessionBridgeMetadataStore m_metadata;
};

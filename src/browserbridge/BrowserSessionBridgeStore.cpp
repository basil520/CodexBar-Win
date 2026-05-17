#include "BrowserSessionBridgeStore.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

BrowserSessionBridgeStore::BrowserSessionBridgeStore(QObject* parent)
    : QObject(parent)
{
    m_metadata.load();
}

void BrowserSessionBridgeStore::upsertClient(const BridgeClientInfo& client)
{
    m_clients[client.id.toBindingId()] = client;
    m_metadata.upsertClient(client);
    m_metadata.save();
}

void BrowserSessionBridgeStore::removeClient(const BridgeClientId& clientId)
{
    const auto bindingId = clientId.toBindingId();
    m_clients.remove(bindingId);
    m_metadata.removeClient(clientId);
    m_metadata.save();
}

void BrowserSessionBridgeStore::saveImportedMaterial(const BridgeSessionMaterial& material)
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(material.providerId);
    if (!spec.has_value()) return;

    const auto bindingId = material.clientId.toBindingId();
    const auto target = credentialTargetFor(material.providerId, bindingId);

    if (spec->materialKind == BridgeMaterialKind::Cookies ||
        spec->materialKind == BridgeMaterialKind::Hybrid) {
        // Build cookie header: name=value; ... for non-expired cookies
        QString header;
        const auto now = QDateTime::currentDateTimeUtc();
        for (const auto& cookie : material.cookies) {
            if (cookie.expirationDateUtc.has_value() && cookie.expirationDateUtc <= now)
                continue;
            if (!header.isEmpty()) header += QStringLiteral("; ");
            header += cookie.name + QStringLiteral("=") + cookie.value;
        }
        if (!header.isEmpty()) {
            ProviderCredentialStore::write(target, QStringLiteral("bridge"), header.toUtf8());
        }
    }

    if (spec->materialKind == BridgeMaterialKind::LocalStorage ||
        spec->materialKind == BridgeMaterialKind::Hybrid) {
        if (!material.localStorage.isEmpty()) {
            QJsonObject payload;
            for (auto it = material.localStorage.constBegin();
                 it != material.localStorage.constEnd(); ++it) {
                payload[it.key()] = it.value();
            }
            const auto lsTarget = credentialTargetFor(material.providerId, bindingId,
                                                      BridgeMaterialKind::LocalStorage);
            ProviderCredentialStore::write(lsTarget, QStringLiteral("bridge"),
                                           QJsonDocument(payload).toJson(QJsonDocument::Compact));

            // Write a marker at the base target so preferredOrFirstBinding()
            // can discover this binding via exists() checks
            if (spec->materialKind == BridgeMaterialKind::LocalStorage) {
                ProviderCredentialStore::write(target, QStringLiteral("bridge"),
                                               QByteArrayLiteral("ls"));
            }
        }
    }

    // Update metadata binding
    BridgeProviderBinding binding;
    binding.preferredBindingId = bindingId;
    binding.autoSync = m_metadata.autoSyncForProvider(material.providerId);
    binding.lastImportedAtUtc = material.capturedAtUtc;
    m_metadata.setBindingForProvider(material.providerId, binding);
    m_metadata.save();
}

std::optional<QString> BrowserSessionBridgeStore::resolvedCookieHeader(
    const QString& providerId,
    const QString& preferredBindingId) const
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return std::nullopt;
    if (spec->materialKind != BridgeMaterialKind::Cookies &&
        spec->materialKind != BridgeMaterialKind::Hybrid)
        return std::nullopt;

    const auto bindingOpt = preferredOrFirstBinding(providerId, preferredBindingId);
    if (!bindingOpt.has_value()) return std::nullopt;

    const auto target = credentialTargetFor(providerId, bindingOpt.value());
    const auto data = ProviderCredentialStore::read(target);
    if (!data.has_value()) return std::nullopt;
    return QString::fromUtf8(data.value());
}

std::optional<QString> BrowserSessionBridgeStore::resolvedSessionPayload(
    const QString& providerId,
    const QString& preferredBindingId) const
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return std::nullopt;
    if (spec->materialKind != BridgeMaterialKind::LocalStorage &&
        spec->materialKind != BridgeMaterialKind::Hybrid)
        return std::nullopt;

    const auto bindingOpt = preferredOrFirstBinding(providerId, preferredBindingId);
    if (!bindingOpt.has_value()) return std::nullopt;

    const auto lsTarget = credentialTargetFor(providerId, bindingOpt.value(),
                                              BridgeMaterialKind::LocalStorage);
    const auto data = ProviderCredentialStore::read(lsTarget);
    if (!data.has_value()) return std::nullopt;
    return QString::fromUtf8(data.value());
}

QStringList BrowserSessionBridgeStore::availableBindingIds(const QString& providerId) const
{
    QStringList result;
    // A binding is available if there's a credential stored for it
    // Check all known clients that could serve this provider
    for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
        const auto& client = it.value();
        const auto bindingId = it.key();
        const auto target = credentialTargetFor(providerId, bindingId);
        const auto lsTarget = credentialTargetFor(providerId, bindingId, BridgeMaterialKind::LocalStorage);
        if (ProviderCredentialStore::exists(target) || ProviderCredentialStore::exists(lsTarget)) {
            result.append(bindingId);
        }
    }
    return result;
}

BrowserSessionBridgeMetadataStore& BrowserSessionBridgeStore::metadataStore()
{
    return m_metadata;
}

const BrowserSessionBridgeMetadataStore& BrowserSessionBridgeStore::metadataStore() const
{
    return m_metadata;
}

QString BrowserSessionBridgeStore::secureTargetName(
    const QString& providerId, const QString& bindingId) const
{
    return credentialTargetFor(providerId, bindingId);
}

QString BrowserSessionBridgeStore::credentialTargetFor(
    const QString& providerId, const QString& bindingId, BridgeMaterialKind kind)
{
    QString target = QStringLiteral("CodexBarX/bridge/session/%1/%2").arg(providerId, bindingId);
    if (kind == BridgeMaterialKind::LocalStorage) {
        target += QStringLiteral("/ls");
    }
    return target;
}

std::optional<QString> BrowserSessionBridgeStore::preferredOrFirstBinding(
    const QString& providerId,
    const QString& preferredBindingId) const
{
    if (!preferredBindingId.isEmpty()) {
        const auto target = credentialTargetFor(providerId, preferredBindingId);
        const auto lsTarget = credentialTargetFor(providerId, preferredBindingId, BridgeMaterialKind::LocalStorage);
        if (ProviderCredentialStore::exists(target) || ProviderCredentialStore::exists(lsTarget))
            return preferredBindingId;
    }

    // Fall back to metadata preferred binding
    const auto binding = m_metadata.bindingForProvider(providerId);
    if (binding && !binding->preferredBindingId.isEmpty()) {
        const auto target = credentialTargetFor(providerId, binding->preferredBindingId);
        const auto lsTarget = credentialTargetFor(providerId, binding->preferredBindingId,
                                                  BridgeMaterialKind::LocalStorage);
        if (ProviderCredentialStore::exists(target) || ProviderCredentialStore::exists(lsTarget))
            return binding->preferredBindingId;
    }

    // Fall back to first available
    const auto available = availableBindingIds(providerId);
    if (available.isEmpty()) return std::nullopt;
    return available.first();
}

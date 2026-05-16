#include "BrowserSessionBridgeMetadataStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

QString BrowserSessionBridgeMetadataStore::metadataFilePath()
{
    const auto appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appData + QStringLiteral("/browser-session-bridge.json");
}

bool BrowserSessionBridgeMetadataStore::load()
{
    const auto path = metadataFilePath();
    QFile f(path);
    if (!f.exists()) return true; // fresh start is fine
    if (!f.open(QIODevice::ReadOnly)) return false;

    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError) return false;
    if (!doc.isObject()) return false;

    const auto root = doc.object();
    m_schemaVersion = root[QStringLiteral("schemaVersion")].toInt(1);
    m_installGuideSeen = root[QStringLiteral("installGuideSeen")].toBool();

    {
        const auto clients = root[QStringLiteral("clients")].toObject();
        for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
            const auto obj = it.value().toObject();
            BridgeClientInfo info;
            info.id.browserFamily = obj[QStringLiteral("browserFamily")].toString();
            info.id.profileInstanceId = obj[QStringLiteral("profileInstanceId")].toString();
            info.id.incognito = obj[QStringLiteral("incognito")].toBool();
            info.extensionId = obj[QStringLiteral("extensionId")].toString();
            info.profileAlias = obj[QStringLiteral("profileAlias")].toString();
            info.browserVersion = obj[QStringLiteral("browserVersion")].toString();
            info.connectedAt = QDateTime::fromString(
                obj[QStringLiteral("connectedAtUtc")].toString(), Qt::ISODate);
            info.lastSeenAt = QDateTime::fromString(
                obj[QStringLiteral("lastSeenAtUtc")].toString(), Qt::ISODate);
            info.supportsCookies = obj[QStringLiteral("supportsCookies")].toBool(true);
            info.supportsLocalStorage = obj[QStringLiteral("supportsLocalStorage")].toBool();
            m_clients[it.key()] = info;
        }
    }

    {
        const auto bindings = root[QStringLiteral("bindings")].toObject();
        for (auto it = bindings.constBegin(); it != bindings.constEnd(); ++it) {
            const auto obj = it.value().toObject();
            BridgeProviderBinding b;
            b.preferredBindingId = obj[QStringLiteral("preferredBindingId")].toString();
            b.autoSync = obj[QStringLiteral("autoSync")].toBool(true);
            b.lastImportedAtUtc = QDateTime::fromString(
                obj[QStringLiteral("lastImportedAtUtc")].toString(), Qt::ISODate);
            m_bindings[it.key()] = b;
        }
    }

    return true;
}

bool BrowserSessionBridgeMetadataStore::save() const
{
    const auto path = metadataFilePath();
    const auto dir = QFileInfo(path).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) return false;
    }

    QJsonObject root;
    root[QStringLiteral("schemaVersion")] = m_schemaVersion;
    root[QStringLiteral("installGuideSeen")] = m_installGuideSeen;

    {
        QJsonObject clients;
        for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
            const auto& info = it.value();
            QJsonObject obj;
            obj[QStringLiteral("browserFamily")] = info.id.browserFamily;
            obj[QStringLiteral("profileInstanceId")] = info.id.profileInstanceId;
            obj[QStringLiteral("incognito")] = info.id.incognito;
            obj[QStringLiteral("extensionId")] = info.extensionId;
            obj[QStringLiteral("profileAlias")] = info.profileAlias;
            obj[QStringLiteral("browserVersion")] = info.browserVersion;
            if (info.connectedAt.isValid())
                obj[QStringLiteral("connectedAtUtc")] = info.connectedAt.toString(Qt::ISODate);
            if (info.lastSeenAt.isValid())
                obj[QStringLiteral("lastSeenAtUtc")] = info.lastSeenAt.toString(Qt::ISODate);
            obj[QStringLiteral("supportsCookies")] = info.supportsCookies;
            obj[QStringLiteral("supportsLocalStorage")] = info.supportsLocalStorage;
            clients[it.key()] = obj;
        }
        root[QStringLiteral("clients")] = clients;
    }

    {
        QJsonObject bindings;
        for (auto it = m_bindings.constBegin(); it != m_bindings.constEnd(); ++it) {
            const auto& b = it.value();
            QJsonObject obj;
            obj[QStringLiteral("preferredBindingId")] = b.preferredBindingId;
            obj[QStringLiteral("autoSync")] = b.autoSync;
            if (b.lastImportedAtUtc.isValid())
                obj[QStringLiteral("lastImportedAtUtc")] = b.lastImportedAtUtc.toString(Qt::ISODate);
            bindings[it.key()] = obj;
        }
        root[QStringLiteral("bindings")] = bindings;
    }

    // Atomic write: write to temp, then rename
    const auto tempPath = path + QStringLiteral(".tmp");
    {
        QFile f(tempPath);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
        const auto data = QJsonDocument(root).toJson(QJsonDocument::Indented);
        if (f.write(data) != data.size()) return false;
        f.close();
    }

    QFile::remove(path);
    if (!QFile::rename(tempPath, path)) {
        QFile::remove(tempPath);
        return false;
    }
    return true;
}

QHash<QString, BridgeClientInfo> BrowserSessionBridgeMetadataStore::clients() const
{
    return m_clients;
}

void BrowserSessionBridgeMetadataStore::upsertClient(const BridgeClientInfo& client)
{
    m_clients[client.id.toBindingId()] = client;
}

void BrowserSessionBridgeMetadataStore::removeClient(const BridgeClientId& clientId)
{
    m_clients.remove(clientId.toBindingId());
}

std::optional<BridgeProviderBinding> BrowserSessionBridgeMetadataStore::bindingForProvider(const QString& providerId) const
{
    auto it = m_bindings.constFind(providerId);
    if (it == m_bindings.constEnd()) return std::nullopt;
    return it.value();
}

void BrowserSessionBridgeMetadataStore::setBindingForProvider(const QString& providerId, const BridgeProviderBinding& binding)
{
    m_bindings[providerId] = binding;
}

bool BrowserSessionBridgeMetadataStore::autoSyncForProvider(const QString& providerId) const
{
    auto it = m_bindings.find(providerId);
    if (it == m_bindings.constEnd()) return true;
    return it->autoSync;
}

void BrowserSessionBridgeMetadataStore::setAutoSyncForProvider(const QString& providerId, bool enabled)
{
    auto& b = m_bindings[providerId];
    b.autoSync = enabled;
}

bool BrowserSessionBridgeMetadataStore::installGuideSeen() const
{
    return m_installGuideSeen;
}

void BrowserSessionBridgeMetadataStore::setInstallGuideSeen(bool seen)
{
    m_installGuideSeen = seen;
}

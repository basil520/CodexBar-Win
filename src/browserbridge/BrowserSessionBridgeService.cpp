#include "BrowserSessionBridgeService.h"

#include "BrowserSessionBridgeCatalog.h"
#include "BrowserSessionBridgeInstallService.h"
#include "../providers/shared/ProviderCredentialStore.h"

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QUuid>

namespace {

bool hasUsableCookie(const ImportResultPayload& payload)
{
    const auto now = QDateTime::currentDateTimeUtc();
    for (const auto& cookie : payload.cookies) {
        if (cookie.name.isEmpty() || cookie.value.isEmpty()) continue;
        if (cookie.expirationDateUtc.has_value() && cookie.expirationDateUtc <= now) continue;
        return true;
    }
    return false;
}

bool isDiagnosticLocalStorageKey(const QString& key)
{
    return key == QLatin1String("cookie_query_diagnostics")
        || key == QLatin1String("cookie_query_error")
        || key == QLatin1String("localStorage_error")
        || key.endsWith(QLatin1String("_diagnostics"))
        || key.endsWith(QLatin1String("_error"));
}

bool hasUsableLocalStorage(const ImportResultPayload& payload, const BridgeProviderSpec& spec)
{
    for (auto it = payload.localStorage.constBegin(); it != payload.localStorage.constEnd(); ++it) {
        if (it.value().trimmed().isEmpty()) continue;
        if (isDiagnosticLocalStorageKey(it.key())) continue;
        if (!spec.localStorageKeys.isEmpty() && !spec.localStorageKeys.contains(it.key())) continue;
        return true;
    }
    return false;
}

QString firstLocalStorageValue(const ImportResultPayload& payload, std::initializer_list<const char*> keys)
{
    for (const char* key : keys) {
        const QString value = payload.localStorage.value(QLatin1String(key)).trimmed();
        if (!value.isEmpty()) return value;
    }
    return {};
}

bool hasKimiUsableLocalStorage(const ImportResultPayload& payload)
{
    if (!firstLocalStorageValue(payload, {
            "access_token",
            "anonymous_access_token",
            "kimi_auth_token",
            "kimi-auth"
        }).isEmpty()) {
        return true;
    }

    const QString volcanoTokenInfo = payload.localStorage.value(
        QStringLiteral("volcano-token-info")).trimmed();
    if (volcanoTokenInfo.isEmpty()) return false;

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(volcanoTokenInfo.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return false;

    const QJsonObject obj = doc.object();
    for (const QString& key : {
             QStringLiteral("access_token"),
             QStringLiteral("accessToken"),
             QStringLiteral("token")
         }) {
        if (!obj.value(key).toString().trimmed().isEmpty()) return true;
    }
    return false;
}

QString codexUsageMaterialError(const ImportResultPayload& payload)
{
    const QString bridgeError = payload.localStorage.value(QStringLiteral("codex_usage_error")).trimmed();
    if (!bridgeError.isEmpty()) {
        return bridgeError;
    }

    const QString usageText = payload.localStorage.value(QStringLiteral("codex_usage_json")).trimmed();
    if (usageText.isEmpty()) {
        return QStringLiteral("No Codex usage snapshot was returned from the connected browser profile. Prepare the extension again, reload it in the browser, make sure chatgpt.com is signed in, and try again.");
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(usageText.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return QStringLiteral("The Codex usage snapshot returned by the browser extension was not valid JSON.");
    }

    return QString();
}

bool hasUsableMaterial(const ImportResultPayload& payload, const BridgeProviderSpec& spec)
{
    if (spec.providerId == QLatin1String("codex")) {
        return codexUsageMaterialError(payload).isEmpty();
    }
    if (spec.providerId == QLatin1String("kimi")) {
        return hasUsableCookie(payload) || hasKimiUsableLocalStorage(payload);
    }

    switch (spec.materialKind) {
    case BridgeMaterialKind::Cookies:
        return hasUsableCookie(payload);
    case BridgeMaterialKind::LocalStorage:
        return hasUsableLocalStorage(payload, spec);
    case BridgeMaterialKind::Hybrid:
        return hasUsableCookie(payload) || hasUsableLocalStorage(payload, spec);
    }
    return false;
}

bool requiresCookieUrlQueryCapableClient(const BridgeProviderSpec& spec)
{
    if (spec.providerId == QLatin1String("codex")) {
        return false;
    }
    return spec.materialKind == BridgeMaterialKind::Cookies
        || spec.materialKind == BridgeMaterialKind::Hybrid;
}

bool requiresAllUrlsCookiePermission(const BridgeProviderSpec& spec)
{
    return spec.materialKind == BridgeMaterialKind::Cookies
        || spec.materialKind == BridgeMaterialKind::Hybrid;
}

QString emptyMaterialMessage(const BridgeProviderSpec& spec)
{
    if (spec.providerId == QLatin1String("codex")) {
        return QStringLiteral("No Codex usage snapshot was returned from the connected browser profile. Prepare the extension again, reload it in the browser, make sure chatgpt.com is signed in, and try again.");
    }
    if (spec.providerId == QLatin1String("kimi")) {
        return QStringLiteral("No Kimi access token or kimi-auth cookie was returned from the connected browser profile. Make sure www.kimi.com is signed in, then try importing again.");
    }
    if (spec.materialKind == BridgeMaterialKind::LocalStorage) {
        return QStringLiteral("No localStorage session data was returned from the connected browser profile. Make sure the page is signed in and try again.");
    }
    if (spec.materialKind == BridgeMaterialKind::Hybrid) {
        return QStringLiteral("No cookies or localStorage session data were returned from the connected browser profile. Make sure the page is signed in and try again.");
    }
    const QString domains = spec.domains.join(QStringLiteral(", "));
    return domains.isEmpty()
        ? QStringLiteral("No cookies were returned from the connected browser profile. Make sure you are signed in with this browser profile and try again.")
        : QStringLiteral("No cookies were returned for %1 from the connected browser profile. Make sure you are signed in with this browser profile and try again.")
            .arg(domains);
}

QString cookieDiagnosticsMessage(const ImportResultPayload& payload)
{
    const QString diagnostics = payload.localStorage.value(
        QStringLiteral("cookie_query_diagnostics")).trimmed();
    if (diagnostics.isEmpty()) return QString();

    QString compact = diagnostics.simplified();
    if (compact.size() > 420) {
        compact = compact.left(420) + QStringLiteral("...");
    }
    return QStringLiteral(" Cookie query diagnostics: %1").arg(compact);
}

QString unusableMaterialMessage(const ImportResultPayload& payload, const BridgeProviderSpec& spec)
{
    if (spec.providerId == QLatin1String("codex")) {
        return codexUsageMaterialError(payload);
    }
    return emptyMaterialMessage(spec) + cookieDiagnosticsMessage(payload);
}

QString selectedProfileUnavailableMessage()
{
    return QStringLiteral("The selected browser profile is not connected or is outdated for this provider. Choose Auto or another connected profile, or click Prepare Extension and reload the unpacked extension in Edge/Chrome.");
}

QString outdatedCodexExtensionMessage()
{
    return QStringLiteral("The connected Browser Session Bridge extension is outdated for Codex import. Click Prepare Extension, then reload the unpacked extension in Edge/Chrome and try again.");
}

QString outdatedCookieExtensionMessage()
{
    return QStringLiteral("The connected Browser Session Bridge extension is outdated for cookie import. Click Prepare Extension, then reload the unpacked extension in Edge/Chrome so it can use the updated all-sites cookie permission.");
}

} // namespace

BrowserSessionBridgeService::BrowserSessionBridgeService(BrowserSessionBridgeStore* store, QObject* parent)
    : QObject(parent)
    , m_store(store)
{
    if (m_store) {
        m_knownClients = m_store->metadataStore().clients();
        m_providerBindings = m_store->metadataStore().bindings();
        m_installGuideSeen = m_store->metadataStore().installGuideSeen();
    }

    m_ioWorker = new QObject();
    m_ioWorker->moveToThread(&m_ioThread);
    m_ioThread.start();

    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(1500);
    connect(&m_debounceTimer, &QTimer::timeout, this, &BrowserSessionBridgeService::onDebounceRefresh);

    refreshExtensionExportStateAsync();
}

BrowserSessionBridgeService::~BrowserSessionBridgeService()
{
    stop();
    if (m_ioWorker) {
        QMetaObject::invokeMethod(m_ioWorker, [worker = m_ioWorker]() {
            worker->deleteLater();
        }, Qt::BlockingQueuedConnection);
        m_ioWorker = nullptr;
    }
    m_ioThread.quit();
    m_ioThread.wait(3000);
}

void BrowserSessionBridgeService::start()
{
    if (m_server) return;

    m_server = new BrowserSessionBridgeServer();

    connect(m_server, &BrowserSessionBridgeServer::serverStateChanged,
            this, &BrowserSessionBridgeService::onServerStateChanged);
    connect(m_server, &BrowserSessionBridgeServer::clientRegistered,
            this, &BrowserSessionBridgeService::onClientRegistered);
    connect(m_server, &BrowserSessionBridgeServer::clientDisconnected,
            this, &BrowserSessionBridgeService::onClientDisconnected);
    connect(m_server, &BrowserSessionBridgeServer::importResultReceived,
            this, &BrowserSessionBridgeService::onImportResultReceived);
    connect(m_server, &BrowserSessionBridgeServer::sessionDirtyReceived,
            this, &BrowserSessionBridgeService::onSessionDirtyReceived);
    connect(m_server, &BrowserSessionBridgeServer::errorOccurred,
            this, &BrowserSessionBridgeService::onServerError);

    m_server->start();
}

void BrowserSessionBridgeService::stop()
{
    if (m_server) {
        m_server->stop();
        delete m_server;
        m_server = nullptr;
    }
    m_connectedBindingIds.clear();
    onServerStateChanged(false, 0);
}

BrowserSessionBridgeStore* BrowserSessionBridgeService::store() const
{
    return m_store;
}

bool BrowserSessionBridgeService::requestImport(const QString& providerId,
                                                const QString& preferredBindingId)
{
    if (!m_server || !m_serverRunning) {
        setLastError(QStringLiteral("Browser Session Bridge server is not running."));
        return false;
    }

    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) {
        setLastError(QStringLiteral("Provider is not supported by Browser Session Bridge."));
        return false;
    }

    QString selectedBindingId = preferredBindingId.trimmed();
    if (selectedBindingId.isEmpty()) {
        const auto binding = m_providerBindings.constFind(providerId);
        if (binding != m_providerBindings.constEnd()) {
            selectedBindingId = binding->preferredBindingId.trimmed();
        }
    }

    const QString bindingId = resolveTargetBindingId(providerId, preferredBindingId);
    if (!selectedBindingId.isEmpty() && bindingId != selectedBindingId) {
        setLastError(selectedProfileUnavailableMessage());
        return false;
    }
    if (bindingId.isEmpty() || !m_connectedBindingIds.contains(bindingId)) {
        bool hasOutdatedCodexClient = false;
        bool hasOutdatedCookieClient = false;
        if (providerId == QLatin1String("codex")) {
            for (const auto& connectedId : m_connectedBindingIds) {
                const auto it = m_knownClients.constFind(connectedId);
                if (it == m_knownClients.constEnd()) continue;
                const BridgeClientInfo& client = it.value();
                if (client.supportsCookies && client.supportsLocalStorage &&
                    !client.supportsCodexUsageSnapshot) {
                    hasOutdatedCodexClient = true;
                    break;
                }
            }
        }
        if (requiresCookieUrlQueryCapableClient(spec.value())) {
            for (const auto& connectedId : m_connectedBindingIds) {
                const auto it = m_knownClients.constFind(connectedId);
                if (it == m_knownClients.constEnd()) continue;
                const BridgeClientInfo& client = it.value();
                if (client.supportsCookies && !client.supportsCookieUrlQuery) {
                    hasOutdatedCookieClient = true;
                    break;
                }
            }
        }
        if (requiresAllUrlsCookiePermission(spec.value())) {
            for (const auto& connectedId : m_connectedBindingIds) {
                const auto it = m_knownClients.constFind(connectedId);
                if (it == m_knownClients.constEnd()) continue;
                const BridgeClientInfo& client = it.value();
                if (client.supportsCookies && !client.supportsAllUrlsCookiePermission) {
                    hasOutdatedCookieClient = true;
                    break;
                }
            }
        }
        setLastError(hasOutdatedCodexClient
            ? outdatedCodexExtensionMessage()
            : hasOutdatedCookieClient
                ? outdatedCookieExtensionMessage()
                : QStringLiteral("No connected browser profile is available for this provider."));
        return false;
    }

    RequestImportPayload req;
    req.requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    req.providerId = providerId;
    req.materialKind = spec->materialKind;
    req.domains = spec->domains;
    req.cookieNames = spec->cookieNames;
    req.localStorageOrigin = spec->localStorageOrigin;
    req.localStorageKeys = spec->localStorageKeys;

    m_server->sendRequestImport(req, bindingId);
    setLastError(QString());
    return true;
}

void BrowserSessionBridgeService::prepareExtension()
{
    if (m_extensionPreparing) return;
    m_extensionPreparing = true;
    emit extensionStateChanged();

    postIoTask([this]() {
        BrowserSessionBridgeInstallService installService;
        const bool ok = installService.ensureExtensionExported();
        const QString error = ok ? QString() : QStringLiteral("Failed to prepare browser extension files.");
        QMetaObject::invokeMethod(this, [this, ok, error]() {
            m_extensionPreparing = false;
            m_extensionExported = ok;
            setLastError(error);
            emit extensionStateChanged();
        }, Qt::QueuedConnection);
    });
}

void BrowserSessionBridgeService::setInstallGuideSeenAsync(bool seen)
{
    if (m_installGuideSeen == seen) return;
    m_installGuideSeen = seen;
    emit installGuideSeenChanged();
    persistInstallGuideSeenAsync(seen);
}

void BrowserSessionBridgeService::setProviderBindingAsync(const QString& providerId, const QString& bindingId)
{
    BridgeProviderBinding binding = m_providerBindings.value(providerId);
    binding.preferredBindingId = bindingId;
    m_providerBindings[providerId] = binding;
    emit providerBindingChanged(providerId);
    persistProviderBindingAsync(providerId, binding);
}

void BrowserSessionBridgeService::setAutoSyncAsync(const QString& providerId, bool enabled)
{
    BridgeProviderBinding binding = m_providerBindings.value(providerId);
    binding.autoSync = enabled;
    m_providerBindings[providerId] = binding;
    emit providerBindingChanged(providerId);
    persistProviderBindingAsync(providerId, binding);
}

bool BrowserSessionBridgeService::isClientConnected(const QString& bindingId) const
{
    return m_connectedBindingIds.contains(bindingId);
}

QStringList BrowserSessionBridgeService::connectedClientBindingIds() const
{
    return QStringList(m_connectedBindingIds.begin(), m_connectedBindingIds.end());
}

QVector<BridgeBindingOption> BrowserSessionBridgeService::bindingOptions(const QString& providerId) const
{
    QVector<BridgeBindingOption> options;
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return options;

    const auto binding = m_providerBindings.constFind(providerId);
    const QString importedBindingId = binding == m_providerBindings.constEnd()
        ? QString()
        : binding->preferredBindingId;
    const bool hasImport = binding != m_providerBindings.constEnd()
        && binding->lastImportedAtUtc.isValid();

    for (auto it = m_knownClients.constBegin(); it != m_knownClients.constEnd(); ++it) {
        const QString bindingId = it.key();
        const BridgeClientInfo& client = it.value();
        if (!clientSupportsProvider(client, spec.value())) continue;

        BridgeBindingOption option;
        option.bindingId = bindingId;
        option.browserFamily = client.id.browserFamily;
        const QString alias = client.profileAlias.isEmpty() ? QStringLiteral("Default") : client.profileAlias;
        const QString shortId = client.id.profileInstanceId.left(8);
        option.label = QStringLiteral("%1 - %2 (%3)")
            .arg(client.id.browserFamily, alias, shortId);
        option.connected = m_connectedBindingIds.contains(bindingId);
        option.hasMaterial = hasImport && importedBindingId == bindingId;
        options.append(option);
    }
    return options;
}

std::optional<BridgeProviderBinding> BrowserSessionBridgeService::bindingForProvider(
    const QString& providerId) const
{
    const auto it = m_providerBindings.constFind(providerId);
    if (it == m_providerBindings.constEnd()) return std::nullopt;
    return it.value();
}

bool BrowserSessionBridgeService::autoSyncForProvider(const QString& providerId) const
{
    const auto it = m_providerBindings.constFind(providerId);
    if (it == m_providerBindings.constEnd()) return true;
    return it->autoSync;
}

bool BrowserSessionBridgeService::installGuideSeen() const
{
    return m_installGuideSeen;
}

bool BrowserSessionBridgeService::extensionExported() const
{
    return m_extensionExported;
}

bool BrowserSessionBridgeService::extensionPreparing() const
{
    return m_extensionPreparing;
}

QString BrowserSessionBridgeService::lastError() const
{
    return m_lastError;
}

std::optional<BridgeSessionLookupInput> BrowserSessionBridgeService::sessionLookupForProvider(
    const QString& providerId) const
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return std::nullopt;

    QString bindingId;
    const auto binding = m_providerBindings.constFind(providerId);
    if (binding != m_providerBindings.constEnd() && !binding->preferredBindingId.isEmpty()) {
        bindingId = binding->preferredBindingId;
    } else {
        bindingId = resolveTargetBindingId(providerId, QString());
    }
    if (bindingId.isEmpty()) return std::nullopt;

    BridgeSessionLookupInput lookup;
    lookup.enabled = true;
    lookup.providerId = providerId;
    lookup.materialKind = spec->materialKind;
    if (spec->materialKind == BridgeMaterialKind::Cookies ||
        spec->materialKind == BridgeMaterialKind::Hybrid) {
        lookup.cookieCredentialTarget = BrowserSessionBridgeStore::credentialTargetFor(
            providerId, bindingId, BridgeMaterialKind::Cookies);
    }
    if (spec->materialKind == BridgeMaterialKind::LocalStorage ||
        spec->materialKind == BridgeMaterialKind::Hybrid) {
        lookup.localStorageCredentialTarget = BrowserSessionBridgeStore::credentialTargetFor(
            providerId, bindingId, BridgeMaterialKind::LocalStorage);
    }
    return lookup;
}

bool BrowserSessionBridgeService::isServerRunning() const
{
    return m_serverRunning;
}

quint16 BrowserSessionBridgeService::serverPort() const
{
    return m_serverPort;
}

void BrowserSessionBridgeService::onServerStateChanged(bool running, quint16 port)
{
    if (m_serverRunning == running && m_serverPort == port) {
        return;
    }

    m_serverRunning = running;
    m_serverPort = port;
    emit serverStateChanged();
}

void BrowserSessionBridgeService::onClientRegistered(const BridgeClientInfo& client)
{
    const QString bindingId = client.id.toBindingId();
    m_knownClients[bindingId] = client;
    m_connectedBindingIds.insert(bindingId);
    persistClientAsync(client);

    RegisterAckPayload ack;
    ack.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    ack.accepted = true;
    ack.providerSpecs = BrowserSessionBridgeCatalog::specs();

    if (m_server) {
        m_server->sendRegisterAck(ack, bindingId);
    }

    emit clientConnectionStateChanged();
}

void BrowserSessionBridgeService::onClientDisconnected(const BridgeClientId& clientId)
{
    m_connectedBindingIds.remove(clientId.toBindingId());
    emit clientConnectionStateChanged();
}

void BrowserSessionBridgeService::onImportResultReceived(const ImportResultPayload& payload,
                                                         const BridgeClientId& clientId)
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(payload.providerId);
    if (!spec.has_value()) return;
    if (!payload.success || !payload.errorCode.isEmpty()) {
        const QString message = payload.errorMessage.isEmpty()
            ? payload.errorCode
            : payload.errorMessage;
        if (payload.providerId == QLatin1String("codex")) {
            persistCodexImportFailureAsync(payload, clientId, message);
        }
        setLastError(message);
        emit providerImportCompleted(payload.providerId, false, message);
        return;
    }
    if (!hasUsableMaterial(payload, spec.value())) {
        const QString message = unusableMaterialMessage(payload, spec.value());
        if (payload.providerId == QLatin1String("codex")) {
            persistCodexImportFailureAsync(payload, clientId, message);
        }
        setLastError(message);
        emit providerImportCompleted(payload.providerId, false, message);
        return;
    }

    BridgeSessionMaterial material;
    material.providerId = payload.providerId;
    material.clientId = clientId;
    material.cookies = payload.cookies;
    material.localStorage = payload.localStorage;
    material.capturedAtUtc = payload.capturedAtUtc.isValid()
        ? payload.capturedAtUtc
        : QDateTime::currentDateTimeUtc();
    material.sourceReason = QStringLiteral("manual_request");

    postIoTask([this, material]() {
        BrowserSessionBridgeStore store;
        const bool persisted = store.saveImportedMaterial(material);
        QMetaObject::invokeMethod(this, [this, material, persisted]() {
            if (!persisted) {
                const QString message = QStringLiteral("Failed to persist imported browser session. Windows Credential Manager may have rejected the session payload; try importing again after reloading the extension.");
                setLastError(message);
                emit providerImportCompleted(material.providerId, false, message);
                return;
            }
            BridgeProviderBinding binding = m_providerBindings.value(material.providerId);
            binding.preferredBindingId = material.clientId.toBindingId();
            binding.lastImportedAtUtc = material.capturedAtUtc;
            m_providerBindings[material.providerId] = binding;
            emit providerBindingChanged(material.providerId);
            emit providerImportCompleted(material.providerId, true, QString());
            enqueueDebounceRefresh(material.providerId);
        }, Qt::QueuedConnection);
    });
}

void BrowserSessionBridgeService::onSessionDirtyReceived(const SessionDirtyPayload& payload)
{
    for (const auto& providerId : payload.providerIds) {
        if (autoSyncForProvider(providerId)) {
            requestImport(providerId);
        }
    }
}

void BrowserSessionBridgeService::onServerError(const QString& error)
{
    qWarning() << "[BrowserSessionBridgeServer]" << error;
    setLastError(error);
}

void BrowserSessionBridgeService::onDebounceRefresh()
{
    const auto providers = m_pendingRefreshProviders;
    m_pendingRefreshProviders.clear();
    for (const auto& providerId : providers) {
        emit providerSessionImported(providerId);
    }
}

void BrowserSessionBridgeService::postIoTask(const std::function<void()>& task) const
{
    if (!m_ioWorker) return;
    QMetaObject::invokeMethod(m_ioWorker, [task]() {
        task();
    }, Qt::QueuedConnection);
}

void BrowserSessionBridgeService::persistClientAsync(const BridgeClientInfo& client)
{
    postIoTask([client]() {
        BrowserSessionBridgeStore store;
        store.upsertClient(client);
    });
}

void BrowserSessionBridgeService::persistProviderBindingAsync(const QString& providerId,
                                                             const BridgeProviderBinding& binding)
{
    postIoTask([providerId, binding]() {
        BrowserSessionBridgeStore store;
        store.metadataStore().setBindingForProvider(providerId, binding);
        store.metadataStore().save();
    });
}

void BrowserSessionBridgeService::persistCodexImportFailureAsync(const ImportResultPayload& payload,
                                                                 const BridgeClientId& clientId,
                                                                 const QString& message)
{
    if (payload.providerId != QLatin1String("codex")) return;
    const QString bindingId = clientId.toBindingId();
    const QString diagnosticMessage = message.trimmed().isEmpty()
        ? QStringLiteral("Codex browser import failed.")
        : message.trimmed();
    const QString errorCode = payload.errorCode;
    const QDateTime capturedAtUtc = payload.capturedAtUtc.isValid()
        ? payload.capturedAtUtc
        : QDateTime::currentDateTimeUtc();
    const QHash<QString, QString> localStorage = payload.localStorage;

    postIoTask([bindingId, diagnosticMessage, errorCode, capturedAtUtc, localStorage]() {
        QJsonObject diagnostic;
        for (auto it = localStorage.constBegin(); it != localStorage.constEnd(); ++it) {
            if (it.key() == QLatin1String("codex_usage_json")) continue;
            diagnostic[it.key()] = it.value();
        }
        if (!diagnostic.contains(QStringLiteral("codex_usage_error"))) {
            diagnostic[QStringLiteral("codex_usage_error")] = diagnosticMessage;
        }
        if (!errorCode.isEmpty()) {
            diagnostic[QStringLiteral("codex_error_code")] = errorCode;
        }
        diagnostic[QStringLiteral("capturedAtUtc")] = capturedAtUtc.toString(Qt::ISODate);

        const QString target = BrowserSessionBridgeStore::credentialTargetFor(
            QStringLiteral("codex"), bindingId, BridgeMaterialKind::LocalStorage);
        ProviderCredentialStore::write(target, QStringLiteral("bridge"),
                                       QJsonDocument(diagnostic).toJson(QJsonDocument::Compact));
    });
}

void BrowserSessionBridgeService::persistInstallGuideSeenAsync(bool seen)
{
    postIoTask([seen]() {
        BrowserSessionBridgeStore store;
        store.metadataStore().setInstallGuideSeen(seen);
        store.metadataStore().save();
    });
}

void BrowserSessionBridgeService::refreshExtensionExportStateAsync()
{
    postIoTask([this]() {
        BrowserSessionBridgeInstallService installService;
        const bool exported = installService.isExtensionExported();
        QMetaObject::invokeMethod(this, [this, exported]() {
            if (m_extensionExported == exported) return;
            m_extensionExported = exported;
            emit extensionStateChanged();
        }, Qt::QueuedConnection);
    });
}

void BrowserSessionBridgeService::enqueueDebounceRefresh(const QString& providerId)
{
    m_pendingRefreshProviders.insert(providerId);
    m_debounceTimer.start();
}

QString BrowserSessionBridgeService::resolveTargetBindingId(const QString& providerId,
                                                           const QString& preferredBindingId) const
{
    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return QString();

    auto isUsable = [&](const QString& bindingId) {
        if (!m_connectedBindingIds.contains(bindingId)) return false;
        const auto client = m_knownClients.constFind(bindingId);
        return client != m_knownClients.constEnd() && clientSupportsProvider(client.value(), spec.value());
    };

    if (!preferredBindingId.isEmpty() && isUsable(preferredBindingId)) {
        return preferredBindingId;
    }

    const auto binding = m_providerBindings.constFind(providerId);
    if (binding != m_providerBindings.constEnd()
        && !binding->preferredBindingId.isEmpty()
        && isUsable(binding->preferredBindingId)) {
        return binding->preferredBindingId;
    }

    for (const auto& id : m_connectedBindingIds) {
        if (isUsable(id)) return id;
    }

    return QString();
}

bool BrowserSessionBridgeService::clientSupportsProvider(const BridgeClientInfo& client,
                                                         const BridgeProviderSpec& spec) const
{
    if (spec.providerId == QLatin1String("codex") && !client.supportsCodexUsageSnapshot) {
        return false;
    }
    if ((spec.materialKind == BridgeMaterialKind::Cookies ||
         spec.materialKind == BridgeMaterialKind::Hybrid) && !client.supportsCookies) {
        return false;
    }
    if (requiresCookieUrlQueryCapableClient(spec) && !client.supportsCookieUrlQuery) {
        return false;
    }
    if (requiresAllUrlsCookiePermission(spec) && !client.supportsAllUrlsCookiePermission) {
        return false;
    }
    if ((spec.materialKind == BridgeMaterialKind::LocalStorage ||
         spec.materialKind == BridgeMaterialKind::Hybrid) && !client.supportsLocalStorage) {
        return false;
    }
    return true;
}

void BrowserSessionBridgeService::setLastError(const QString& message)
{
    if (m_lastError == message) return;
    m_lastError = message;
    emit lastErrorChanged();
}

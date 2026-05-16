#include "BrowserSessionBridgeService.h"

#include "BrowserSessionBridgeCatalog.h"

#include <QDebug>

BrowserSessionBridgeService::BrowserSessionBridgeService(BrowserSessionBridgeStore* store, QObject* parent)
    : QObject(parent)
    , m_store(store)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(1500);
    connect(&m_debounceTimer, &QTimer::timeout, this, &BrowserSessionBridgeService::onDebounceRefresh);
}

BrowserSessionBridgeService::~BrowserSessionBridgeService()
{
    stop();
}

void BrowserSessionBridgeService::start()
{
    if (m_server) return;

    m_server = new BrowserSessionBridgeServer(this);

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
        m_server->deleteLater();
        m_server = nullptr;
    }
    m_connectedBindingIds.clear();
}

BrowserSessionBridgeStore* BrowserSessionBridgeService::store() const
{
    return m_store;
}

void BrowserSessionBridgeService::requestImport(const QString& providerId,
                                                 const QString& preferredBindingId)
{
    if (!m_server || !m_server->isRunning()) return;

    const auto spec = BrowserSessionBridgeCatalog::specForProvider(providerId);
    if (!spec.has_value()) return;

    const QString bindingId = resolveTargetBindingId(providerId, preferredBindingId);
    if (bindingId.isEmpty() || !m_connectedBindingIds.contains(bindingId)) return;

    RequestImportPayload req;
    req.requestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    req.providerId = providerId;
    req.materialKind = spec->materialKind;
    req.domains = spec->domains;
    req.cookieNames = spec->cookieNames;
    req.localStorageOrigin = spec->localStorageOrigin;
    req.localStorageKeys = spec->localStorageKeys;

    m_server->sendRequestImport(req, bindingId);
}

bool BrowserSessionBridgeService::isClientConnected(const QString& bindingId) const
{
    return m_connectedBindingIds.contains(bindingId);
}

QStringList BrowserSessionBridgeService::connectedClientBindingIds() const
{
    return QStringList(m_connectedBindingIds.begin(), m_connectedBindingIds.end());
}

void BrowserSessionBridgeService::onClientRegistered(const BridgeClientInfo& client)
{
    if (!m_store) return;

    m_store->upsertClient(client);
    m_connectedBindingIds.insert(client.id.toBindingId());

    RegisterAckPayload ack;
    ack.protocolVersion = BRIDGE_PROTOCOL_VERSION;
    ack.accepted = true;
    ack.providerSpecs = BrowserSessionBridgeCatalog::specs();

    m_server->sendRegisterAck(ack, client.id.toBindingId());

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
    if (!m_store) return;

    const auto spec = BrowserSessionBridgeCatalog::specForProvider(payload.providerId);
    if (!spec.has_value()) return;

    BridgeSessionMaterial material;
    material.providerId = payload.providerId;
    material.clientId = clientId;
    material.cookies = payload.cookies;
    material.localStorage = payload.localStorage;
    material.capturedAtUtc = payload.capturedAtUtc;
    material.sourceReason = QStringLiteral("manual_request");

    m_store->saveImportedMaterial(material);
    enqueueDebounceRefresh(payload.providerId);
}

void BrowserSessionBridgeService::onSessionDirtyReceived(const SessionDirtyPayload& payload)
{
    if (!m_store) return;

    for (const auto& providerId : payload.providerIds) {
        const auto binding = m_store->metadataStore().bindingForProvider(providerId);
        if (binding.has_value() && binding->autoSync) {
            enqueueDebounceRefresh(providerId);
        }
    }
}

void BrowserSessionBridgeService::onServerError(const QString& error)
{
    qWarning() << "[BrowserSessionBridgeServer]" << error;
}

void BrowserSessionBridgeService::onDebounceRefresh()
{
    const auto providers = m_pendingRefreshProviders;
    m_pendingRefreshProviders.clear();
    for (const auto& providerId : providers) {
        emit providerSessionImported(providerId);
    }
}

void BrowserSessionBridgeService::enqueueDebounceRefresh(const QString& providerId)
{
    m_pendingRefreshProviders.insert(providerId);
    m_debounceTimer.start();
}

QString BrowserSessionBridgeService::resolveTargetBindingId(const QString& providerId,
                                                             const QString& preferredBindingId) const
{
    if (!preferredBindingId.isEmpty() && m_connectedBindingIds.contains(preferredBindingId)) {
        return preferredBindingId;
    }

    const auto binding = m_store->metadataStore().bindingForProvider(providerId);
    if (binding.has_value() && !binding->preferredBindingId.isEmpty()
        && m_connectedBindingIds.contains(binding->preferredBindingId)) {
        return binding->preferredBindingId;
    }

    for (const auto& id : m_connectedBindingIds) {
        if (m_store->availableBindingIds(providerId).contains(id)) {
            return id;
        }
    }

    return QString();
}

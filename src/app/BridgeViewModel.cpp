#include "BridgeViewModel.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

namespace {

QVariantMap bindingOptionToMap(const BridgeBindingOption& option)
{
    QVariantMap map;
    map[QStringLiteral("bindingId")] = option.bindingId;
    map[QStringLiteral("label")] = option.label;
    map[QStringLiteral("browserFamily")] = option.browserFamily;
    map[QStringLiteral("connected")] = option.connected;
    map[QStringLiteral("hasMaterial")] = option.hasMaterial;
    return map;
}

} // namespace

BridgeViewModel::BridgeViewModel(BrowserSessionBridgeService* service,
                                 BrowserSessionBridgeStore* store,
                                 QObject* parent)
    : QObject(parent)
    , m_service(service)
    , m_store(store)
{
    Q_UNUSED(m_store)

    connect(m_service, &BrowserSessionBridgeService::clientConnectionStateChanged,
            this, &BridgeViewModel::onClientConnectionStateChanged);
    connect(m_service, &BrowserSessionBridgeService::serverStateChanged,
            this, &BridgeViewModel::onServerStateChanged);
    connect(m_service, &BrowserSessionBridgeService::providerSessionImported,
            this, &BridgeViewModel::onProviderSessionImported);
    connect(m_service, &BrowserSessionBridgeService::providerImportCompleted,
            this, &BridgeViewModel::onProviderImportCompleted);
    connect(m_service, &BrowserSessionBridgeService::providerBindingChanged,
            this, &BridgeViewModel::providerBindingChanged);
    connect(m_service, &BrowserSessionBridgeService::installGuideSeenChanged,
            this, &BridgeViewModel::installGuideSeenChanged);
    connect(m_service, &BrowserSessionBridgeService::extensionStateChanged,
            this, &BridgeViewModel::extensionInstalledChanged);
    connect(m_service, &BrowserSessionBridgeService::lastErrorChanged,
            this, &BridgeViewModel::lastErrorChanged);

    refreshConnectedClients();
}

bool BridgeViewModel::serverRunning() const
{
    return m_service->isServerRunning();
}

int BridgeViewModel::serverPort() const
{
    return m_service->serverPort();
}

QVariantList BridgeViewModel::connectedClients() const
{
    return m_connectedClients;
}

QString BridgeViewModel::extensionInstallPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + QStringLiteral("/browser-session-bridge/extension");
}

bool BridgeViewModel::extensionInstalled() const
{
    return m_service->extensionExported();
}

bool BridgeViewModel::extensionPreparing() const
{
    return m_service->extensionPreparing();
}

QString BridgeViewModel::lastError() const
{
    return m_service->lastError();
}

bool BridgeViewModel::installGuideSeen() const
{
    return m_service->installGuideSeen();
}

void BridgeViewModel::setInstallGuideSeen(bool seen)
{
    m_service->setInstallGuideSeenAsync(seen);
}

bool BridgeViewModel::isProviderSupported(const QString& providerId) const
{
    return BrowserSessionBridgeCatalog::specForProvider(providerId).has_value();
}

QStringList BridgeViewModel::supportedProviders() const
{
    QStringList result;
    const auto specs = BrowserSessionBridgeCatalog::specs();
    for (const auto& spec : specs) {
        result.append(spec.providerId);
    }
    return result;
}

QVariantMap BridgeViewModel::bindingForProvider(const QString& providerId) const
{
    const auto binding = m_service->bindingForProvider(providerId);
    if (!binding.has_value()) return {};

    QVariantMap map;
    map[QStringLiteral("preferredBindingId")] = binding->preferredBindingId;
    map[QStringLiteral("autoSync")] = binding->autoSync;
    map[QStringLiteral("lastImportedAtUtc")] = binding->lastImportedAtUtc.toString(Qt::ISODate);
    map[QStringLiteral("lastImportTimeRelative")] = formatRelativeTime(binding->lastImportedAtUtc);
    return map;
}

QVariantList BridgeViewModel::bindingOptions(const QString& providerId) const
{
    QVariantList list;
    for (const auto& option : m_service->bindingOptions(providerId)) {
        list.append(bindingOptionToMap(option));
    }
    return list;
}

QStringList BridgeViewModel::availableBindings(const QString& providerId) const
{
    QStringList ids;
    for (const auto& option : m_service->bindingOptions(providerId)) {
        if (option.hasMaterial) ids.append(option.bindingId);
    }
    return ids;
}

bool BridgeViewModel::autoSync(const QString& providerId) const
{
    return m_service->autoSyncForProvider(providerId);
}

QString BridgeViewModel::lastImportTime(const QString& providerId) const
{
    const auto binding = m_service->bindingForProvider(providerId);
    if (!binding.has_value() || !binding->lastImportedAtUtc.isValid()) {
        return QString();
    }
    return formatRelativeTime(binding->lastImportedAtUtc);
}

bool BridgeViewModel::importBusy(const QString& providerId) const
{
    return m_importBusyProviders.contains(providerId);
}

QString BridgeViewModel::importError(const QString& providerId) const
{
    return m_importErrors.value(providerId);
}

void BridgeViewModel::prepareExtension()
{
    m_service->prepareExtension();
}

void BridgeViewModel::requestImport(const QString& providerId)
{
    if (m_importBusyProviders.contains(providerId)) return;
    if (m_importErrors.contains(providerId)) {
        m_importErrors.remove(providerId);
        emit importFeedbackChanged(providerId);
    }

    const int generation = m_importGenerations.value(providerId, 0) + 1;
    m_importGenerations[providerId] = generation;

    m_importBusyProviders.insert(providerId);
    emit importBusyChanged(providerId);

    if (!m_service->requestImport(providerId)) {
        m_importGenerations[providerId] = generation + 1;
        m_importBusyProviders.remove(providerId);
        m_importErrors[providerId] = m_service->lastError();
        emit importBusyChanged(providerId);
        emit importFeedbackChanged(providerId);
        return;
    }

    QTimer::singleShot(30000, this, [this, providerId, generation]() {
        if (m_importGenerations.value(providerId) != generation) return;
        if (!m_importBusyProviders.contains(providerId)) return;
        m_importBusyProviders.remove(providerId);
        m_importErrors[providerId] = QStringLiteral("Import timed out. Check that the browser extension is still connected, then try again.");
        m_importGenerations[providerId] = generation + 1;
        emit importBusyChanged(providerId);
        emit importFeedbackChanged(providerId);
    });
}

void BridgeViewModel::setBindingForProvider(const QString& providerId, const QString& bindingId)
{
    m_service->setProviderBindingAsync(providerId, bindingId);
}

void BridgeViewModel::setAutoSync(const QString& providerId, bool enabled)
{
    m_service->setAutoSyncAsync(providerId, enabled);
}

void BridgeViewModel::openExtensionFolder() const
{
    const QString path = extensionInstallPath();
    if (!path.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void BridgeViewModel::copyExtensionPath() const
{
    const QString path = extensionInstallPath();
    if (!path.isEmpty()) {
        QApplication::clipboard()->setText(QDir::toNativeSeparators(path));
    }
}

void BridgeViewModel::openChromeExtensionsPage() const
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("chrome://extensions")));
}

void BridgeViewModel::openEdgeExtensionsPage() const
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("edge://extensions")));
}

void BridgeViewModel::onClientConnectionStateChanged()
{
    refreshConnectedClients();
    emit connectedClientsChanged();
}

void BridgeViewModel::onServerStateChanged()
{
    emit serverRunningChanged();
}

void BridgeViewModel::onProviderSessionImported(const QString& providerId)
{
    if (m_importBusyProviders.remove(providerId)) {
        emit importBusyChanged(providerId);
    }
    emit providerBindingChanged(providerId);
}

void BridgeViewModel::onProviderImportCompleted(const QString& providerId, bool success, const QString& message)
{
    m_importGenerations[providerId] = m_importGenerations.value(providerId, 0) + 1;
    if (m_importBusyProviders.remove(providerId)) {
        emit importBusyChanged(providerId);
    }
    if (success) {
        if (m_importErrors.contains(providerId)) {
            m_importErrors.remove(providerId);
            emit importFeedbackChanged(providerId);
        }
        return;
    }

    m_importErrors[providerId] = message.isEmpty() ? m_service->lastError() : message;
    emit importFeedbackChanged(providerId);
}

void BridgeViewModel::refreshConnectedClients()
{
    m_connectedClients.clear();

    for (const auto& providerId : supportedProviders()) {
        Q_UNUSED(providerId)
    }

    const auto connectedIds = m_service->connectedClientBindingIds();
    const QSet<QString> connectedSet(connectedIds.begin(), connectedIds.end());
    QSet<QString> emitted;

    for (const auto& providerId : supportedProviders()) {
        for (const auto& option : m_service->bindingOptions(providerId)) {
            if (!connectedSet.contains(option.bindingId) || emitted.contains(option.bindingId)) continue;
            QVariantMap map = bindingOptionToMap(option);
            map[QStringLiteral("connectedAt")] = QString();
            map[QStringLiteral("lastSeenAt")] = QString();
            map[QStringLiteral("lastSeenRelative")] = QStringLiteral("connected");
            m_connectedClients.append(map);
            emitted.insert(option.bindingId);
        }
    }
}

QString BridgeViewModel::formatRelativeTime(const QDateTime& dateTime)
{
    if (!dateTime.isValid()) return QString();

    const auto now = QDateTime::currentDateTimeUtc();
    const auto secs = dateTime.secsTo(now);

    if (secs < 60) return QStringLiteral("just now");
    if (secs < 3600) return QStringLiteral("%1 min ago").arg(secs / 60);
    if (secs < 86400) return QStringLiteral("%1 hr ago").arg(secs / 3600);
    return QStringLiteral("%1 days ago").arg(secs / 86400);
}

#pragma once

#include "UsageBackendTypes.h"

#include "../providers/IFetchStrategy.h"
#include "../providers/ProviderFetchContext.h"

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QVector>
#include <optional>

class IProvider;

namespace UsageBackendJobs {

struct CredentialCacheInput {
    QString target;
    bool hasValue = false;
    QByteArray value;
    bool missing = false;
};

struct ProviderFetchSettingInput {
    ProviderSettingsDescriptor descriptor;
    QVariant value;
};

struct ProviderFetchCommandInput {
    QString providerId;
    QHash<QString, QString> env;
    QVector<ProviderFetchSettingInput> settingsFields;
    QHash<QString, QVariant> providerSettings;
    QHash<QString, CredentialCacheInput> credentialCache;
    QString codexActiveAccountId;
    QString codexManagedHomePath;
    QString defaultTokenAccountId;

    // Bridge session data (pre-resolved on main thread for worker consumption)
    std::optional<QString> bridgeCookieHeader;
    std::optional<ImportedBrowserSession> bridgeImportedSession;
};

struct CredentialPreloadItem {
    QString providerId;
    QString key;
    QString target;
};

ProviderRefreshPayload refreshProvider(IProvider* provider,
                                       const ProviderFetchCommandInput& input);
ProviderConnectionTestPayload testProviderConnection(IProvider* provider,
                                                     const ProviderFetchCommandInput& input,
                                                     qint64 startedAt);
CredentialPreloadPayload preloadCredentials(const QVector<CredentialPreloadItem>& items);

} // namespace UsageBackendJobs

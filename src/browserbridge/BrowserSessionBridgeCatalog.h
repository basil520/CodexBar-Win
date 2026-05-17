#pragma once

#include "BrowserSessionBridgeTypes.h"

#include <QVector>
#include <optional>

struct BridgeProviderSpec {
    QString providerId;
    BridgeMaterialKind materialKind = BridgeMaterialKind::Cookies;
    QStringList domains;
    QStringList cookieNames;        // empty means all cookies for those domains
    QString localStorageOrigin;
    QStringList localStorageKeys;
    bool supportsAutoSync = true;
};

class BrowserSessionBridgeCatalog {
public:
    static QVector<BridgeProviderSpec> specs();
    static std::optional<BridgeProviderSpec> specForProvider(const QString& providerId);
};

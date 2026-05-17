#pragma once

#include "../IProvider.h"
#include "../IFetchStrategy.h"
#include "../ProviderFetchContext.h"
#include "../ProviderFetchResult.h"

#include <QObject>
#include <QString>
#include <QJsonObject>

class QianFanProvider : public IProvider {
    Q_OBJECT
public:
    explicit QianFanProvider(QObject* parent = nullptr);

    QString id() const override { return "qianfan"; }
    QString displayName() const override { return "QianFan"; }
    QString sessionLabel() const override { return "5h Usage"; }
    QString weeklyLabel() const override { return "Weekly"; }
    QString opusLabel() const override { return "Monthly"; }
    bool supportsCredits() const override { return false; }
    bool defaultEnabled() const override { return false; }

    QVector<IFetchStrategy*> createStrategies(const ProviderFetchContext& ctx) override;

    QVector<ProviderSettingsDescriptor> settingsDescriptors() const override {
        return {
            {"manualCookieHeader", "Cookie header", "secret", QVariant(),
             {}, "com.codexbarx.cookie.qianfan", {},
             "BDUSS=...; STOKEN=...",
             "Session cookie from Baidu Cloud Console", true, true}
        };
    }

    QString brandColor() const override { return "#2932E1"; }
    QString dashboardURL() const override {
        return "https://console.bce.baidu.com/qianfan/overview";
    }
    QVector<QString> supportedSourceModes() const override { return {"web"}; }
    bool supportsMultipleAccounts() const override { return true; }
    QVector<QString> requiredCredentialTypes() const override { return {"cookie"}; }
};

class QianFanWebStrategy : public IFetchStrategy {
    Q_OBJECT
public:
    explicit QianFanWebStrategy(QObject* parent = nullptr);

    QString id() const override { return "qianfan.web"; }
    int kind() const override { return ProviderFetchKind::Web; }
    bool isAvailable(const ProviderFetchContext& ctx) const override;
    ProviderFetchResult fetchSync(const ProviderFetchContext& ctx) override;
    bool shouldFallback(const ProviderFetchResult& result,
                        const ProviderFetchContext& ctx) const override;

    static ProviderFetchResult parseResponse(const QJsonObject& json);

private:
    static QString resolveCookieHeader(const ProviderFetchContext& ctx);
};

#pragma once

#include "../IProvider.h"
#include "../IFetchStrategy.h"
#include "../ProviderFetchContext.h"
#include "../ProviderFetchResult.h"

#include <QObject>
#include <QString>
#include <QJsonObject>

class XFXinChenProvider : public IProvider {
    Q_OBJECT
public:
    explicit XFXinChenProvider(QObject* parent = nullptr);

    QString id() const override { return "xfxinchen"; }
    QString displayName() const override { return "XFXinChen"; }
    QString sessionLabel() const override { return "5h Usage"; }
    QString weeklyLabel() const override { return "Weekly"; }
    QString opusLabel() const override { return "Package"; }
    bool supportsCredits() const override { return false; }
    bool defaultEnabled() const override { return false; }

    QVector<IFetchStrategy*> createStrategies(const ProviderFetchContext& ctx) override;

    QVector<ProviderSettingsDescriptor> settingsDescriptors() const override {
        return {
            {"manualCookieHeader", "Cookie header", "secret", QVariant(),
             {}, "com.codexbarx.cookie.xfxinchen", "XFYUN_COOKIE",
             "Cookie from maas.xfyun.cn",
             "Paste session cookie from xfyun.cn", true, true}
        };
    }

    QString brandColor() const override { return "#0066FF"; }
    QString dashboardURL() const override { return "https://maas.xfyun.cn"; }
    QVector<QString> supportedSourceModes() const override { return {"web"}; }
};

class XFXinChenWebStrategy : public IFetchStrategy {
    Q_OBJECT
public:
    explicit XFXinChenWebStrategy(QObject* parent = nullptr);

    QString id() const override { return "xfxinchen.web"; }
    int kind() const override { return ProviderFetchKind::Web; }
    bool isAvailable(const ProviderFetchContext& ctx) const override;
    ProviderFetchResult fetchSync(const ProviderFetchContext& ctx) override;
    bool shouldFallback(const ProviderFetchResult& result,
                        const ProviderFetchContext& ctx) const override;

    static ProviderFetchResult parseResponse(const QJsonObject& json);

private:
    static QString resolveCookieHeader(const ProviderFetchContext& ctx);
};
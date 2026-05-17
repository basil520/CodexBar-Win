#include "QianFanProvider.h"
#include "../../network/NetworkManager.h"
#include "../../providers/shared/CookieImporter.h"
#include "../../providers/shared/ProviderCredentialStore.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QLocale>

namespace {

QString countText(int value)
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(value);
}

std::optional<RateWindow> parseQuotaWindow(const QJsonObject& obj)
{
    const int used = obj.value(QStringLiteral("used")).toInt(-1);
    const int limit = obj.value(QStringLiteral("limit")).toInt(0);
    if (limit <= 0 || used < 0) {
        return std::nullopt;
    }

    RateWindow window;
    window.usedPercent = used * 100.0 / limit;

    const QString resetAtStr = obj.value(QStringLiteral("resetAt")).toString();
    if (!resetAtStr.isEmpty()) {
        QDateTime resetsAt = QDateTime::fromString(resetAtStr, Qt::ISODate);
        if (resetsAt.isValid()) {
            window.resetsAt = resetsAt;
        }
    }

    window.resetDescription = QStringLiteral("%1 / %2 Requests")
        .arg(countText(used)).arg(countText(limit));

    return window;
}

} // namespace

QianFanProvider::QianFanProvider(QObject* parent) : IProvider(parent) {}

QVector<IFetchStrategy*> QianFanProvider::createStrategies(const ProviderFetchContext& ctx) {
    Q_UNUSED(ctx)
    return { new QianFanWebStrategy(this) };
}

QianFanWebStrategy::QianFanWebStrategy(QObject* parent) : IFetchStrategy(parent) {}

QString QianFanWebStrategy::resolveCookieHeader(const ProviderFetchContext& ctx) {
    if (ctx.manualCookieHeader.has_value() && !ctx.manualCookieHeader->isEmpty()) {
        return *ctx.manualCookieHeader;
    }
    if (ctx.accountCredentials.web.has_value() && !ctx.accountCredentials.web->cookieValue.toString().isEmpty()) {
        return ctx.accountCredentials.web->cookieValue.toString();
    }
    if (ctx.importedBrowserSession.has_value() &&
        ctx.importedBrowserSession->providerId == QLatin1String("qianfan") &&
        !ctx.importedBrowserSession->cookieHeader.trimmed().isEmpty()) {
        return ctx.importedBrowserSession->cookieHeader.trimmed();
    }
    QStringList domains = {"console.bce.baidu.com", "bce.baidu.com", "login.bce.baidu.com"};
    for (auto browser : CookieImporter::importOrder()) {
        if (!CookieImporter::isBrowserInstalled(browser)) continue;
        auto cookies = CookieImporter::importCookies(browser, domains);
        if (cookies.isEmpty()) continue;
        QStringList parts;
        for (const auto& c : cookies) parts.append(c.name() + "=" + c.value());
        return parts.join("; ");
    }
    return {};
}

bool QianFanWebStrategy::isAvailable(const ProviderFetchContext& ctx) const {
    return !resolveCookieHeader(ctx).isEmpty();
}

bool QianFanWebStrategy::shouldFallback(const ProviderFetchResult& result,
                                         const ProviderFetchContext& ctx) const {
    Q_UNUSED(ctx)
    return !result.success;
}

ProviderFetchResult QianFanWebStrategy::fetchSync(const ProviderFetchContext& ctx) {
    ProviderFetchResult result;
    result.strategyID = id();
    result.strategyKind = kind();
    result.sourceLabel = "web";

    QString cookieHeader = resolveCookieHeader(ctx);
    if (cookieHeader.isEmpty()) {
        result.success = false;
        result.errorMessage = "QianFan session cookie not configured.";
        return result;
    }

    QHash<QString, QString> headers;
    headers["Cookie"] = cookieHeader;
    headers["Accept"] = "application/json";
    headers["Accept-Language"] = "zh-CN,zh;q=0.9,en;q=0.8";
    headers["Referer"] = "https://console.bce.baidu.com/";
    headers["User-Agent"] =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36";

    const QUrl url("https://console.bce.baidu.com/api/qianfan/charge/codingPlan/resourceList");
    auto [json, rawData, httpStatus, respHeaders] = NetworkManager::instance().getJsonSyncWithHeaders(
        url, headers, ctx.networkTimeoutMs);

    if (json.isEmpty()) {
        const QString contentType = respHeaders.value(QStringLiteral("Content-Type"));
        const QString preview = QString::fromUtf8(rawData).simplified().left(200);
        if (httpStatus == 0) {
            result.errorMessage = QStringLiteral("Network error (HTTP 0). Raw: %1").arg(preview);
        } else if (httpStatus == 302 || httpStatus == 401) {
            result.errorMessage = QStringLiteral("Auth failed (HTTP %1). Raw: %2")
                .arg(httpStatus).arg(preview);
        } else if (contentType.contains(QStringLiteral("text/html"), Qt::CaseInsensitive)) {
            result.errorMessage = QStringLiteral("Session expired (HTTP %1, HTML login page). Raw: %2")
                .arg(httpStatus).arg(preview);
        } else {
            result.errorMessage = QStringLiteral("HTTP %1, invalid JSON. Raw: %2")
                .arg(httpStatus).arg(preview);
        }
        result.success = false;
        return result;
    }

    return parseResponse(json);
}

ProviderFetchResult QianFanWebStrategy::parseResponse(const QJsonObject& json) {
    ProviderFetchResult result;
    result.strategyID = "qianfan.web";
    result.strategyKind = ProviderFetchKind::Web;
    result.sourceLabel = "web";

    if (json.isEmpty()) {
        result.success = false;
        result.errorMessage = "Empty response from QianFan API";
        return result;
    }

    if (!json.value(QStringLiteral("success")).toBool(true)) {
        result.success = false;
        result.errorMessage = json.value(QStringLiteral("message")).toString(
            QStringLiteral("QianFan API returned success=false"));
        return result;
    }

    const QJsonObject resultObj = json.value(QStringLiteral("result")).toObject();
    const QJsonArray items = resultObj.value(QStringLiteral("items")).toArray();
    if (items.isEmpty()) {
        result.success = false;
        result.errorMessage = "No coding plan resources found";
        return result;
    }

    const QJsonObject* runningItem = nullptr;
    for (const QJsonValue& itemVal : items) {
        const QJsonObject item = itemVal.toObject();
        if (item.value(QStringLiteral("resourceStatus")).toString() == QStringLiteral("Running")) {
            runningItem = &item;
            break;
        }
    }
    if (!runningItem) {
        result.success = false;
        result.errorMessage = "No running coding plan resource found";
        return result;
    }

    const QJsonObject quota = runningItem->value(QStringLiteral("quota")).toObject();
    const QString planType = runningItem->value(QStringLiteral("planType")).toString();

    UsageSnapshot snap;
    snap.updatedAt = QDateTime::currentDateTime();

    ProviderIdentitySnapshot identity;
    identity.providerID = UsageProvider::qianfan;
    identity.loginMethod = planType;
    snap.identity = identity;

    auto fiveHour = parseQuotaWindow(quota.value(QStringLiteral("fiveHour")).toObject());
    if (fiveHour.has_value()) {
        snap.primary = fiveHour;
    }

    auto week = parseQuotaWindow(quota.value(QStringLiteral("week")).toObject());
    if (week.has_value()) {
        snap.secondary = week;
    }

    auto month = parseQuotaWindow(quota.value(QStringLiteral("month")).toObject());
    if (month.has_value()) {
        snap.tertiary = month;
    }

    result.usage = snap;
    result.success = true;
    return result;
}

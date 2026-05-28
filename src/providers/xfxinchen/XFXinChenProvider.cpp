#include "XFXinChenProvider.h"
#include "../../network/NetworkManager.h"
#include "../../providers/shared/CookieImporter.h"

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

std::optional<RateWindow> parseQuotaWindow(int used, int limit)
{
    if (limit <= 0 || used < 0) return std::nullopt;

    RateWindow rw;
    rw.usedPercent = used * 100.0 / limit;
    rw.resetDescription = QStringLiteral("%1 / %2 Requests")
        .arg(countText(used)).arg(countText(limit));
    return rw;
}

} // namespace

XFXinChenProvider::XFXinChenProvider(QObject* parent) : IProvider(parent) {}

QVector<IFetchStrategy*> XFXinChenProvider::createStrategies(const ProviderFetchContext& ctx) {
    Q_UNUSED(ctx)
    return { new XFXinChenWebStrategy(this) };
}

XFXinChenWebStrategy::XFXinChenWebStrategy(QObject* parent) : IFetchStrategy(parent) {}

QString XFXinChenWebStrategy::resolveCookieHeader(const ProviderFetchContext& ctx) {
    // 1. Manual cookie header from settings
    if (ctx.manualCookieHeader.has_value() && !ctx.manualCookieHeader->isEmpty()) {
        return *ctx.manualCookieHeader;
    }

    // 2. Environment variable
    for (const auto& key : {"XFYUN_COOKIE", "XFYUN_AUTH_COOKIE"}) {
        if (ctx.env.contains(key)) {
            QString val = ctx.env[key];
            if (!val.trimmed().isEmpty()) return val.trimmed();
        }
    }

    // 3. Browser Session Bridge import
    if (ctx.importedBrowserSession.has_value() &&
        ctx.importedBrowserSession->providerId == QLatin1String("xfxinchen") &&
        !ctx.importedBrowserSession->cookieHeader.trimmed().isEmpty()) {
        return ctx.importedBrowserSession->cookieHeader.trimmed();
    }

    // 4. Legacy browser cookie import
    if (ctx.disableLegacyCookieImport) return {};
    QStringList domains = {"maas.xfyun.cn", "passport.xfyun.cn", "login.xfyun.cn"};
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

bool XFXinChenWebStrategy::isAvailable(const ProviderFetchContext& ctx) const {
    return !resolveCookieHeader(ctx).isEmpty();
}

bool XFXinChenWebStrategy::shouldFallback(const ProviderFetchResult& result,
                                           const ProviderFetchContext& ctx) const {
    Q_UNUSED(ctx)
    return !result.success;
}

ProviderFetchResult XFXinChenWebStrategy::fetchSync(const ProviderFetchContext& ctx) {
    ProviderFetchResult result;
    result.strategyID = id();
    result.strategyKind = kind();
    result.sourceLabel = "web";

    QString cookieHeader = resolveCookieHeader(ctx);
    if (cookieHeader.isEmpty()) {
        result.success = false;
        result.errorMessage = "XFXinChen session cookie not configured.";
        return result;
    }

    QHash<QString, QString> headers;
    headers["Cookie"] = cookieHeader;
    headers["Accept"] = "application/json";
    headers["Accept-Language"] = "zh-CN,zh;q=0.9,en;q=0.8";
    headers["Referer"] = "https://maas.xfyun.cn/";
    headers["User-Agent"] =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36";

    const QUrl url("https://maas.xfyun.cn/api/v1/gpt-finetune/coding-plan/list");
    auto [json, rawData, httpStatus, respHeaders] = NetworkManager::instance().getJsonSyncWithHeaders(
        url, headers, ctx.networkTimeoutMs);

    if (json.isEmpty()) {
        const QString contentType = respHeaders.value(QStringLiteral("Content-Type"));
        const QString preview = QString::fromUtf8(rawData).simplified().left(200);
        if (httpStatus == 0) {
            result.errorMessage = QStringLiteral("Network error (HTTP 0). Raw: %1").arg(preview);
        } else if (httpStatus == 401 || httpStatus == 403) {
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

ProviderFetchResult XFXinChenWebStrategy::parseResponse(const QJsonObject& json) {
    ProviderFetchResult result;
    result.strategyID = "xfxinchen.web";
    result.strategyKind = ProviderFetchKind::Web;
    result.sourceLabel = "web";

    if (json.isEmpty()) {
        result.success = false;
        result.errorMessage = "Empty response from XFXinChen API";
        return result;
    }

    if (!json.value(QStringLiteral("succeed")).toBool(true)) {
        result.success = false;
        const QString msg = json.value(QStringLiteral("messageEn")).toString();
        result.errorMessage = msg.isEmpty()
            ? json.value(QStringLiteral("message")).toString(QStringLiteral("XFXinChen API returned succeed=false"))
            : msg;
        return result;
    }

    const QJsonObject dataObj = json.value(QStringLiteral("data")).toObject();
    const QJsonArray rows = dataObj.value(QStringLiteral("rows")).toArray();
    if (rows.isEmpty()) {
        result.success = false;
        result.errorMessage = "No coding plan found";
        return result;
    }

    QJsonObject activeRow;
    for (const QJsonValue& rowVal : rows) {
        const QJsonObject row = rowVal.toObject();
        if (row.value(QStringLiteral("status")).toInt() == 1) {
            activeRow = row;
            break;
        }
    }
    if (activeRow.isEmpty()) {
        result.success = false;
        result.errorMessage = "No active coding plan found";
        return result;
    }

    const QJsonObject usageDTO = activeRow.value(QStringLiteral("codingPlanUsageDTO")).toObject();
    const QString planName = activeRow.value(QStringLiteral("name")).toString();

    UsageSnapshot snap;
    snap.updatedAt = QDateTime::currentDateTime();

    // Primary = 5h rate window
    auto fiveHour = parseQuotaWindow(
        usageDTO.value(QStringLiteral("rp5hUsage")).toInt(0),
        usageDTO.value(QStringLiteral("rp5hLimit")).toInt(0));
    if (fiveHour.has_value()) {
        snap.primary = fiveHour;
    }

    // Secondary = weekly
    auto weekly = parseQuotaWindow(
        usageDTO.value(QStringLiteral("rpwUsage")).toInt(0),
        usageDTO.value(QStringLiteral("rpwLimit")).toInt(0));
    if (weekly.has_value()) {
        snap.secondary = weekly;
    }

    // Tertiary = package total, resetsAt = expiresAt
    auto package = parseQuotaWindow(
        usageDTO.value(QStringLiteral("packageUsage")).toInt(0),
        usageDTO.value(QStringLiteral("packageLimit")).toInt(0));
    if (package.has_value()) {
        const QString expiresAtStr = activeRow.value(QStringLiteral("expiresAt")).toString();
        if (!expiresAtStr.isEmpty()) {
            QDateTime expiresAt = QDateTime::fromString(expiresAtStr, Qt::ISODate);
            if (!expiresAt.isValid()) {
                // Try "yyyy-MM-dd HH:mm:ss" format (xfyun uses this)
                expiresAt = QDateTime::fromString(expiresAtStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            }
            if (expiresAt.isValid()) {
                package->resetsAt = expiresAt;
            }
        }
        snap.tertiary = package;
    }

    ProviderIdentitySnapshot identity;
    identity.providerID = UsageProvider::xfxinchen;
    identity.loginMethod = planName;
    snap.identity = identity;

    result.usage = snap;
    result.success = snap.primary.has_value() || snap.secondary.has_value() || snap.tertiary.has_value();
    if (!result.success) {
        result.errorMessage = "No usage data found in coding plan";
    }
    return result;
}

#include "MiMoProvider.h"
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

std::optional<int> apiCode(const QJsonObject& json)
{
    if (!json.contains(QStringLiteral("code"))) {
        return std::nullopt;
    }
    return json.value(QStringLiteral("code")).toInt();
}

QString apiMessage(const QJsonObject& json)
{
    return json.value(QStringLiteral("message")).toString().trimmed();
}

QJsonObject payloadObject(const QJsonObject& json, QString* errorMessage = nullptr)
{
    if (json.isEmpty()) {
        return {};
    }

    if (const auto code = apiCode(json); code.has_value()) {
        if (*code != 0) {
            if (errorMessage) {
                const QString message = apiMessage(json);
                *errorMessage = message.isEmpty()
                    ? QStringLiteral("MiMo API returned code %1").arg(*code)
                    : message;
            }
            return {};
        }
        return json.value(QStringLiteral("data")).toObject();
    }

    return json;
}

std::optional<double> parseNumberValue(const QJsonValue& value)
{
    if (value.isDouble()) {
        return value.toDouble();
    }

    bool ok = false;
    const double parsed = value.toString().trimmed().toDouble(&ok);
    return ok ? std::optional<double>(parsed) : std::nullopt;
}

double numberValue(const QJsonValue& value, double fallback = 0.0)
{
    return parseNumberValue(value).value_or(fallback);
}

int intValue(const QJsonValue& value, int fallback = 0)
{
    if (value.isDouble()) {
        return value.toInt(fallback);
    }

    bool ok = false;
    const int parsed = value.toString().trimmed().toInt(&ok);
    return ok ? parsed : fallback;
}

QString currencyText(double value, const QString& currency)
{
    const QString normalized = currency.trimmed().toUpper();
    QString symbol = normalized;
    if (normalized == QLatin1String("USD")) {
        symbol = QStringLiteral("$");
    } else if (normalized == QLatin1String("CNY")) {
        symbol = QString::fromUtf8("\xC2\xA5");
    } else if (symbol.isEmpty()) {
        symbol = QStringLiteral("$");
    }
    return QStringLiteral("%1%2").arg(symbol).arg(value, 0, 'f', 2);
}

QString countText(int value)
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(value);
}

QString planLabel(const QString& planCode)
{
    QString trimmed = planCode.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }
    trimmed[0] = trimmed[0].toUpper();
    return trimmed;
}

std::optional<QDateTime> parseMiMoDateTime(const QString& value)
{
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty()) {
        return std::nullopt;
    }

    QDateTime parsed = QDateTime::fromString(trimmed, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (!parsed.isValid()) {
        return std::nullopt;
    }
    parsed.setTimeSpec(Qt::UTC);
    return parsed;
}

} // namespace

MiMoProvider::MiMoProvider(QObject* parent) : IProvider(parent) {}

QVector<IFetchStrategy*> MiMoProvider::createStrategies(const ProviderFetchContext& ctx) {
    Q_UNUSED(ctx)
    return { new MiMoWebStrategy(this) };
}

MiMoWebStrategy::MiMoWebStrategy(QObject* parent) : IFetchStrategy(parent) {}

QString MiMoWebStrategy::resolveCookieHeader(const ProviderFetchContext& ctx) {
    if (ctx.manualCookieHeader.has_value() && !ctx.manualCookieHeader->isEmpty()) {
        return *ctx.manualCookieHeader;
    }
    if (ctx.accountCredentials.web.has_value() && !ctx.accountCredentials.web->cookieValue.toString().isEmpty()) {
        return ctx.accountCredentials.web->cookieValue.toString();
    }
    if (ctx.importedBrowserSession.has_value() &&
        ctx.importedBrowserSession->providerId == QLatin1String("mimo") &&
        !ctx.importedBrowserSession->cookieHeader.trimmed().isEmpty()) {
        return ctx.importedBrowserSession->cookieHeader.trimmed();
    }
    QStringList domains = {"xiaomimimo.com", "platform.xiaomimimo.com", "aistudio.xiaomimimo.com"};
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

bool MiMoWebStrategy::isAvailable(const ProviderFetchContext& ctx) const {
    return !resolveCookieHeader(ctx).isEmpty();
}

bool MiMoWebStrategy::shouldFallback(const ProviderFetchResult& result,
                                      const ProviderFetchContext& ctx) const {
    Q_UNUSED(ctx)
    return !result.success;
}

ProviderFetchResult MiMoWebStrategy::fetchSync(const ProviderFetchContext& ctx) {
    ProviderFetchResult result;
    result.strategyID = id();
    result.strategyKind = kind();
    result.sourceLabel = "web";

    QString cookieHeader = resolveCookieHeader(ctx);
    if (cookieHeader.isEmpty()) {
        result.success = false;
        result.errorMessage = "MiMo session cookie not configured.";
        return result;
    }

    QHash<QString, QString> headers;
    headers["Cookie"] = cookieHeader;
    headers["Accept"] = "application/json, text/plain, */*";
    headers["Accept-Language"] = "en-US,en;q=0.9";
    headers["Origin"] = "https://platform.xiaomimimo.com";
    headers["Referer"] = "https://platform.xiaomimimo.com/#/console/balance";
    headers["User-Agent"] =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36";
    headers["x-timeZone"] = "UTC+08:00";

    // 获取余额
    QJsonObject balanceJson = NetworkManager::instance().getJsonSync(
        QUrl("https://platform.xiaomimimo.com/api/v1/balance"), headers, ctx.networkTimeoutMs);

    // 获取计划详情
    QJsonObject planJson = NetworkManager::instance().getJsonSync(
        QUrl("https://platform.xiaomimimo.com/api/v1/tokenPlan/detail"), headers, ctx.networkTimeoutMs);

    // 获取用量
    QJsonObject usageJson = NetworkManager::instance().getJsonSync(
        QUrl("https://platform.xiaomimimo.com/api/v1/tokenPlan/usage"), headers, ctx.networkTimeoutMs);

    return parseResponse(balanceJson, planJson, usageJson);
}

ProviderFetchResult MiMoWebStrategy::parseResponse(const QJsonObject& balanceJson,
                                                    const QJsonObject& planJson,
                                                    const QJsonObject& usageJson) {
    ProviderFetchResult result;
    result.strategyID = "mimo.web";
    result.strategyKind = ProviderFetchKind::Web;
    result.sourceLabel = "web";

    if (balanceJson.isEmpty() && usageJson.isEmpty()) {
        result.success = false;
        result.errorMessage = "Empty response from MiMo API";
        return result;
    }

    QString balanceError;
    const QJsonObject balancePayload = payloadObject(balanceJson, &balanceError);
    if (!balanceError.isEmpty()) {
        result.success = false;
        result.errorMessage = balanceError;
        return result;
    }

    if (!balanceJson.isEmpty() && balancePayload.isEmpty()) {
        result.success = false;
        result.errorMessage = QStringLiteral("Could not parse Xiaomi MiMo balance: missing payload");
        return result;
    }

    const auto parsedBalance = parseNumberValue(balancePayload.value(QStringLiteral("balance")));
    if (!parsedBalance.has_value()) {
        result.success = false;
        result.errorMessage = QStringLiteral("Could not parse Xiaomi MiMo balance: invalid balance value");
        return result;
    }

    const double balance = *parsedBalance;
    QString currency = balancePayload.value(QStringLiteral("currency")).toString(QStringLiteral("CNY")).trimmed();
    if (currency.isEmpty()) {
        currency = QStringLiteral("CNY");
    }

    const QJsonObject planPayload = payloadObject(planJson);
    const QString planCode = planPayload.value(QStringLiteral("planCode")).toString();
    const bool planExpired = planPayload.value(QStringLiteral("expired")).toBool(false);
    const auto periodEnd = parseMiMoDateTime(planPayload.value(QStringLiteral("currentPeriodEnd")).toString());

    const QJsonObject usagePayload = payloadObject(usageJson);
    const QJsonObject monthUsage = usagePayload.value(QStringLiteral("monthUsage")).toObject();
    const QJsonArray items = monthUsage.value(QStringLiteral("items")).toArray();
    const QJsonObject firstUsageItem = items.isEmpty() ? QJsonObject{} : items.first().toObject();

    const int tokenUsed = intValue(firstUsageItem.value(QStringLiteral("used")));
    const int tokenLimit = intValue(firstUsageItem.value(QStringLiteral("limit")));
    double tokenPercent = numberValue(firstUsageItem.value(QStringLiteral("percent")));
    if (tokenPercent <= 1.0) {
        tokenPercent *= 100.0;
    }
    if (tokenPercent <= 0.0 && tokenLimit > 0) {
        tokenPercent = tokenUsed * 100.0 / tokenLimit;
    }
    tokenPercent = std::clamp(tokenPercent, 0.0, 100.0);

    UsageSnapshot snap;
    snap.updatedAt = QDateTime::currentDateTime();

    ProviderIdentitySnapshot identity;
    identity.providerID = UsageProvider::mimo;
    const QString label = planExpired ? QStringLiteral("Expired") : planLabel(planCode);
    identity.loginMethod = label.isEmpty()
        ? QCoreApplication::translate("ProviderLabels", "Balance: %1").arg(currencyText(balance, currency))
        : label;
    snap.identity = identity;

    // 主窗口：Token 使用率
    if (tokenLimit > 0) {
        RateWindow primary;
        primary.usedPercent = tokenPercent;
        if (periodEnd.has_value()) {
            primary.resetsAt = *periodEnd;
        }
        primary.resetDescription = QCoreApplication::translate("ProviderLabels", "%1 / %2 Credits")
            .arg(countText(tokenUsed)).arg(countText(tokenLimit));
        snap.primary = primary;
    }

    result.usage = snap;
    result.success = true;

    return result;
}

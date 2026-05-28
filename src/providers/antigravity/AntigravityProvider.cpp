#include "AntigravityProvider.h"
#include "../../network/NetworkManager.h"

#include <QProcess>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QMutexLocker>
#include <QTcpSocket>

// Cache isAvailable() for 30s to avoid repeated PowerShell calls blocking the thread pool.
static constexpr int ANTIGRAVITY_AVAIL_CACHE_MS = 30000;

AntigravityLocalStrategy::AvailabilityCache AntigravityLocalStrategy::s_availCache;
QMutex AntigravityLocalStrategy::s_availCacheMutex;

AntigravityProvider::AntigravityProvider(QObject* parent) : IProvider(parent) {}

QVector<IFetchStrategy*> AntigravityProvider::createStrategies(const ProviderFetchContext& ctx) {
    Q_UNUSED(ctx)
    return { new AntigravityLocalStrategy(this) };
}

AntigravityLocalStrategy::AntigravityLocalStrategy(QObject* parent) : IFetchStrategy(parent) {}

namespace {

QString antigravityCommandLine()
{
    QProcess proc;
#ifdef Q_OS_WIN
    proc.start(QStringLiteral("powershell"),
               {QStringLiteral("-NoProfile"),
                QStringLiteral("-Command"),
                QStringLiteral("Get-CimInstance Win32_Process | Where-Object {$_.Name -like 'antigravity*'} | Select-Object -First 1 -ExpandProperty CommandLine")});
#else
    proc.start(QStringLiteral("ps"), {QStringLiteral("ax"), QStringLiteral("-o"), QStringLiteral("command=")});
#endif
    proc.waitForFinished(1000);

    const QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
#ifdef Q_OS_WIN
    return output;
#else
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.contains(QStringLiteral("antigravity"), Qt::CaseInsensitive)
            && !trimmed.contains(QStringLiteral("ps ax -o command="))) {
            return trimmed;
        }
    }
    return {};
#endif
}

bool canConnectLocalPort(int port)
{
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, static_cast<quint16>(port));
    return socket.waitForConnected(250);
}

} // namespace

bool AntigravityLocalStrategy::findProcess(int& port, QString& csrfToken) {
    QString cmdline = antigravityCommandLine();

    if (cmdline.isEmpty()) return false;

    // Extract port
    QRegularExpression portRe("--extension_server_port=(\\d+)");
    auto portMatch = portRe.match(cmdline);
    if (portMatch.hasMatch()) {
        port = portMatch.captured(1).toInt();
    } else {
        // Fallback: scan a small set of common ports quickly
        for (int p = 3000; p <= 3004; p++) {
            if (canConnectLocalPort(p)) {
                port = p;
                break;
            }
        }
        if (port <= 0) return false;
    }

    // Extract CSRF token
    QRegularExpression csrfRe("--csrf_token=([a-fA-F0-9]+)");
    auto csrfMatch = csrfRe.match(cmdline);
    if (csrfMatch.hasMatch()) {
        csrfToken = csrfMatch.captured(1);
    }

    return true;
}

bool AntigravityLocalStrategy::isAvailable(const ProviderFetchContext& ctx) const {
    Q_UNUSED(ctx)

    {
        QMutexLocker locker(&s_availCacheMutex);
        if (s_availCache.cachedAt.isValid() &&
            s_availCache.cachedAt.msecsTo(QDateTime::currentDateTime()) < ANTIGRAVITY_AVAIL_CACHE_MS) {
            return s_availCache.available;
        }
    }

    int port = 0;
    QString csrf;
    bool ok = findProcess(port, csrf);

    {
        QMutexLocker locker(&s_availCacheMutex);
        s_availCache.available = ok;
        s_availCache.port = port;
        s_availCache.csrfToken = csrf;
        s_availCache.cachedAt = QDateTime::currentDateTime();
    }
    return ok;
}

bool AntigravityLocalStrategy::shouldFallback(const ProviderFetchResult& result,
                                               const ProviderFetchContext& ctx) const {
    Q_UNUSED(ctx)
    return !result.success;
}

ProviderFetchResult AntigravityLocalStrategy::queryStatus(int port, const QString& csrfToken) {
    ProviderFetchResult result;
    result.strategyID = "antigravity.local";
    result.strategyKind = ProviderFetchKind::LocalProbe;
    result.sourceLabel = "local";

    QHash<QString, QString> headers;
    headers["Content-Type"] = "application/json";
    headers["Connect-Protocol-Version"] = "1";
    if (!csrfToken.isEmpty())
        headers["X-Codeium-Csrf-Token"] = csrfToken;

    QJsonObject body;
    QJsonObject json = NetworkManager::instance().postJsonSync(
        QUrl(QString("http://127.0.0.1:%1/exa.language_server_pb.LanguageServerService/GetUserStatus").arg(port)),
        body, headers, 5000);

    if (json.isEmpty()) {
        result.success = false;
        result.errorMessage = "No response from Antigravity local service";
        return result;
    }

    int code = json["code"].toInt(-1);
    if (code != 0) {
        result.success = false;
        result.errorMessage = QString("Antigravity error code: %1").arg(code);
        return result;
    }

    QJsonObject userStatus = json["userStatus"].toObject();
    QJsonObject cascadeConfig = userStatus["cascadeModelConfigData"].toObject();
    QJsonArray configs = cascadeConfig["clientModelConfigs"].toArray();

    UsageSnapshot snap;
    snap.updatedAt = QDateTime::currentDateTime();

    ProviderIdentitySnapshot identity;
    identity.providerID = UsageProvider::antigravity;
    identity.accountEmail = userStatus["email"].toString();
    snap.identity = identity;

    for (const auto& c : configs) {
        QJsonObject config = c.toObject();
        QString label = config["label"].toString().toLower();
        QJsonObject quotaInfo = config["quotaInfo"].toObject();
        double remaining = quotaInfo["remainingFraction"].toDouble(1.0);
        QString resetTime = quotaInfo["resetTime"].toString();

        RateWindow rw;
        rw.usedPercent = 100.0 - (remaining * 100.0);
        if (!resetTime.isEmpty())
            rw.resetsAt = QDateTime::fromString(resetTime, Qt::ISODate);
        rw.resetDescription = config["label"].toString();

        if (label.contains("claude")) {
            if (!snap.primary.has_value()) snap.primary = rw;
        } else if (label.contains("gemini") && label.contains("flash")) {
            if (!snap.tertiary.has_value()) snap.tertiary = rw;
        } else if (label.contains("gemini")) {
            if (!snap.secondary.has_value()) snap.secondary = rw;
        } else {
            if (!snap.primary.has_value()) snap.primary = rw;
            else if (!snap.secondary.has_value()) snap.secondary = rw;
        }
    }

    result.usage = snap;
    result.success = true;
    return result;
}

ProviderFetchResult AntigravityLocalStrategy::fetchSync(const ProviderFetchContext& ctx) {
    Q_UNUSED(ctx)

    int port = 0;
    QString csrfToken;
    if (!findProcess(port, csrfToken)) {
        ProviderFetchResult result;
        result.success = false;
        result.errorMessage = "Antigravity is not running.";
        return result;
    }

    return queryStatus(port, csrfToken);
}

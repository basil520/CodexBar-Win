#include "ProviderErrorClassifier.h"

#include <QRegularExpression>

ProviderErrorClassifier::ProviderErrorClassifier(QObject* parent)
    : QObject(parent)
{
}

QString ProviderErrorClassifier::normalized(const QString& text)
{
    return text.trimmed().toLower();
}

QString ProviderErrorClassifier::redacted(const QString& text)
{
    QString out = text;
    const QList<QRegularExpression> patterns = {
        QRegularExpression(QStringLiteral(R"((authorization\s*:\s*bearer\s+)[^\s;]+)"),
                           QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(R"(((?:api[_-]?key|token|access[_-]?token|refresh[_-]?token|session(?:id)?|cookie)\s*[:=]\s*)[^\s;,]+)"),
                           QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(R"((cookie\s*:\s*)[^\r\n]+)"),
                           QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(R"((bearer\s+)[A-Za-z0-9._~+/=-]+)"),
                           QRegularExpression::CaseInsensitiveOption),
        QRegularExpression(QStringLiteral(R"(sk-[A-Za-z0-9_-]{6,})"),
                           QRegularExpression::CaseInsensitiveOption),
    };

    for (const QRegularExpression& pattern : patterns) {
        QRegularExpressionMatch match;
        int index = 0;
        while ((index = out.indexOf(pattern, index, &match)) >= 0) {
            const QString replacement = match.lastCapturedIndex() >= 1
                ? match.captured(1) + QStringLiteral("[redacted]")
                : QStringLiteral("[redacted]");
            out.replace(index, match.capturedLength(0), replacement);
            index += replacement.size();
        }
    }
    return out;
}

QVariantMap ProviderErrorClassifier::classify(const QString& providerId, const QString& rawMessage) const
{
    const QString lower = normalized(rawMessage);
    QString category = QStringLiteral("unknown");
    QString severity = QStringLiteral("error");
    QString title = QStringLiteral("Provider error");
    QString summary = rawMessage.trimmed();
    QString actionLabel = QStringLiteral("Retry");
    QString suggestedAction = QStringLiteral("Refresh this provider. If the issue persists, open provider settings and check credentials or network access.");

    if (lower.contains(QStringLiteral("network")) || lower.contains(QStringLiteral("timed out"))
        || lower.contains(QStringLiteral("timeout")) || lower.contains(QStringLiteral("unreachable"))
        || lower.contains(QStringLiteral("connection refused")) || lower.contains(QStringLiteral("http 0"))
        || lower.contains(QStringLiteral("ssl"))) {
        category = QStringLiteral("network");
        title = QStringLiteral("Network error");
        summary = QStringLiteral("The provider could not be reached.");
        suggestedAction = QStringLiteral("Refresh, then check network, proxy, VPN, or the provider status page.");
    } else if (lower.contains(QStringLiteral("401")) || lower.contains(QStringLiteral("403"))
               || lower.contains(QStringLiteral("unauthorized")) || lower.contains(QStringLiteral("forbidden"))
               || lower.contains(QStringLiteral("token expired")) || lower.contains(QStringLiteral("login required"))
               || lower.contains(QStringLiteral("not logged in"))) {
        category = QStringLiteral("auth");
        title = QStringLiteral("Authentication required");
        summary = QStringLiteral("Login or credentials appear to be invalid.");
        actionLabel = QStringLiteral("Open settings");
        suggestedAction = QStringLiteral("Re-authenticate this provider or update its API key/cookies.");
    } else if (lower.contains(QStringLiteral("429")) || lower.contains(QStringLiteral("rate limit"))
               || lower.contains(QStringLiteral("quota")) || lower.contains(QStringLiteral("insufficient credits"))
               || lower.contains(QStringLiteral("exhausted"))) {
        category = QStringLiteral("quota");
        severity = QStringLiteral("warning");
        title = QStringLiteral("Quota or rate limit");
        summary = QStringLiteral("The provider reported quota, credits, or rate limit pressure.");
        actionLabel = QStringLiteral("Check usage");
        suggestedAction = QStringLiteral("Wait for reset, reduce usage, or check billing/credits.");
    } else if (lower.contains(QStringLiteral("binary not found")) || lower.contains(QStringLiteral("not installed"))
               || lower.contains(QStringLiteral("command not found")) || lower.contains(QStringLiteral("cli"))) {
        category = QStringLiteral("cli");
        title = QStringLiteral("CLI unavailable");
        summary = QStringLiteral("The provider CLI is missing or unavailable.");
        actionLabel = QStringLiteral("Open settings");
        suggestedAction = QStringLiteral("Install the provider CLI or update the configured executable path.");
    } else if (lower.contains(QStringLiteral("parse")) || lower.contains(QStringLiteral("invalid balance"))
               || lower.contains(QStringLiteral("could not parse")) || lower.contains(QStringLiteral("invalid value"))) {
        category = QStringLiteral("parse");
        severity = QStringLiteral("warning");
        title = QStringLiteral("Parse error");
        summary = QStringLiteral("CodexBarX could not read the provider response format.");
        actionLabel = QStringLiteral("Copy diagnostics");
        suggestedAction = QStringLiteral("Copy diagnostics and check whether the provider output format changed.");
    } else if (lower.contains(QStringLiteral("status")) || lower.contains(QStringLiteral("incident"))
               || lower.contains(QStringLiteral("degraded")) || lower.contains(QStringLiteral("maintenance"))) {
        category = QStringLiteral("provider_status");
        severity = QStringLiteral("warning");
        title = QStringLiteral("Provider status issue");
        summary = QStringLiteral("The provider may be degraded or under maintenance.");
        suggestedAction = QStringLiteral("Refresh later or check the provider status page.");
    }

    if (rawMessage.trimmed().isEmpty()) {
        summary = QStringLiteral("No error details were provided.");
    }

    const QString safeDetail = redacted(rawMessage);
    const QString safeSummary = redacted(summary);
    const QString safeProvider = providerId.trimmed();
    const QString copy = QStringLiteral("Provider: %1\nCategory: %2\nSeverity: %3\nSummary: %4\nDetails:\n%5")
        .arg(safeProvider.isEmpty() ? QStringLiteral("unknown") : safeProvider,
             category,
             severity,
             safeSummary,
             safeDetail);

    QVariantMap view;
    view.insert(QStringLiteral("providerId"), providerId);
    view.insert(QStringLiteral("title"), title);
    view.insert(QStringLiteral("summary"), safeSummary);
    view.insert(QStringLiteral("detail"), safeDetail);
    view.insert(QStringLiteral("rawMessage"), safeDetail);
    view.insert(QStringLiteral("category"), category);
    view.insert(QStringLiteral("severity"), severity);
    view.insert(QStringLiteral("suggestedAction"), suggestedAction);
    view.insert(QStringLiteral("actionLabel"), actionLabel);
    view.insert(QStringLiteral("source"), QStringLiteral("provider"));
    view.insert(QStringLiteral("httpStatus"), 0);
    view.insert(QStringLiteral("copyText"), copy);
    return view;
}

QString ProviderErrorClassifier::copyText(const QString& providerId, const QString& rawMessage) const
{
    return classify(providerId, rawMessage).value(QStringLiteral("copyText")).toString();
}

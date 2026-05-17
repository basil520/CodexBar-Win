#include <QtTest/QtTest>

#include "../src/providers/ProviderSettingsSnapshot.h"
#include "../src/providers/codex/CodexProvider.h"
#include "../src/providers/codex/CodexDashboardCache.h"

#include <QJsonDocument>
#include <QJsonObject>

class tst_CodexProvider : public QObject {
    Q_OBJECT
private slots:
    void autoModeUsesOAuthThenRpcThenPtyThenWeb() {
        CodexProvider provider;
        ProviderFetchContext ctx;
        ctx.settings.set("sourceMode", "auto");

        QVector<IFetchStrategy*> strategies = provider.createStrategies(ctx);

        QCOMPARE(strategies.size(), 4);
        QCOMPARE(strategies.at(0)->id(), QString("codex.oauth"));
        QCOMPARE(strategies.at(1)->id(), QString("codex.cli.rpc"));
        QCOMPARE(strategies.at(2)->id(), QString("codex.cli.pty"));
        QCOMPARE(strategies.at(3)->id(), QString("codex.web"));
        qDeleteAll(strategies);
    }

    void cliModeUsesRpcThenPtyOnly() {
        CodexProvider provider;
        ProviderFetchContext ctx;
        ctx.settings.set("sourceMode", "cli");

        QVector<IFetchStrategy*> strategies = provider.createStrategies(ctx);

        QCOMPARE(strategies.size(), 2);
        QCOMPARE(strategies.at(0)->id(), QString("codex.cli.rpc"));
        QCOMPARE(strategies.at(1)->id(), QString("codex.cli.pty"));
        qDeleteAll(strategies);
    }

    void mapsRpcRateLimitsAndAccountToUsageSnapshot() {
        QJsonObject rateLimits{
            {"rateLimits", QJsonObject{
                {"primary", QJsonObject{
                    {"usedPercent", 42},
                    {"windowDurationMins", 300},
                    {"resetsAt", 1893456000}
                }},
                {"secondary", QJsonObject{
                    {"usedPercent", 17},
                    {"windowDurationMins", 10080},
                    {"resetsAt", 1894060800}
                }},
                {"credits", QJsonObject{
                    {"hasCredits", true},
                    {"unlimited", false},
                    {"balance", "12.50"}
                }}
            }}
        };
        QJsonObject account{
            {"account", QJsonObject{
                {"type", "chatgpt"},
                {"email", "dev@example.com"},
                {"planType", "pro"}
            }},
            {"requiresOpenaiAuth", false}
        };

        ProviderFetchResult result = CodexAppServerStrategy::mapRpcResult(rateLimits, account);

        QVERIFY(result.success);
        QCOMPARE(result.strategyID, QString("codex.cli.rpc"));
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 42.0);
        QCOMPARE(result.usage.primary->windowMinutes.value(), 300);
        QCOMPARE(result.usage.primary->resetsAt->toSecsSinceEpoch(), qint64(1893456000));
        QVERIFY(result.usage.secondary.has_value());
        QCOMPARE(result.usage.secondary->usedPercent, 17.0);
        QCOMPARE(result.usage.secondary->windowMinutes.value(), 10080);
        QVERIFY(result.credits.has_value());
        QCOMPARE(result.credits->remaining, 12.5);
        QVERIFY(result.usage.providerCost.has_value());
        QCOMPARE(result.usage.providerCost->used, 12.5);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("dev@example.com"));
        QCOMPARE(result.usage.identity->loginMethod.value(), QString("pro"));
    }

    void prefersCodexBucketFromRpcRateLimitsByLimitId() {
        QJsonObject result{
            {"rateLimits", QJsonObject{
                {"primary", QJsonObject{{"usedPercent", 90}, {"windowDurationMins", 300}}}
            }},
            {"rateLimitsByLimitId", QJsonObject{
                {"other", QJsonObject{
                    {"primary", QJsonObject{{"usedPercent", 80}, {"windowDurationMins", 300}}}
                }},
                {"codex", QJsonObject{
                    {"limitId", "codex"},
                    {"primary", QJsonObject{{"usedPercent", 11}, {"windowDurationMins", 300}}},
                    {"secondary", QJsonObject{{"usedPercent", 22}, {"windowDurationMins", 10080}}}
                }}
            }}
        };

        ProviderFetchResult mapped = CodexAppServerStrategy::mapRpcResult(result);

        QVERIFY(mapped.success);
        QVERIFY(mapped.usage.primary.has_value());
        QCOMPARE(mapped.usage.primary->usedPercent, 11.0);
        QVERIFY(mapped.usage.secondary.has_value());
        QCOMPARE(mapped.usage.secondary->usedPercent, 22.0);
    }

    void mapsRpcErrorsToFallbackFriendlyFailure() {
        QJsonObject error{
            {"code", -32603},
            {"message", "body={\"rate_limit\":{\"primary_window\":{\"used_percent\":9,\"limit_window_seconds\":300,\"reset_at\":1893456000}},\"plan_type\":\"plus\",\"email\":\"dev@example.com\"}"}
        };

        ProviderFetchResult result = CodexAppServerStrategy::mapRpcError(error);

        QVERIFY(result.success);
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 9.0);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("dev@example.com"));
        QCOMPARE(result.usage.identity->loginMethod.value(), QString("plus"));
    }

    void mapsWebDashboardBackendUsageJson() {
        QJsonObject json{
            {"rate_limit", QJsonObject{
                {"primary_window", QJsonObject{
                    {"used_percent", 33},
                    {"limit_window_seconds", 18000},
                    {"reset_at", 1893456000}
                }},
                {"secondary_window", QJsonObject{
                    {"used_percent", 44},
                    {"limit_window_seconds", 604800},
                    {"reset_at", 1894060800}
                }}
            }},
            {"credits", QJsonObject{
                {"has_credits", true},
                {"unlimited", false},
                {"balance", "7.25"}
            }},
            {"email", "web@example.com"},
            {"plan_type", "plus"}
        };

        ProviderFetchResult result = CodexWebDashboardStrategy::mapUsageJson(json);

        QVERIFY(result.success);
        QCOMPARE(result.strategyID, QString("codex.web"));
        QCOMPARE(result.sourceLabel, QString("web"));
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 33.0);
        QCOMPARE(result.usage.primary->windowMinutes.value(), 300);
        QVERIFY(result.usage.secondary.has_value());
        QCOMPARE(result.usage.secondary->usedPercent, 44.0);
        QCOMPARE(result.usage.secondary->windowMinutes.value(), 10080);
        QVERIFY(result.credits.has_value());
        QCOMPARE(result.credits->remaining, 7.25);
        QVERIFY(result.usage.providerCost.has_value());
        QCOMPARE(result.usage.providerCost->used, 7.25);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("web@example.com"));
        QCOMPARE(result.usage.identity->loginMethod.value(), QString("plus"));
    }

    void extractsAccessTokenFromWebAuthSessionJson() {
        QJsonObject json{
            {"accessToken", "access-token-123"},
            {"user", QJsonObject{{"email", "WEB@EXAMPLE.COM"}}},
            {"account", QJsonObject{{"id", "account-abc"}}},
            {"plan_type", "pro"}
        };

        auto session = CodexWebDashboardStrategy::mapAuthSessionJson(json);

        QVERIFY(session.has_value());
        QCOMPARE(session->accessToken, QString("access-token-123"));
        QCOMPARE(session->accountEmail, QString("web@example.com"));
        QCOMPARE(session->accountId, QString("account-abc"));
        QCOMPARE(session->planType, QString("pro"));
    }

    void webUsageJsonFallsBackToSessionIdentity() {
        QJsonObject json{
            {"rate_limit", QJsonObject{
                {"primary_window", QJsonObject{
                    {"used_percent", 12},
                    {"limit_window_seconds", 18000},
                    {"reset_at", 1893456000}
                }}
            }}
        };

        ProviderFetchResult result = CodexWebDashboardStrategy::mapUsageJson(
            json, QStringLiteral("session@example.com"), QStringLiteral("plus"));

        QVERIFY(result.success);
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 12.0);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("session@example.com"));
        QCOMPARE(result.usage.identity->loginMethod.value(), QString("plus"));
    }

    void mapsImportedBridgeUsagePayload() {
        QJsonObject usageJson{
            {"rate_limit", QJsonObject{
                {"primary_window", QJsonObject{
                    {"used_percent", 21},
                    {"limit_window_seconds", 18000},
                    {"reset_at", 1893456000}
                }}
            }},
            {"credits", QJsonObject{
                {"has_credits", true},
                {"unlimited", false},
                {"balance", "3.50"}
            }},
            {"email", "bridge@example.com"},
            {"plan_type", "plus"}
        };
        QJsonObject payload{
            {"codex_usage_json",
             QString::fromUtf8(QJsonDocument(usageJson).toJson(QJsonDocument::Compact))}
        };

        ProviderFetchResult result = CodexWebDashboardStrategy::mapImportedSessionPayload(
            QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)));

        QVERIFY(result.success);
        QCOMPARE(result.strategyID, QString("codex.web"));
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 21.0);
        QVERIFY(result.credits.has_value());
        QCOMPARE(result.credits->remaining, 3.5);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("bridge@example.com"));
        QCOMPARE(result.usage.identity->loginMethod.value(), QString("plus"));
    }

    void fetchSyncUsesImportedBridgePayloadWithoutCookieHeader() {
        QJsonObject usageJson{
            {"rate_limit", QJsonObject{
                {"primary_window", QJsonObject{
                    {"used_percent", 18},
                    {"limit_window_seconds", 18000},
                    {"reset_at", 1893456000}
                }}
            }},
            {"email", "bridge-only@example.com"},
            {"plan_type", "plus"}
        };
        QJsonObject payload{
            {"codex_usage_json",
             QString::fromUtf8(QJsonDocument(usageJson).toJson(QJsonDocument::Compact))}
        };

        ProviderFetchContext ctx;
        ctx.sourceMode = ProviderSourceMode::Web;
        ImportedBrowserSession session;
        session.providerId = QStringLiteral("codex");
        session.sessionPayload = QString::fromUtf8(
            QJsonDocument(payload).toJson(QJsonDocument::Compact));
        ctx.importedBrowserSession = session;

        CodexWebDashboardStrategy strategy;
        ProviderFetchResult result = strategy.fetchSync(ctx);

        QVERIFY(result.success);
        QVERIFY(result.usage.primary.has_value());
        QCOMPARE(result.usage.primary->usedPercent, 18.0);
        QVERIFY(result.usage.identity.has_value());
        QCOMPARE(result.usage.identity->accountEmail.value(), QString("bridge-only@example.com"));
    }

    void fetchSyncReportsImportedBridgeErrorWithoutHtmlFallback() {
        QJsonObject payload{
            {"codex_usage_error", "codex_usage_http_401 at backend-api/wham/usage"}
        };

        ProviderFetchContext ctx;
        ctx.sourceMode = ProviderSourceMode::Web;
        ImportedBrowserSession session;
        session.providerId = QStringLiteral("codex");
        session.sessionPayload = QString::fromUtf8(
            QJsonDocument(payload).toJson(QJsonDocument::Compact));
        ctx.importedBrowserSession = session;

        CodexWebDashboardStrategy strategy;
        ProviderFetchResult result = strategy.fetchSync(ctx);

        QVERIFY(!result.success);
        QVERIFY(result.errorMessage.contains(QStringLiteral("codex_usage_http_401")));
        QVERIFY(!result.errorMessage.contains(QStringLiteral("Response preview")));
        QVERIFY(!result.errorMessage.contains(QStringLiteral("No cookie header")));
    }

    void codexDashboardCacheClearRemovesFile() {
        // Save a dummy entry
        CodexDashboardCacheEntry entry;
        entry.accountEmail = "test@example.com";
        entry.html = "<html>test</html>";
        entry.updatedAt = QDateTime::currentDateTime();
        CodexDashboardCache::save(entry);

        // Verify saved
        auto loaded = CodexDashboardCache::load("test@example.com");
        QVERIFY(loaded.has_value());
        QCOMPARE(loaded->html, QString("<html>test</html>"));

        // Clear
        CodexDashboardCache::clear();

        // Verify cleared
        auto afterClear = CodexDashboardCache::load("test@example.com");
        QVERIFY(!afterClear.has_value());
    }
};

QTEST_MAIN(tst_CodexProvider)
#include "tst_CodexProvider.moc"

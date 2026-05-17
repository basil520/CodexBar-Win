#include "BrowserSessionBridgeCatalog.h"

QVector<BridgeProviderSpec> BrowserSessionBridgeCatalog::specs()
{
    return {
        // --- Cookie-based providers ---
        {QStringLiteral("cursor"), BridgeMaterialKind::Cookies,
         {QStringLiteral("cursor.com")},
         {QStringLiteral("WorkosCursorSessionToken"),
          QStringLiteral("next-auth.session-token"),
          QStringLiteral("wos-session"),
          QStringLiteral("authjs.session-token"),
          QStringLiteral("__Secure-WorkosCursorSessionToken"),
          QStringLiteral("__Secure-next-auth.session-token"),
          QStringLiteral("__Secure-wos-session"),
          QStringLiteral("__Secure-authjs.session-token")}},

        {QStringLiteral("codex"), BridgeMaterialKind::Hybrid,
         {QStringLiteral("chatgpt.com")},
         {},
         QStringLiteral("https://chatgpt.com"),
         {}},

        {QStringLiteral("claude"), BridgeMaterialKind::Cookies,
         {QStringLiteral("claude.ai")},
         {QStringLiteral("sessionKey")}},

        {QStringLiteral("kimi"), BridgeMaterialKind::Hybrid,
         {QStringLiteral("kimi.com"), QStringLiteral("www.kimi.com"),
          QStringLiteral("auth.kimi.com"), QStringLiteral("kimi.moonshot.cn"),
          QStringLiteral("login.moonshot.cn"), QStringLiteral("login.moonshot.ai")},
         {QStringLiteral("kimi-auth")},
         QStringLiteral("https://www.kimi.com"),
         {QStringLiteral("access_token"), QStringLiteral("refresh_token"),
          QStringLiteral("anonymous_access_token"), QStringLiteral("anonymous_refresh_token"),
          QStringLiteral("volcano-token-info")}},

        {QStringLiteral("opencode"), BridgeMaterialKind::Cookies,
         {QStringLiteral("opencode.ai"), QStringLiteral("app.opencode.ai")},
         {QStringLiteral("auth"), QStringLiteral("__Host-auth")}},

        {QStringLiteral("opencodego"), BridgeMaterialKind::Cookies,
         {QStringLiteral("opencode.ai"), QStringLiteral("app.opencode.ai")},
         {QStringLiteral("auth"), QStringLiteral("__Host-auth")}},

        {QStringLiteral("amp"), BridgeMaterialKind::Cookies,
         {QStringLiteral("ampcode.com"), QStringLiteral("www.ampcode.com")},
         {}},

        {QStringLiteral("abacus"), BridgeMaterialKind::Cookies,
         {QStringLiteral("abacus.ai"), QStringLiteral("apps.abacus.ai")},
         {}},

        {QStringLiteral("augment"), BridgeMaterialKind::Cookies,
         {QStringLiteral("augmentcode.com"), QStringLiteral("app.augmentcode.com")},
         {}},

        {QStringLiteral("factory"), BridgeMaterialKind::Cookies,
         {QStringLiteral("app.factory.ai"), QStringLiteral("factory.ai")},
         {}},

        {QStringLiteral("perplexity"), BridgeMaterialKind::Cookies,
         {QStringLiteral("perplexity.ai"), QStringLiteral("www.perplexity.ai")},
         {}},

        {QStringLiteral("minimax"), BridgeMaterialKind::Cookies,
         {QStringLiteral("platform.minimax.io"), QStringLiteral("platform.minimaxi.com"),
          QStringLiteral("minimax.io"), QStringLiteral("minimaxi.com")},
         {}},

        {QStringLiteral("ollama"), BridgeMaterialKind::Cookies,
         {QStringLiteral("ollama.com"), QStringLiteral("www.ollama.com")},
         {}},

        {QStringLiteral("mistral"), BridgeMaterialKind::Cookies,
         {QStringLiteral("mistral.ai"), QStringLiteral("admin.mistral.ai"),
          QStringLiteral("auth.mistral.ai")},
         {}},

        {QStringLiteral("alibaba"), BridgeMaterialKind::Cookies,
         {QStringLiteral("aliyun.com"), QStringLiteral("www.aliyun.com"),
          QStringLiteral("alibabacloud.com"), QStringLiteral("www.alibabacloud.com")},
         {}},

        {QStringLiteral("mimo"), BridgeMaterialKind::Cookies,
         {QStringLiteral("xiaomimimo.com"), QStringLiteral("platform.xiaomimimo.com"),
          QStringLiteral("aistudio.xiaomimimo.com"), QStringLiteral("mimo.xiaomi.com"),
          QStringLiteral("xiaomi.com")},
         {}},

        {QStringLiteral("manus"), BridgeMaterialKind::Cookies,
         {QStringLiteral("manus.im"), QStringLiteral("api.manus.im")},
         {}},

        {QStringLiteral("commandcode"), BridgeMaterialKind::Cookies,
         {QStringLiteral("commandcode.ai"), QStringLiteral("api.commandcode.ai")},
         {}},

        // --- LocalStorage-based providers ---
        {QStringLiteral("windsurf"), BridgeMaterialKind::LocalStorage,
         {},
         {},
         QStringLiteral("https://windsurf.com"),
         {QStringLiteral("devin_session_token"),
          QStringLiteral("devin_auth1_token"),
          QStringLiteral("devin_account_id"),
          QStringLiteral("devin_primary_org_id")}},
    };
}

std::optional<BridgeProviderSpec> BrowserSessionBridgeCatalog::specForProvider(const QString& providerId)
{
    const auto all = specs();
    for (const auto& s : all) {
        if (s.providerId == providerId)
            return s;
    }
    return std::nullopt;
}

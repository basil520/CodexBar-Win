#include <QtTest/QtTest>
#include "../src/providers/IFetchStrategy.h"

#include "../src/providers/codex/CodexProvider.h"
#include "../src/providers/amp/AmpProvider.h"
#include "../src/providers/abacus/AbacusProvider.h"
#include "../src/providers/augment/AugmentProvider.h"
#include "../src/providers/factory/FactoryProvider.h"
#include "../src/providers/perplexity/PerplexityProvider.h"
#include "../src/providers/kimi/KimiProvider.h"
#include "../src/providers/minimax/MiniMaxProvider.h"
#include "../src/providers/opencode/OpenCodeProvider.h"
#include "../src/providers/opencode/OpenCodeGoProvider.h"
#include "../src/providers/cursor/CursorProvider.h"
#include "../src/providers/claude/ClaudeProvider.h"
#include "../src/providers/alibaba/AlibabaProvider.h"
#include "../src/providers/ollama/OllamaProvider.h"
#include "../src/providers/mistral/MistralProvider.h"
#include "../src/providers/mimo/MiMoProvider.h"
#include "../src/providers/manus/ManusProvider.h"
#include "../src/providers/commandcode/CommandCodeProvider.h"
#include "../src/providers/windsurf/WindsurfProvider.h"

class tst_CredentialTargetBaseline : public QObject {
    Q_OBJECT

private slots:
    void descriptorTargetsArePresent();
};

void tst_CredentialTargetBaseline::descriptorTargetsArePresent() {
    auto checkProvider = [](const QString& providerId,
                            const QVector<ProviderSettingsDescriptor>& descs) {
        bool found = false;
        for (const auto& d : descs) {
            if (d.key == "manualCookieHeader") {
                found = true;
                QVERIFY2(!d.credentialTarget.isEmpty(),
                         qPrintable(providerId + " manualCookieHeader has no credentialTarget"));
                QVERIFY2(d.sensitive,
                         qPrintable(providerId + " manualCookieHeader is not marked sensitive"));
            }
        }
        QVERIFY2(found, qPrintable(providerId + " has no manualCookieHeader field"));
    };

    // Phase 0: 10 providers that needed credentialTarget added
    checkProvider("codex", CodexProvider().settingsDescriptors());
    checkProvider("amp", AmpProvider().settingsDescriptors());
    checkProvider("abacus", AbacusProvider().settingsDescriptors());
    checkProvider("augment", AugmentProvider().settingsDescriptors());
    checkProvider("factory", FactoryProvider().settingsDescriptors());
    checkProvider("perplexity", PerplexityProvider().settingsDescriptors());
    checkProvider("kimi", KimiProvider().settingsDescriptors());
    checkProvider("minimax", MiniMaxProvider().settingsDescriptors());
    checkProvider("opencode", OpenCodeProvider().settingsDescriptors());
    checkProvider("opencodego", OpenCodeGoProvider().settingsDescriptors());

    // 8 providers that already had credentialTarget
    checkProvider("cursor", CursorProvider().settingsDescriptors());
    checkProvider("claude", ClaudeProvider().settingsDescriptors());
    checkProvider("alibaba", AlibabaProvider().settingsDescriptors());
    checkProvider("ollama", OllamaProvider().settingsDescriptors());
    checkProvider("mistral", MistralProvider().settingsDescriptors());
    checkProvider("mimo", MiMoProvider().settingsDescriptors());
    checkProvider("manus", ManusProvider().settingsDescriptors());
    checkProvider("commandcode", CommandCodeProvider().settingsDescriptors());

    // 1 LocalStorage provider
    checkProvider("windsurf", WindsurfProvider().settingsDescriptors());
}

QTEST_MAIN(tst_CredentialTargetBaseline)
#include "tst_CredentialTargetBaseline.moc"

#include <QtTest/QtTest>

#include <QFile>
#include <QRegularExpression>
#include <QStringList>

namespace {

QString readFile(const QString& relativePath)
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/") + relativePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << relativePath << file.errorString();
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

} // namespace

class QmlArchitectureTest : public QObject {
    Q_OBJECT

private slots:
    void trayPanelDoesNotUseSynchronousUsageStoreGetters();
    void tokenUsagePaneDoesNotUseSynchronousUsageStoreGetters();
    void usageStoreInteractiveProviderJobsUseBackend();
    void usageStoreBackendJobsDoNotCaptureStore();
    void usageStoreSettingsAndStatusJobsUseBackend();
    void usageStoreCleanupJobsUseBackend();
    void settingsProvidersUsesSettingsProvidersModel();
    void usageDetailsRowsArePreparedByBackend();
    void costUsageViewDataIsLayered();
    void trayTokenUsageUsesScopedSummaryData();
    void costHistoryChartRefreshesAfterBackendDataArrives();
    void costHistoryRequestsAreProviderScopedAndCacheEmptyResults();
    void costHistoryDoesNotDependOnManualTokenUsageExpansion();
    void costHistoryChartUsesSharedHoverDetail();
    void usageStoreLegacyApisAreNotQmlInvokable();
    void bridgeViewModelDoesNotPerformSynchronousIo();
    void bridgeQmlDoesNotCallSynchronousBindingScan();
    void browserSessionCardInvalidatesImportFeedbackBindings();
    void browserSessionCardDoesNotDisplayRawStaleBindingIds();
    void browserSessionBridgeUiHonorsGlobalSetting();
    void glassOpacityLivesInDisplayPane();
    void topLevelWindowsUseAcrylicBackdropLayer();
    void nativeGlassExtendsDwmIntoClientArea();
    void claudePeakHoursLivesInDisplayPane();
    void appThemeExposesSharedGlassMaterialTokens();
    void acrylicBackdropUsesThemeTintScrim();
    void qmlSurfacesUseSharedMaterialHelpers();
    void paneFilesUsingQmlThemeHelpersImportParentThemeSingleton();
    void highRiskQmlDoesNotUseLegacyHardcodedSurfaceColors();
    void framelessGlassWindowsDoNotExposeNativeCaptionText();
    void usageStoreDoesNotInjectBridgeLookupWhenDisabled();
    void browserSessionBridgeExtensionUsesCanonicalWireProtocol();
    void providerUiBuildersUseCatalogSnapshot();
    void costUsageScanUsesCostUsageService();
    void openCodeCostScanScopesSqlToRecentSessions();
    void phaseZeroUiFoundationComponentsAreGuarded();
    void providerIconsUseSharedAvatar();
    void uiPolishRouteBComponentsAreGuarded();
    void actionButtonSupportsKeyboardAndAccessibleFocus();
    void providerErrorsUseSharedErrorNotice();
    void sharedControlsDoNotKeepLocalDuplicates();
    void trayProviderDockSupportsKeyboardAndAccessibility();
    void tokenUsagePaneUsesProductionUsageProviderRow();
    void finalUiPolishGuardsStayInPlace();
    void scrollBarsAvoidPermanentActiveState();
    void appThemeExposesInteractionPolishTokens();
    void providerAvatarUsesPolicyDrivenIconVessel();
    void providerSwitcherUsesAvatarDockPattern();
    void appQmlResourceBypassesMultiConfigAutoRcc();
};

void QmlArchitectureTest::trayPanelDoesNotUseSynchronousUsageStoreGetters()
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/qml/TrayPanel.qml"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

    const QString contents = QString::fromUtf8(file.readAll());
    const QStringList forbiddenCalls = {
        QStringLiteral("UsageStore.snapshotData("),
        QStringLiteral("UsageStore.tokenAccountsForProvider("),
        QStringLiteral("UsageStore.defaultTokenAccount("),
        QStringLiteral("UsageStore.providerStatusURL("),
        QStringLiteral("UsageStore.providerDashboardData("),
        QStringLiteral("UsageStore.providerCostUsageList("),
        QStringLiteral("UsageStore.costUsageData("),
    };

    for (const QString& call : forbiddenCalls) {
        QVERIFY2(!contents.contains(call),
                 qPrintable(QStringLiteral("TrayPanel.qml must read cached view-model state instead of calling %1").arg(call)));
    }
}

void QmlArchitectureTest::tokenUsagePaneDoesNotUseSynchronousUsageStoreGetters()
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/TokenUsagePane.qml"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

    const QString contents = QString::fromUtf8(file.readAll());
    const QStringList forbiddenCalls = {
        QStringLiteral("UsageStore.costUsageData("),
        QStringLiteral("UsageStore.providerCostUsageList("),
        QStringLiteral("UsageStore.providerList("),
    };

    for (const QString& call : forbiddenCalls) {
        QVERIFY2(!contents.contains(call),
                 qPrintable(QStringLiteral("TokenUsagePane.qml must read cached view-model state instead of calling %1").arg(call)));
    }
}

void QmlArchitectureTest::usageStoreInteractiveProviderJobsUseBackend()
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

    const QString contents = QString::fromUtf8(file.readAll());
    const QStringList forbiddenSnippets = {
        QStringLiteral("QtConcurrent::run(pool ? pool : m_threadPool"),
        QStringLiteral("QtConcurrent::run(m_interactiveThreadPool"),
    };

    for (const QString& snippet : forbiddenSnippets) {
        QVERIFY2(!contents.contains(snippet),
                 qPrintable(QStringLiteral("Interactive provider jobs must dispatch through UsageBackend, not %1").arg(snippet)));
    }

    QFile refreshCoordinator(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/ProviderRefreshCoordinator.cpp"));
    QVERIFY2(refreshCoordinator.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(refreshCoordinator.errorString()));
    const QString refreshContents = QString::fromUtf8(refreshCoordinator.readAll());

    QVERIFY2(contents.contains(QStringLiteral("QStringLiteral(\"providerConnectionTest\")")),
             "UsageStore must dispatch Test Connection through UsageBackend.");
    QVERIFY2(refreshContents.contains(QStringLiteral("QStringLiteral(\"providerRefresh\")")),
             "ProviderRefreshCoordinator must dispatch provider refresh through UsageBackend.");
}

void QmlArchitectureTest::usageStoreBackendJobsDoNotCaptureStore()
{
    QFile usageStore(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStore.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(usageStore.errorString()));
    const QString contents = QString::fromUtf8(usageStore.readAll());

    const QStringList forbiddenSnippets = {
        QStringLiteral("QStringLiteral(\"providerRefresh\"), 0, [this"),
        QStringLiteral("QStringLiteral(\"providerConnectionTest\"), 0,\n        [this"),
        QStringLiteral("QtConcurrent::run(usageStore->threadPool()"),
        QStringLiteral("m_threadPool"),
        QStringLiteral("m_interactiveThreadPool"),
    };

    for (const QString& snippet : forbiddenSnippets) {
        QVERIFY2(!contents.contains(snippet),
                 qPrintable(QStringLiteral("UsageStore must only dispatch/apply backend work, not retain old worker ownership: %1").arg(snippet)));
    }

    QFile refreshCoordinator(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/ProviderRefreshCoordinator.cpp"));
    QVERIFY2(refreshCoordinator.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(refreshCoordinator.errorString()));
    const QString refreshContents = QString::fromUtf8(refreshCoordinator.readAll());

    QVERIFY2(refreshContents.contains(QStringLiteral("UsageBackendJobs::refreshProvider")),
             "Provider refresh worker logic must live behind UsageBackendJobs.");
    QVERIFY2(contents.contains(QStringLiteral("UsageBackendJobs::testProviderConnection")),
             "Connection test worker logic must live behind UsageBackendJobs.");
    QVERIFY2(contents.contains(QStringLiteral("UsageBackendJobs::preloadCredentials")),
             "Credential preload worker logic must live behind UsageBackendJobs.");

    QFile mainFile(QStringLiteral(PROJECT_SOURCE_DIR "/src/main.cpp"));
    QVERIFY2(mainFile.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(mainFile.errorString()));
    const QString mainContents = QString::fromUtf8(mainFile.readAll());
    QVERIFY2(!mainContents.contains(QStringLiteral("QtConcurrent::run(usageStore->threadPool()")),
             "main.cpp must request credential preload through UsageStore/UsageBackend instead of owning a thread-pool job.");
}

void QmlArchitectureTest::usageStoreSettingsAndStatusJobsUseBackend()
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

    const QString contents = QString::fromUtf8(file.readAll());
    const QStringList forbiddenSnippets = {
        QStringLiteral("buildProviderListNow();\n        m_providerListRefreshQueued = false"),
        QStringLiteral("buildProviderDescriptorDataNow(providerId);\n        m_providerDescriptorRefreshQueued.remove"),
        QStringLiteral("QtConcurrent::run(m_threadPool, [batch, finishBatch, target]"),
        QStringLiteral("QtConcurrent::run(m_threadPool, [batch, finishBatch, workspaceURL"),
    };

    for (const QString& snippet : forbiddenSnippets) {
        QVERIFY2(!contents.contains(snippet),
                 qPrintable(QStringLiteral("Settings/status preparation must dispatch through UsageBackend, not %1").arg(snippet)));
    }

    QVERIFY2(contents.contains(QStringLiteral("providerStatuses")),
             "UsageStore must dispatch provider status polling through UsageBackend.");
    QVERIFY2(contents.contains(QStringLiteral("providerListModel")),
             "UsageStore must dispatch provider list preparation through UsageBackend.");
    QVERIFY2(contents.contains(QStringLiteral("providerDescriptorData")),
             "UsageStore must dispatch provider descriptor preparation through UsageBackend.");
}

void QmlArchitectureTest::usageStoreCleanupJobsUseBackend()
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

    const QString contents = QString::fromUtf8(file.readAll());
    QVERIFY2(!contents.contains(QStringLiteral("QtConcurrent::run(m_threadPool")),
             "UsageStore cleanup jobs must dispatch through UsageBackend instead of owning thread-pool jobs.");

    const QStringList requiredJobKinds = {
        QStringLiteral("QStringLiteral(\"codexCreditsRefresh\")"),
        QStringLiteral("QStringLiteral(\"credentialStatusCheck\")"),
        QStringLiteral("QStringLiteral(\"providerSecretWrite\")"),
        QStringLiteral("QStringLiteral(\"providerSecretRemove\")"),
        QStringLiteral("QStringLiteral(\"providerLoginStart\")"),
        QStringLiteral("QStringLiteral(\"providerLoginPoll\")"),
    };

    for (const QString& jobKind : requiredJobKinds) {
        QVERIFY2(contents.contains(jobKind),
                 qPrintable(QStringLiteral("UsageStore must dispatch cleanup job through UsageBackend: %1").arg(jobKind)));
    }
}

void QmlArchitectureTest::settingsProvidersUsesSettingsProvidersModel()
{
    QFile settingsWindow(QStringLiteral(PROJECT_SOURCE_DIR "/qml/SettingsWindow.qml"));
    QVERIFY2(settingsWindow.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(settingsWindow.errorString()));
    const QString settingsContents = QString::fromUtf8(settingsWindow.readAll());

    QVERIFY2(settingsContents.contains(QStringLiteral("SettingsProvidersModel")),
             "SettingsWindow.qml must route Providers tab state through SettingsProvidersModel.");

    const QStringList settingsForbiddenCalls = {
        QStringLiteral("UsageStore.providerList("),
        QStringLiteral("UsageStore.providerDescriptorData("),
        QStringLiteral("UsageStore.requestProviderList("),
        QStringLiteral("UsageStore.requestProviderDescriptor("),
    };
    for (const QString& call : settingsForbiddenCalls) {
        QVERIFY2(!settingsContents.contains(call),
                 qPrintable(QStringLiteral("SettingsWindow.qml must not call legacy UsageStore provider APIs: %1").arg(call)));
    }

    QFile providersPane(QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/ProvidersPane.qml"));
    QVERIFY2(providersPane.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(providersPane.errorString()));
    const QString providersContents = QString::fromUtf8(providersPane.readAll());
    QVERIFY2(providersContents.contains(QStringLiteral("model.providerId")),
             "ProvidersPane.qml must consume QAbstractListModel roles for provider rows.");
    QVERIFY2(!providersContents.contains(QStringLiteral("UsageStore.moveProvider(")),
             "ProvidersPane.qml must emit a command signal instead of calling UsageStore.moveProvider().");

    QFile providerDetail(QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/ProviderDetailView.qml"));
    QVERIFY2(providerDetail.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(providerDetail.errorString()));
    const QString detailContents = QString::fromUtf8(providerDetail.readAll());

    const QStringList detailForbiddenCalls = {
        QStringLiteral("UsageStore.tokenAccountsForProvider("),
        QStringLiteral("UsageStore.defaultTokenAccount("),
        QStringLiteral("UsageStore.requestAddTokenAccount("),
        QStringLiteral("UsageStore.requestAddTokenAccountWithApiKey("),
        QStringLiteral("UsageStore.requestRemoveTokenAccount("),
        QStringLiteral("UsageStore.requestSetDefaultTokenAccount("),
        QStringLiteral("UsageStore.requestSetTokenAccountSourceMode("),
        QStringLiteral("UsageStore.requestSetTokenAccountVisibility("),
        QStringLiteral("UsageStore.codexAccountState"),
        QStringLiteral("UsageStore.codexConsumerProjectionData("),
    };

    for (const QString& call : detailForbiddenCalls) {
        QVERIFY2(!detailContents.contains(call),
                 qPrintable(QStringLiteral("ProviderDetailView.qml must bind prepared SettingsProvidersModel state instead of %1").arg(call)));
    }
}

void QmlArchitectureTest::usageDetailsRowsArePreparedByBackend()
{
    QFile viewModel(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageDetailsViewModel.cpp"));
    QVERIFY2(viewModel.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(viewModel.errorString()));
    const QString vmContents = QString::fromUtf8(viewModel.readAll());

    const QStringList forbiddenVmSnippets = {
        QStringLiteral("buildProviderRows("),
        QStringLiteral("m_store->providerList("),
        QStringLiteral("m_store->providerCostUsageList("),
    };
    for (const QString& snippet : forbiddenVmSnippets) {
        QVERIFY2(!vmContents.contains(snippet),
                 qPrintable(QStringLiteral("UsageDetailsViewModel must consume backend-prepared rows, not %1").arg(snippet)));
    }
    QVERIFY2(vmContents.contains(QStringLiteral("m_store->costUsageDetailsRows()")),
             "UsageDetailsViewModel must read backend-prepared details rows.");
    QVERIFY2(vmContents.contains(QStringLiteral("requestCostUsageProviderDetail")),
             "UsageDetailsViewModel must request provider detail lazily.");

    QFile usageStore(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStore.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(usageStore.errorString()));
    const QString storeContents = QString::fromUtf8(usageStore.readAll());
    QVERIFY2(storeContents.contains(QStringLiteral("QStringLiteral(\"costUsageProviderDetail\")")),
             "UsageStore must dispatch provider detail preparation through UsageBackend.");
    QVERIFY2(storeContents.contains(QStringLiteral("QStringLiteral(\"costUsageDetailsRows\")")),
             "UsageStore must dispatch Usage details row preparation through UsageBackend.");
    QVERIFY2(storeContents.contains(QStringLiteral("CostUsageService::detailsRows")),
             "Cost usage details rows must be built by CostUsageService in the backend job.");

    QFile pane(QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/TokenUsagePane.qml"));
    QVERIFY2(pane.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(pane.errorString()));
    const QString paneContents = QString::fromUtf8(pane.readAll());
    QVERIFY2(paneContents.contains(QStringLiteral("UsageDetailsViewModel.requestProviderDetail")),
             "TokenUsagePane must request model breakdown only when a provider is expanded.");
    QVERIFY2(paneContents.contains(QStringLiteral("property bool expanded: false")),
             "Provider usage cards must start collapsed.");
    QVERIFY2(!paneContents.contains(QStringLiteral("model: card.provider.models")),
             "TokenUsagePane must not render model breakdown from first-screen provider rows.");
}

void QmlArchitectureTest::costUsageViewDataIsLayered()
{
    QFile usageStore(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStore.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(usageStore.errorString()));
    const QString storeContents = QString::fromUtf8(usageStore.readAll());

    const QStringList requiredJobKinds = {
        QStringLiteral("QStringLiteral(\"costUsageSummary\")"),
        QStringLiteral("QStringLiteral(\"costUsageProviderRows\")"),
        QStringLiteral("QStringLiteral(\"costUsageDetailsRows\")"),
        QStringLiteral("QStringLiteral(\"costUsageProviderDetail\")"),
    };
    for (const QString& jobKind : requiredJobKinds) {
        QVERIFY2(storeContents.contains(jobKind),
                 qPrintable(QStringLiteral("Cost usage view data must be dispatched as layered backend jobs: %1").arg(jobKind)));
    }
    QVERIFY2(!storeContents.contains(QStringLiteral("QStringLiteral(\"costUsageViewData\")")),
             "UsageStore must not dispatch one combined costUsageViewData job.");

    const QString summaryStart = QStringLiteral("QVariantMap UsageStore::costUsageData() const");
    const QString providerRowsStart = QStringLiteral("QVariantList UsageStore::providerCostUsageList() const");
    const QString detailsRowsStart = QStringLiteral("QVariantList UsageStore::costUsageDetailsRows() const");
    const QString tokenCountStart = QStringLiteral("int UsageStore::costUsageTokenProviderCount() const");
    const int summaryStartIndex = storeContents.indexOf(summaryStart);
    QVERIFY2(summaryStartIndex >= 0, "Missing UsageStore::costUsageData().");
    const int providerRowsStartIndex = storeContents.indexOf(providerRowsStart, summaryStartIndex + summaryStart.size());
    QVERIFY2(providerRowsStartIndex > summaryStartIndex, "Missing UsageStore::providerCostUsageList().");
    const int detailsRowsStartIndex = storeContents.indexOf(detailsRowsStart, providerRowsStartIndex + providerRowsStart.size());
    QVERIFY2(detailsRowsStartIndex > providerRowsStartIndex, "Missing UsageStore::costUsageDetailsRows().");
    const int tokenCountStartIndex = storeContents.indexOf(tokenCountStart, detailsRowsStartIndex + detailsRowsStart.size());
    QVERIFY2(tokenCountStartIndex > detailsRowsStartIndex, "Missing UsageStore::costUsageTokenProviderCount().");

    const QString summaryBody = storeContents.mid(summaryStartIndex, providerRowsStartIndex - summaryStartIndex);
    QVERIFY2(summaryBody.contains(QStringLiteral("requestCostUsageSummary")),
             "costUsageData() must only schedule the summary builder.");
    QVERIFY2(!summaryBody.contains(QStringLiteral("requestCostUsageProviderRows")),
             "costUsageData() must not schedule provider row construction.");
    QVERIFY2(!summaryBody.contains(QStringLiteral("requestCostUsageDetailsRows")),
             "costUsageData() must not schedule Usage details rows.");

    const QString providerRowsBody = storeContents.mid(providerRowsStartIndex, detailsRowsStartIndex - providerRowsStartIndex);
    QVERIFY2(providerRowsBody.contains(QStringLiteral("requestCostUsageProviderRows")),
             "providerCostUsageList() must schedule only the provider rows builder.");
    QVERIFY2(!providerRowsBody.contains(QStringLiteral("requestCostUsageDetailsRows")),
             "providerCostUsageList() must not schedule Usage details rows.");

    const QString detailsRowsBody = storeContents.mid(detailsRowsStartIndex, tokenCountStartIndex - detailsRowsStartIndex);
    QVERIFY2(detailsRowsBody.contains(QStringLiteral("requestCostUsageDetailsRows")),
             "costUsageDetailsRows() must be the only getter that schedules Usage details rows.");
    QVERIFY2(!detailsRowsBody.contains(QStringLiteral("requestCostUsageSummary")),
             "costUsageDetailsRows() must not request a combined view-data build.");

    QFile trayViewModel(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/TrayViewModel.cpp"));
    QVERIFY2(trayViewModel.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(trayViewModel.errorString()));
    const QString trayContents = QString::fromUtf8(trayViewModel.readAll());
    const QString traySyncStart = QStringLiteral("void TrayViewModel::syncCostData()");
    const int traySyncStartIndex = trayContents.indexOf(traySyncStart);
    QVERIFY2(traySyncStartIndex >= 0, "Missing TrayViewModel::syncCostData().");
    const QString traySyncBody = trayContents.mid(traySyncStartIndex);
    QVERIFY2(!traySyncBody.contains(QStringLiteral("m_store->providerCostUsageList()")),
             "TrayViewModel must keep the tray-visible sync path to summary-only cost usage data.");

    QFile detailsViewModel(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageDetailsViewModel.cpp"));
    QVERIFY2(detailsViewModel.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(detailsViewModel.errorString()));
    const QString detailsVmContents = QString::fromUtf8(detailsViewModel.readAll());
    QVERIFY2(detailsVmContents.contains(QStringLiteral("requestCostUsageSummary")),
             "UsageDetailsViewModel must request the summary layer explicitly.");
    QVERIFY2(detailsVmContents.contains(QStringLiteral("requestCostUsageDetailsRows")),
             "UsageDetailsViewModel must request details rows only while active.");
    QVERIFY2(!detailsVmContents.contains(QStringLiteral("requestCostUsageViewData")),
             "UsageDetailsViewModel must not request a combined view-data build.");

    QFile service(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/CostUsageService.cpp"));
    QVERIFY2(service.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(service.errorString()));
    const QString serviceContents = QString::fromUtf8(service.readAll());
    QVERIFY2(serviceContents.contains(QStringLiteral("CostUsageService::summaryData")),
             "CostUsageService must own cost summary view-data construction.");
    QVERIFY2(serviceContents.contains(QStringLiteral("CostUsageService::providerRows")),
             "CostUsageService must own provider row view-data construction.");
    QVERIFY2(serviceContents.contains(QStringLiteral("CostUsageService::detailsRows")),
             "CostUsageService must own Usage details row view-data construction.");
}

void QmlArchitectureTest::trayTokenUsageUsesScopedSummaryData()
{
    QFile tray(QStringLiteral(PROJECT_SOURCE_DIR "/qml/TrayPanel.qml"));
    QVERIFY2(tray.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(tray.errorString()));
    const QString trayContents = QString::fromUtf8(tray.readAll());

    QVERIFY2(trayContents.contains(QStringLiteral("displayCostData")),
             "TrayPanel must bind Token Usage summary/daily chart through selected-provider scoped displayCostData.");
    QVERIFY2(trayContents.contains(QStringLiteral("TrayViewModel.displayCostData")),
             "TrayPanel displayCostData must come from TrayViewModel, not directly from the overview costData.");
    QVERIFY2(!trayContents.contains(QStringLiteral("title: qsTr(\"Today\")\n                                value: \"$\" + formatCost(costData.sessionCostUSD)")),
             "TrayPanel selected-provider Token Usage summary must not read overview costData directly.");
    QVERIFY2(!trayContents.contains(QStringLiteral("model: root.costExpanded && costData.daily")),
             "TrayPanel selected-provider Token Usage daily bars must not read overview costData directly.");

    QFile trayViewModel(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/TrayViewModel.cpp"));
    QVERIFY2(trayViewModel.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(trayViewModel.errorString()));
    const QString vmContents = QString::fromUtf8(trayViewModel.readAll());
    QVERIFY2(vmContents.contains(QStringLiteral("costUsageDataForProvider")),
             "TrayViewModel must expose selected-provider scoped Token Usage summary data.");
}

void QmlArchitectureTest::costHistoryChartRefreshesAfterBackendDataArrives()
{
    QFile chart(QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/CostHistoryChart.qml"));
    QVERIFY2(chart.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(chart.errorString()));
    const QString chartContents = QString::fromUtf8(chart.readAll());

    QVERIFY2(chartContents.contains(QStringLiteral("function refreshPoints()")),
             "CostHistoryChart must own a refreshPoints() path that can re-read backend-prepared data.");
    QVERIFY2(chartContents.contains(QStringLiteral("UsageStore.costHistoryChartData(root.providerId)")),
             "CostHistoryChart must request scoped cost history points for its provider.");
    QVERIFY2(chartContents.contains(QStringLiteral("function onCostHistoryChanged")),
             "CostHistoryChart must listen for async cost history completion.");
    QVERIFY2(chartContents.contains(QStringLiteral("function onCostUsageChanged")),
             "CostHistoryChart must re-request points when token usage refresh invalidates chart caches.");
    QVERIFY2(chartContents.contains(QStringLiteral("root.refreshPoints()")),
             "CostHistoryChart must re-read points when UsageStore emits costHistoryChanged().");

    QFile card(QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/ProviderDetailCard.qml"));
    QVERIFY2(card.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(card.errorString()));
    const QString cardContents = QString::fromUtf8(card.readAll());
    QVERIFY2(!cardContents.contains(QStringLiteral("points: UsageStore.costHistoryChartData(root.providerId)")),
             "ProviderDetailCard must not one-shot bind cost history points; the chart must refresh after backend completion.");
}

void QmlArchitectureTest::costHistoryRequestsAreProviderScopedAndCacheEmptyResults()
{
    QFile header(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.h"));
    QVERIFY2(header.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(header.errorString()));
    const QString headerContents = QString::fromUtf8(header.readAll());
    QVERIFY2(headerContents.contains(QStringLiteral("m_costHistoryCachedProviderIds")),
             "Cost history must track cache validity per provider, including empty point lists.");
    QVERIFY2(headerContents.contains(QStringLiteral("m_costHistoryQueuedProviderIds")),
             "Cost history builds must queue per provider instead of using one global in-flight flag.");
    QVERIFY2(!headerContents.contains(QStringLiteral("mutable bool m_costHistoryBuildQueued")),
             "A single global cost history build flag drops simultaneous provider requests.");

    QFile source(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(source.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(source.errorString()));
    const QString sourceContents = QString::fromUtf8(source.readAll());
    const QString getterStart = QStringLiteral("QVariantList UsageStore::costHistoryChartData(const QString& providerId) const");
    const QString getterEnd = QStringLiteral("QVariantList UsageStore::creditsHistoryData() const");
    const int getterStartIndex = sourceContents.indexOf(getterStart);
    QVERIFY2(getterStartIndex >= 0, "Missing UsageStore::costHistoryChartData().");
    const int getterEndIndex = sourceContents.indexOf(getterEnd, getterStartIndex + getterStart.size());
    QVERIFY2(getterEndIndex > getterStartIndex, "Missing method after costHistoryChartData().");
    const QString getterBody = sourceContents.mid(getterStartIndex, getterEndIndex - getterStartIndex);
    QVERIFY2(getterBody.contains(QStringLiteral("m_costHistoryCachedProviderIds.contains")),
             "costHistoryChartData() must treat an empty cached list as a valid cached result.");
    QVERIFY2(!getterBody.contains(QStringLiteral("!it->isEmpty()")),
             "costHistoryChartData() must not keep re-queueing providers whose valid result is empty.");
}

void QmlArchitectureTest::costHistoryDoesNotDependOnManualTokenUsageExpansion()
{
    QFile header(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.h"));
    QVERIFY2(header.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(header.errorString()));
    const QString headerContents = QString::fromUtf8(header.readAll());
    QVERIFY2(headerContents.contains(QStringLiteral("m_costUsageDataAvailable")),
             "UsageStore must distinguish an unknown token-usage baseline from a valid empty cost-history result.");

    QFile source(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(source.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(source.errorString()));
    const QString sourceContents = QString::fromUtf8(source.readAll());

    const QString requestStart = QStringLiteral("void UsageStore::requestCostHistory(const QString& providerId)");
    const QString requestEnd = QStringLiteral("void UsageStore::requestCreditsHistory()");
    const int requestStartIndex = sourceContents.indexOf(requestStart);
    QVERIFY2(requestStartIndex >= 0, "Missing UsageStore::requestCostHistory().");
    const int requestEndIndex = sourceContents.indexOf(requestEnd, requestStartIndex + requestStart.size());
    QVERIFY2(requestEndIndex > requestStartIndex, "Missing method after requestCostHistory().");
    const QString requestBody = sourceContents.mid(requestStartIndex, requestEndIndex - requestStartIndex);
    QVERIFY2(requestBody.contains(QStringLiteral("!m_costUsageDataAvailable")),
             "requestCostHistory() must not build/cache empty chart data before token usage scan has produced a baseline.");
    QVERIFY2(requestBody.contains(QStringLiteral("ensureCostUsageEnabled()")),
             "CostHistoryChart must be able to trigger token usage scanning without relying on the Token Usage card.");

    const QString refreshStart = QStringLiteral("void UsageStore::refreshCostUsage()");
    const QString refreshEnd = QStringLiteral("QVariantMap UsageStore::costUsageData() const");
    const int refreshStartIndex = sourceContents.indexOf(refreshStart);
    QVERIFY2(refreshStartIndex >= 0, "Missing UsageStore::refreshCostUsage().");
    const int refreshEndIndex = sourceContents.indexOf(refreshEnd, refreshStartIndex + refreshStart.size());
    QVERIFY2(refreshEndIndex > refreshStartIndex, "Missing method after refreshCostUsage().");
    const QString refreshBody = sourceContents.mid(refreshStartIndex, refreshEndIndex - refreshStartIndex);
    QVERIFY2(refreshBody.contains(QStringLiteral("m_costUsageDataAvailable = true")),
             "refreshCostUsage() must mark when token usage has a known baseline, even if that baseline is empty.");
}

void QmlArchitectureTest::costHistoryChartUsesSharedHoverDetail()
{
    QFile qrc(QStringLiteral(PROJECT_SOURCE_DIR "/resources/qml.qrc"));
    QVERIFY2(qrc.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(qrc.errorString()));
    const QString qrcContents = QString::fromUtf8(qrc.readAll());
    QVERIFY2(qrcContents.contains(QStringLiteral("qml/components/ChartHoverDetail.qml")),
             "ChartHoverDetail must be packaged with the QML resources.");

    QFile hoverDetail(QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/ChartHoverDetail.qml"));
    QVERIFY2(hoverDetail.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(hoverDetail.errorString()));
    const QString hoverContents = QString::fromUtf8(hoverDetail.readAll());
    QVERIFY2(hoverContents.contains(QStringLiteral("property string primaryText")),
             "ChartHoverDetail must expose primary text for chart detail rows.");
    QVERIFY2(hoverContents.contains(QStringLiteral("property string secondaryText")),
             "ChartHoverDetail must expose secondary text for chart detail rows.");
    QVERIFY2(hoverContents.contains(QStringLiteral("Rectangle {")),
             "ChartHoverDetail must render a visible tooltip/panel container, not just loose text rows.");
    QVERIFY2(hoverContents.contains(QStringLiteral("property bool floating")),
             "ChartHoverDetail must support floating overlay usage inside charts.");

    QFile chart(QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/CostHistoryChart.qml"));
    QVERIFY2(chart.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(chart.errorString()));
    const QString chartContents = QString::fromUtf8(chart.readAll());
    QVERIFY2(chartContents.contains(QStringLiteral("ChartHoverDetail {")),
             "CostHistoryChart must render hover/detail text through the shared ChartHoverDetail component.");
    QVERIFY2(!chartContents.contains(QStringLiteral("id: detailArea")),
             "CostHistoryChart must not keep a duplicate inline detailArea implementation.");
    QVERIFY2(chartContents.contains(QStringLiteral("floating: true")),
             "CostHistoryChart must use ChartHoverDetail as a floating hover panel instead of a fixed footer row.");
    QVERIFY2(chartContents.contains(QStringLiteral("visible: hoveredIndex >= 0")),
             "CostHistoryChart hover detail must appear only while hovering a chart point.");
    QVERIFY2(chartContents.contains(QStringLiteral("width: implicitWidth")),
             "Floating ChartHoverDetail must bind width to implicitWidth because it is outside a Layout.");
    QVERIFY2(chartContents.contains(QStringLiteral("height: implicitHeight")),
             "Floating ChartHoverDetail must bind height to implicitHeight because it is outside a Layout.");
    QVERIFY2(chartContents.contains(QStringLiteral("property real hoverX")),
             "CostHistoryChart must track the hover X coordinate to position the floating detail panel.");
    QVERIFY2(chartContents.contains(QStringLiteral("property real hoverY")),
             "CostHistoryChart must track the hover Y coordinate to position the floating detail panel.");
    QVERIFY2(!chartContents.contains(QStringLiteral("hoverDetail.implicitHeight")),
             "CostHistoryChart must not reserve bottom layout height for the hover detail panel.");
}

void QmlArchitectureTest::usageStoreLegacyApisAreNotQmlInvokable()
{
    QFile usageStoreHeader(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.h"));
    QVERIFY2(usageStoreHeader.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(usageStoreHeader.errorString()));
    const QString headerContents = QString::fromUtf8(usageStoreHeader.readAll());

    const QStringList forbiddenInvokables = {
        QStringLiteral("Q_INVOKABLE void setProviderEnabled"),
        QStringLiteral("Q_INVOKABLE void setProviderSetting"),
        QStringLiteral("Q_INVOKABLE bool setProviderSecret"),
        QStringLiteral("Q_INVOKABLE bool clearProviderSecret"),
        QStringLiteral("Q_INVOKABLE void testProviderConnection"),
        QStringLiteral("Q_INVOKABLE void startProviderLogin"),
        QStringLiteral("Q_INVOKABLE void cancelProviderLogin"),
        QStringLiteral("Q_INVOKABLE void refreshProviderStatuses"),
        QStringLiteral("Q_INVOKABLE QVariantMap providerCostUsageData"),
        QStringLiteral("Q_INVOKABLE QVariantList providerSettingsFields"),
    };
    for (const QString& signature : forbiddenInvokables) {
        QVERIFY2(!headerContents.contains(signature),
                 qPrintable(QStringLiteral("Legacy UsageStore API must not remain QML-invokable: %1").arg(signature)));
    }

    QVERIFY2(!headerContents.contains(QStringLiteral("providerCostUsageData(")),
             "providerCostUsageData() must be removed after UsageDetailsModel/CostUsageService layering.");
    QVERIFY2(!headerContents.contains(QStringLiteral("providerSettingsFields(")),
             "providerSettingsFields() direct QML path must be removed; descriptor data carries settings fields.");

    QFile usageStoreSource(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStoreSource.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(usageStoreSource.errorString()));
    const QString sourceContents = QString::fromUtf8(usageStoreSource.readAll());
    QVERIFY2(!sourceContents.contains(QStringLiteral("UsageStore::providerCostUsageData")),
             "providerCostUsageData() implementation must be deleted.");
    QVERIFY2(!sourceContents.contains(QStringLiteral("UsageStore::providerSettingsFields")),
             "providerSettingsFields() implementation must be deleted.");

    const QStringList qmlFiles = {
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/TrayPanel.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/SettingsWindow.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/UsageWindow.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/PlanUtilizationChart.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/DebugPane.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/ProvidersPane.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/panes/TokenUsagePane.qml"),
        QStringLiteral(PROJECT_SOURCE_DIR "/qml/components/ProviderDetailView.qml"),
    };
    const QStringList forbiddenQmlCalls = {
        QStringLiteral("UsageStore.setProviderEnabled("),
        QStringLiteral("UsageStore.setProviderSetting("),
        QStringLiteral("UsageStore.setProviderSecret("),
        QStringLiteral("UsageStore.clearProviderSecret("),
        QStringLiteral("UsageStore.testProviderConnection("),
        QStringLiteral("UsageStore.startProviderLogin("),
        QStringLiteral("UsageStore.cancelProviderLogin("),
        QStringLiteral("UsageStore.providerCostUsageData("),
        QStringLiteral("UsageStore.providerSettingsFields("),
    };
    for (const QString& fileName : qmlFiles) {
        QFile file(fileName);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));
        const QString contents = QString::fromUtf8(file.readAll());
        for (const QString& call : forbiddenQmlCalls) {
            QVERIFY2(!contents.contains(call),
                     qPrintable(QStringLiteral("%1 must route legacy UsageStore access through a ViewModel/request command, not %2")
                                    .arg(fileName, call)));
        }
    }
}

void QmlArchitectureTest::bridgeViewModelDoesNotPerformSynchronousIo()
{
    const QString source = readFile("src/app/BridgeViewModel.cpp");
    QVERIFY2(!source.contains("ProviderCredentialStore"),
             "BridgeViewModel must not access WinCred or credential storage on the UI thread.");
    QVERIFY2(!source.contains("metadataStore().save()"),
             "BridgeViewModel invokables must persist metadata asynchronously.");
    QVERIFY2(!source.contains("ensureExtensionExported()"),
             "BridgeViewModel must dispatch extension export to a worker instead of running file IO inline.");
    QVERIFY2(!source.contains("isExtensionExported()"),
             "QML properties must not perform filesystem existence scans.");
}

void QmlArchitectureTest::bridgeQmlDoesNotCallSynchronousBindingScan()
{
    const QString providerDetail = readFile("qml/components/ProviderDetailView.qml");
    QVERIFY2(!providerDetail.contains("availableBindings("),
             "Provider detail UI must use cached binding options instead of synchronous credential exists() scans.");
}

void QmlArchitectureTest::browserSessionCardInvalidatesImportFeedbackBindings()
{
    const QString card = readFile("qml/components/BrowserSessionCard.qml");
    QVERIFY2(card.contains("visible: root.refreshKey"),
             "BrowserSessionCard visibility bindings that call BridgeViewModel invokables must depend on refreshKey so import failures repaint immediately.");
    QVERIFY2(card.contains("text: root.refreshKey"),
             "BrowserSessionCard text bindings that call BridgeViewModel invokables must depend on refreshKey so import failures repaint immediately.");
}

void QmlArchitectureTest::browserSessionCardDoesNotDisplayRawStaleBindingIds()
{
    const QString card = readFile("qml/components/BrowserSessionCard.qml");
    QVERIFY2(!card.contains("return id"),
             "BrowserSessionCard must not display raw stale binding ids such as edge:<uuid>; show Auto or an unavailable-profile label instead.");
}

void QmlArchitectureTest::browserSessionBridgeUiHonorsGlobalSetting()
{
    const QString advanced = readFile("qml/panes/AdvancedPane.qml");
    const QString providerDetail = readFile("qml/components/ProviderDetailView.qml");

    QVERIFY2(advanced.contains(QStringLiteral("SettingsStore.browserSessionBridgeEnabled")),
             "AdvancedPane must expose a global Browser Session Bridge/Cookies import setting.");
    QVERIFY2(advanced.contains(QStringLiteral("active: SettingsStore.browserSessionBridgeEnabled")),
             "AdvancedPane must not instantiate Bridge install UI while Cookies import is disabled.");
    QVERIFY2(providerDetail.contains(QStringLiteral("SettingsStore.browserSessionBridgeEnabled")),
             "ProviderDetailView must hide Browser Session Bridge UI behind the global Cookies import setting.");
    QVERIFY2(providerDetail.contains(QStringLiteral("active: SettingsStore.browserSessionBridgeEnabled")),
             "ProviderDetailView must not instantiate BrowserSessionCard while Cookies import is disabled.");
}

void QmlArchitectureTest::glassOpacityLivesInDisplayPane()
{
    const QString display = readFile("qml/panes/DisplayPane.qml");
    QVERIFY2(display.contains(QStringLiteral("SettingsStore.glassEffectEnabled")),
             "DisplayPane must expose the glass effect toggle.");
    QVERIFY2(display.contains(QStringLiteral("SettingsStore.glassEffectOpacity")),
             "DisplayPane must bind a glass opacity setting.");
    QVERIFY2(display.contains(QStringLiteral("Slider")),
             "Glass opacity must use a slider control.");
    QVERIFY2(display.contains(QStringLiteral("from: 5")),
             "Glass opacity must allow a low enough value for an obvious acrylic effect.");
}

void QmlArchitectureTest::topLevelWindowsUseAcrylicBackdropLayer()
{
    const QString qrc = readFile("resources/qml.qrc");
    QVERIFY2(qrc.contains(QStringLiteral("qml/components/AcrylicBackdrop.qml")),
             "AcrylicBackdrop.qml must be embedded in app resources.");

    const QStringList windows = {
        QStringLiteral("qml/SettingsWindow.qml"),
        QStringLiteral("qml/TrayPanel.qml"),
        QStringLiteral("qml/UsageWindow.qml"),
    };

    for (const QString& path : windows) {
        const QString contents = readFile(path);
        QVERIFY2(contents.contains(QStringLiteral("Components.AcrylicBackdrop")),
                 qPrintable(path + QStringLiteral(" must render the shared acrylic material layer.")));
    }
}

void QmlArchitectureTest::nativeGlassExtendsDwmIntoClientArea()
{
    const QString contents = readFile("src/app/WindowGlassEffect.cpp");
    QVERIFY2(contents.contains(QStringLiteral("DwmExtendFrameIntoClientArea")),
             "Native glass must extend the DWM frame into the Qt client area.");
    QVERIFY2(contents.contains(QStringLiteral("DwmEnableBlurBehindWindow")),
             "Native glass must enable DWM blur behind the window, not only set an accent policy.");
}

void QmlArchitectureTest::claudePeakHoursLivesInDisplayPane()
{
    const QString general = readFile("qml/panes/GeneralPane.qml");
    const QString display = readFile("qml/panes/DisplayPane.qml");

    QVERIFY2(!general.contains(QStringLiteral("Claude Peak Hours")),
             "Claude Peak Hours is a display preference and must not remain in GeneralPane.");
    QVERIFY2(!general.contains(QStringLiteral("claudePeakHoursEnabled")),
             "GeneralPane must not bind claudePeakHoursEnabled after moving the setting to DisplayPane.");
    QVERIFY2(display.contains(QStringLiteral("Claude Peak Hours")),
             "DisplayPane must show the Claude Peak Hours setting.");
    QVERIFY2(display.contains(QStringLiteral("SettingsStore.claudePeakHoursEnabled")),
             "DisplayPane must bind the existing claudePeakHoursEnabled setting.");
}

void QmlArchitectureTest::appThemeExposesSharedGlassMaterialTokens()
{
    const QString theme = readFile("qml/AppTheme.qml");
    const QStringList requiredTokens = {
        QStringLiteral("property color surfaceWindow"),
        QStringLiteral("property color surfacePane"),
        QStringLiteral("property color surfaceCard"),
        QStringLiteral("property color surfaceControl"),
        QStringLiteral("property color surfacePopup"),
        QStringLiteral("property color surfaceChart"),
        QStringLiteral("property color surfaceHover"),
        QStringLiteral("property color surfacePressed"),
        QStringLiteral("property color surfaceBorder"),
        QStringLiteral("property color textOnAccent"),
        QStringLiteral("function withAlpha("),
        QStringLiteral("function providerBrandColor("),
    };

    for (const QString& token : requiredTokens) {
        QVERIFY2(theme.contains(token),
                 qPrintable(QStringLiteral("AppTheme.qml must expose shared glass/theme material token: %1").arg(token)));
    }
}

void QmlArchitectureTest::acrylicBackdropUsesThemeTintScrim()
{
    const QString backdrop = readFile("qml/components/AcrylicBackdrop.qml");
    QVERIFY2(backdrop.contains(QStringLiteral("root.tint.r")),
             "AcrylicBackdrop must actually use the provided theme tint color as a scrim.");
    QVERIFY2(backdrop.contains(QStringLiteral("AppTheme.surfaceWindow")),
             "AcrylicBackdrop must align its base material with AppTheme.surfaceWindow.");
    QVERIFY2(!backdrop.contains(QStringLiteral("Qt.rgba(255, 255, 255, 0.018")),
             "AcrylicBackdrop must not use a white wash as the primary acrylic layer.");
}

void QmlArchitectureTest::qmlSurfacesUseSharedMaterialHelpers()
{
    const QStringList files = {
        QStringLiteral("qml/SettingsWindow.qml"),
        QStringLiteral("qml/UsageWindow.qml"),
        QStringLiteral("qml/TrayPanel.qml"),
        QStringLiteral("qml/components/SettingsGroupBox.qml"),
        QStringLiteral("qml/components/SettingsComboBox.qml"),
        QStringLiteral("qml/components/SecretInput.qml"),
        QStringLiteral("qml/components/TrayProviderDock.qml"),
        QStringLiteral("qml/components/ProviderDetailCard.qml"),
        QStringLiteral("qml/components/ChartHoverDetail.qml"),
        QStringLiteral("qml/components/DeleteConfirmationDialog.qml"),
        QStringLiteral("qml/components/BrowserSessionBindingDialog.qml"),
        QStringLiteral("qml/panes/TokenUsagePane.qml"),
    };

    for (const QString& path : files) {
        const QString contents = readFile(path);
        QVERIFY2(contents.contains(QStringLiteral("AppTheme.surface")),
                 qPrintable(path + QStringLiteral(" must use AppTheme surface tokens instead of local material math.")));
        QVERIFY2(!contents.contains(QStringLiteral("glassEffectOpacity / 100")),
                 qPrintable(path + QStringLiteral(" must not compute glass opacity locally; use AppTheme surface tokens.")));
        QVERIFY2(!contents.contains(QStringLiteral("function colorWithAlpha")),
                 qPrintable(path + QStringLiteral(" must not define local colorWithAlpha helpers; use AppTheme.withAlpha().")));
    }
}

void QmlArchitectureTest::paneFilesUsingQmlThemeHelpersImportParentThemeSingleton()
{
    const QStringList paneFiles = {
        QStringLiteral("qml/panes/ProvidersPane.qml"),
        QStringLiteral("qml/panes/TokenUsagePane.qml"),
        QStringLiteral("qml/panes/DebugPane.qml"),
    };

    for (const QString& path : paneFiles) {
        const QString contents = readFile(path);
        const bool usesQmlThemeHelpers =
            contents.contains(QStringLiteral("AppTheme.surface"))
            || contents.contains(QStringLiteral("AppTheme.withAlpha"))
            || contents.contains(QStringLiteral("AppTheme.providerBrandColor"))
            || contents.contains(QStringLiteral("AppTheme.textOnAccent"));
        if (!usesQmlThemeHelpers) {
            continue;
        }

        QVERIFY2(contents.contains(QStringLiteral("import \"..\"")),
                 qPrintable(path + QStringLiteral(" must import the parent qml directory so AppTheme resolves to AppTheme.qml, not the C++ AppTheme singleton.")));
    }
}

void QmlArchitectureTest::highRiskQmlDoesNotUseLegacyHardcodedSurfaceColors()
{
    const QStringList files = {
        QStringLiteral("qml/TrayPanel.qml"),
        QStringLiteral("qml/components/ProviderDetailCard.qml"),
        QStringLiteral("qml/components/ChartHoverDetail.qml"),
        QStringLiteral("qml/components/SettingsButton.qml"),
        QStringLiteral("qml/components/SettingsSwitch.qml"),
        QStringLiteral("qml/components/DeleteConfirmationDialog.qml"),
    };
    const QStringList forbiddenColors = {
        QStringLiteral("#252545"),
        QStringLiteral("#1f1f38"),
        QStringLiteral("#1c1c32"),
        QStringLiteral("#202038"),
        QStringLiteral("#2a2a4a"),
        QStringLiteral("#3a3a5c"),
        QStringLiteral("#3a3a6a"),
        QStringLiteral("#4a4a7a"),
        QStringLiteral("#25253e"),
        QStringLiteral("#3b3b5d"),
        QStringLiteral("#ddd"),
        QStringLiteral("#aaa"),
        QStringLiteral("#888"),
        QStringLiteral("#666"),
        QStringLiteral("#555"),
        QStringLiteral("#eef0ff"),
    };

    for (const QString& path : files) {
        const QString contents = readFile(path);
        for (const QString& color : forbiddenColors) {
            QVERIFY2(!contents.contains(color),
                     qPrintable(path + QStringLiteral(" must use theme tokens instead of legacy hardcoded surface/text color ") + color));
        }
    }
}

void QmlArchitectureTest::framelessGlassWindowsDoNotExposeNativeCaptionText()
{
    const QString main = readFile("src/main.cpp");
    QVERIFY2(!main.contains(QStringLiteral("settingsView.setTitle(QCoreApplication::translate(\"App\", \"CodexBar Settings\"))")),
             "SettingsWindow has a custom QML title bar; setting a native title can leak a Windows caption into the acrylic client area.");
    QVERIFY2(!main.contains(QStringLiteral("usageView.setTitle(QCoreApplication::translate(\"App\", \"Usage Details\"))")),
             "UsageWindow has a custom QML title bar; setting a native title can leak a Windows caption into the acrylic client area.");

    const QString expectedFlags = QStringLiteral("Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint");
    QVERIFY2(main.contains(QStringLiteral("settingsView.setFlags(")) && main.contains(expectedFlags),
             "SettingsWindow must explicitly customize its frameless native flags to avoid a native caption layer.");
    QVERIFY2(main.contains(QStringLiteral("usageView.setFlags(")) && main.contains(expectedFlags),
             "UsageWindow must explicitly customize its frameless native flags to avoid a native caption layer.");
}

void QmlArchitectureTest::usageStoreDoesNotInjectBridgeLookupWhenDisabled()
{
    const QString usageStore = readFile("src/app/UsageStore.cpp");
    const QString lookupCall = QStringLiteral("input.bridgeSessionLookup = m_bridgeService->sessionLookupForProvider(providerId);");
    const int lookupIndex = usageStore.indexOf(lookupCall);
    QVERIFY2(lookupIndex >= 0, "UsageStore must still support Browser Session Bridge lookup when enabled.");

    const int guardIndex = usageStore.lastIndexOf(QStringLiteral("if (m_bridgeService"), lookupIndex);
    QVERIFY2(guardIndex >= 0,
             "UsageStore must guard Browser Session Bridge lookup before calling sessionLookupForProvider().");

    const QString guardedRegion = usageStore.mid(guardIndex, lookupIndex - guardIndex + lookupCall.size());
    QVERIFY2(guardedRegion.contains(QStringLiteral("m_settingsStore")),
             "The Browser Session Bridge lookup guard must read the global SettingsStore value.");
    QVERIFY2(guardedRegion.contains(QStringLiteral("browserSessionBridgeEnabled()")),
             "UsageStore must guard Browser Session Bridge lookup with SettingsStore::browserSessionBridgeEnabled().");
}

void QmlArchitectureTest::browserSessionBridgeExtensionUsesCanonicalWireProtocol()
{
    const QString worker = readFile("resources/browser-session-bridge/service_worker.js");
    const QString protocol = readFile("resources/browser-session-bridge/protocol.js");
    const QString combined = worker + protocol;
    QVERIFY(combined.contains("'register_client'") || combined.contains("\"register_client\""));
    QVERIFY(combined.contains("'register_ack'") || combined.contains("\"register_ack\""));
    QVERIFY(combined.contains("'request_import'") || combined.contains("\"request_import\""));
    QVERIFY(combined.contains("'import_result'") || combined.contains("\"import_result\""));
    QVERIFY2(!worker.contains("'RegisterClient'") && !worker.contains("\"RegisterClient\""),
             "Extension must not use PascalCase message type names.");
    QVERIFY2(!worker.contains("'RequestImport'") && !worker.contains("\"RequestImport\""),
             "Extension must not use PascalCase message type names.");
    QVERIFY2(!worker.contains("'ImportResult'") && !worker.contains("\"ImportResult\""),
             "Extension must not use PascalCase message type names.");
    QVERIFY2(!worker.contains("materialKind === 'Cookies'") && !worker.contains("materialKind === \"Cookies\""),
             "Extension must compare materialKind against canonical 'cookies'.");
    QVERIFY2(worker.contains("capabilities"),
             "register_client payload must send capabilities.cookies/localStorage.");
}

void QmlArchitectureTest::providerUiBuildersUseCatalogSnapshot()
{
    QFile catalogHeader(QStringLiteral(PROJECT_SOURCE_DIR "/src/providers/ProviderCatalogSnapshot.h"));
    QVERIFY2(catalogHeader.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(catalogHeader.errorString()));
    const QString catalogContents = QString::fromUtf8(catalogHeader.readAll());
    QVERIFY2(catalogContents.contains(QStringLiteral("ProviderCatalogEntry")),
             "Provider catalog snapshot must expose immutable provider entries.");

    QFile usageStore(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStore.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(usageStore.errorString()));
    const QString contents = QString::fromUtf8(usageStore.readAll());
    QVERIFY2(contents.contains(QStringLiteral("ProviderCatalogSnapshot::fromRegistry")),
             "UsageStore must rebuild a ProviderCatalogSnapshot from the registry at state boundaries.");

    // Verify UsageStore delegates to ProviderUIService
    QVERIFY2(contents.contains(QStringLiteral("m_uiService->requestProviderList()")),
             "UsageStore must delegate provider list building to ProviderUIService.");
    QVERIFY2(contents.contains(QStringLiteral("m_uiService->requestProviderDescriptor")),
             "UsageStore must delegate provider descriptor building to ProviderUIService.");
    QVERIFY2(!contents.contains(QStringLiteral("m_snapshotDataCache")),
             "snapshotData cache must live in ProviderUIService, not UsageStore.");
    QVERIFY2(!contents.contains(QStringLiteral("refreshJobDispatched")),
             "Provider refresh request mapping must stay inside ProviderRefreshCoordinator.");
    QVERIFY2(!contents.contains(QStringLiteral("m_backendRequestProviderIds")),
             "UsageStore must not own the shared provider refresh backend request map.");

    // Verify ProviderUIService uses catalog snapshot
    QFile uiService(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/ProviderUIService.cpp"));
    QVERIFY2(uiService.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(uiService.errorString()));
    const QString uiServiceContents = QString::fromUtf8(uiService.readAll());

    const QString listStart = QStringLiteral("void ProviderUIService::requestProviderList()");
    const QString listEnd = QStringLiteral("void ProviderUIService::requestProviderDescriptor");
    const int listStartIndex = uiServiceContents.indexOf(listStart);
    QVERIFY2(listStartIndex >= 0, "Missing ProviderUIService::requestProviderList().");
    const int listEndIndex = uiServiceContents.indexOf(listEnd, listStartIndex + listStart.size());
    QVERIFY2(listEndIndex > listStartIndex, "Missing method after ProviderUIService::requestProviderList().");
    const QString listBody = uiServiceContents.mid(listStartIndex, listEndIndex - listStartIndex);
    QVERIFY2(!listBody.contains(QStringLiteral("ProviderRegistry::instance()")),
             "Provider list backend input must read ProviderCatalogSnapshot, not live ProviderRegistry/provider QObject.");
    QVERIFY2(listBody.contains(QStringLiteral("collectProviderListBuildItems(m_catalog")),
             "Provider list backend input must be prepared from ProviderCatalogSnapshot helper input.");
    QVERIFY2(uiServiceContents.contains(QStringLiteral("catalog->providers()")),
             "Provider list helper must iterate catalog snapshot entries.");

    const QString descriptorStart = QStringLiteral("void ProviderUIService::requestProviderDescriptor");
    const QString descriptorEnd = QStringLiteral("void ProviderUIService::invalidateProviderListCache");
    const int descriptorStartIndex = uiServiceContents.indexOf(descriptorStart);
    QVERIFY2(descriptorStartIndex >= 0, "Missing ProviderUIService::requestProviderDescriptor().");
    const int descriptorEndIndex = uiServiceContents.indexOf(descriptorEnd, descriptorStartIndex + descriptorStart.size());
    QVERIFY2(descriptorEndIndex > descriptorStartIndex, "Missing method after ProviderUIService::requestProviderDescriptor().");
    const QString descriptorBody = uiServiceContents.mid(descriptorStartIndex, descriptorEndIndex - descriptorStartIndex);
    QVERIFY2(!descriptorBody.contains(QStringLiteral("ProviderRegistry::instance()")),
             "Provider descriptor backend input must read ProviderCatalogSnapshot, not live ProviderRegistry/provider QObject.");
    QVERIFY2(!descriptorBody.contains(QStringLiteral("settingsDescriptors()")),
             "Provider descriptor backend input must use snapshotted settings descriptors.");
    QVERIFY2(descriptorBody.contains(QStringLiteral("buildProviderDescriptorInput(")),
             "Provider descriptor backend input must be prepared through the descriptor helper.");
    QVERIFY2(uiServiceContents.contains(QStringLiteral("catalog->provider(providerId)")),
             "Provider descriptor helper must look up provider metadata in the catalog snapshot.");

    QVERIFY2(!contents.contains(QStringLiteral("UsageStore::providerSettingsFields")),
             "Provider settings fields must be prepared through providerDescriptorData instead of a direct QML getter.");

    QFile bootstrap(QStringLiteral(PROJECT_SOURCE_DIR "/src/providers/ProviderBootstrap.cpp"));
    QVERIFY2(bootstrap.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(bootstrap.errorString()));
    const QString bootstrapContents = QString::fromUtf8(bootstrap.readAll());
    QVERIFY2(bootstrapContents.contains(QStringLiteral("ProviderCatalogSnapshot::fromRegistry")),
             "Provider bootstrap must use the catalog snapshot for provider default/enabled metadata.");
    QVERIFY2(!bootstrapContents.contains(QStringLiteral("provider->defaultEnabled()")),
             "Provider bootstrap must not query live provider metadata after the catalog snapshot is available.");

    QFile cliUsage(QStringLiteral(PROJECT_SOURCE_DIR "/src/cli/CLIUsageCommand.cpp"));
    QVERIFY2(cliUsage.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(cliUsage.errorString()));
    const QString cliUsageContents = QString::fromUtf8(cliUsage.readAll());
    QVERIFY2(cliUsageContents.contains(QStringLiteral("ProviderCatalogSnapshot::fromRegistry")),
             "CLI usage command must share provider metadata through ProviderCatalogSnapshot.");
    QVERIFY2(!cliUsageContents.contains(QStringLiteral("allProviders()")),
             "CLI usage command must use catalog enabled IDs instead of iterating live provider objects for metadata.");
}

void QmlArchitectureTest::costUsageScanUsesCostUsageService()
{
    QFile serviceHeader(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/CostUsageService.h"));
    QVERIFY2(serviceHeader.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(serviceHeader.errorString()));
    const QString serviceHeaderContents = QString::fromUtf8(serviceHeader.readAll());
    QVERIFY2(serviceHeaderContents.contains(QStringLiteral("CostUsageScanPlan")),
             "CostUsageService must own the cost scan plan DTO.");

    QFile serviceSource(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/CostUsageService.cpp"));
    QVERIFY2(serviceSource.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(serviceSource.errorString()));
    const QString serviceContents = QString::fromUtf8(serviceSource.readAll());
    QVERIFY2(serviceContents.contains(QStringLiteral("CostUsageScanner scanner")),
             "CostUsageService must own CostUsageScanner work.");
    QVERIFY2(serviceContents.contains(QStringLiteral("CostUsageCache::instance()")),
             "CostUsageService must own CostUsageCache loading/saving.");

    QFile usageStore(QStringLiteral(PROJECT_SOURCE_DIR "/src/app/UsageStore.cpp"));
    QVERIFY2(usageStore.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(usageStore.errorString()));
    const QString contents = QString::fromUtf8(usageStore.readAll());
    QVERIFY2(!contents.contains(QStringLiteral("#include \"../util/CostUsageScanner.h\"")),
             "UsageStore must not include CostUsageScanner directly after CostUsageService extraction.");
    QVERIFY2(!contents.contains(QStringLiteral("#include \"../util/CostUsageCache.h\"")),
             "UsageStore must not include CostUsageCache directly after CostUsageService extraction.");

    const QString refreshStart = QStringLiteral("void UsageStore::refreshCostUsage()");
    const QString refreshEnd = QStringLiteral("QVariantMap UsageStore::costUsageData()");
    const int refreshStartIndex = contents.indexOf(refreshStart);
    QVERIFY2(refreshStartIndex >= 0, "Missing UsageStore::refreshCostUsage().");
    const int refreshEndIndex = contents.indexOf(refreshEnd, refreshStartIndex + refreshStart.size());
    QVERIFY2(refreshEndIndex > refreshStartIndex, "Missing method after UsageStore::refreshCostUsage().");
    const QString refreshBody = contents.mid(refreshStartIndex, refreshEndIndex - refreshStartIndex);
    QVERIFY2(refreshBody.contains(QStringLiteral("CostUsageService::buildScanPlan")),
             "UsageStore must build cost scan plans through CostUsageService.");
    QVERIFY2(refreshBody.contains(QStringLiteral("costUsageSubscribedProviderIDs")),
             "UsageStore must pass provider subscriptions into the cost scan plan.");
    QVERIFY2(refreshBody.contains(QStringLiteral("CostUsageService::refresh")),
             "UsageStore must dispatch CostUsageService refresh work.");
    QVERIFY2(!refreshBody.contains(QStringLiteral("CostUsageScanner")),
             "UsageStore refresh path must not instantiate or reference CostUsageScanner.");
    QVERIFY2(!refreshBody.contains(QStringLiteral("CostUsageCache")),
             "UsageStore refresh path must not load/save CostUsageCache.");
    QVERIFY2(!refreshBody.contains(QStringLiteral("scanOpenCodeDB")),
             "UsageStore refresh path must not own SQLite/opencode scan details.");

    QVERIFY2(serviceContents.contains(QStringLiteral("scanOpenCodeDB(since, today, plan.openCodeDBProviderIds)")),
             "CostUsageService must pass an enabled+subscribed provider allow-list into OpenCode DB scans.");
}

void QmlArchitectureTest::openCodeCostScanScopesSqlToRecentSessions()
{
    QFile scanner(QStringLiteral(PROJECT_SOURCE_DIR "/src/util/CostUsageScanner.cpp"));
    QVERIFY2(scanner.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(scanner.errorString()));
    const QString contents = QString::fromUtf8(scanner.readAll());

    QVERIFY2(contents.contains(QStringLiteral("recent_sessions")),
             "OpenCode DB cost scan must first scope work to sessions active in the requested date range.");
    QVERIFY2(contents.contains(QStringLiteral("time_created BETWEEN :sinceMs AND :untilMs")),
             "OpenCode DB cost scan must use sinceMs in SQL instead of reading every historical message.");
    QVERIFY2(contents.contains(QStringLiteral("query.bindValue(\":sinceMs\", sinceMs)")),
             "OpenCode DB cost scan must bind sinceMs for the recent session scope.");
}

void QmlArchitectureTest::phaseZeroUiFoundationComponentsAreGuarded()
{
    const QString qrc = readFile("resources/qml.qrc");
    const QStringList requiredComponents = {
        QStringLiteral("qml/components/ProviderAvatar.qml"),
        QStringLiteral("qml/components/ErrorNotice.qml"),
        QStringLiteral("qml/components/IconButton.qml"),
        QStringLiteral("qml/components/StatusPill.qml"),
    };
    for (const QString& component : requiredComponents) {
        QVERIFY2(qrc.contains(component),
                 qPrintable(component + QStringLiteral(" must be registered in qml.qrc so Release builds cannot drop UI foundation components.")));
    }

    const QString tray = readFile("qml/TrayPanel.qml");
    QVERIFY2(tray.contains(QStringLiteral("Components.ErrorNotice")),
             "TrayPanel inline provider errors must route through ErrorNotice.");
    QVERIFY2(!tray.contains(QStringLiteral("text: snap.error")),
             "TrayPanel must not render raw snap.error directly.");
    QVERIFY2(!tray.contains(QStringLiteral("text: snap.creditsError")),
             "TrayPanel must not render raw snap.creditsError directly.");
}

void QmlArchitectureTest::providerIconsUseSharedAvatar()
{
    const QString qrc = readFile("resources/qml.qrc");
    QVERIFY2(qrc.contains(QStringLiteral("qml/components/ProviderAvatar.qml")),
             "ProviderAvatar.qml must be registered in qml.qrc so Release builds ship the shared provider icon renderer.");
    QVERIFY2(qrc.contains(QStringLiteral("qml/components/ProviderIconPolicy.qml")),
             "ProviderIconPolicy.qml must be registered in qml.qrc so Release builds ship contrast policy data.");

    const QString avatar = readFile("qml/components/ProviderAvatar.qml");
    const QString policy = readFile("qml/components/ProviderIconPolicy.qml");
    QVERIFY2(avatar.contains(QStringLiteral("ProviderIconPolicy")),
             "ProviderAvatar must centralize provider icon policy instead of duplicating contrast decisions at call sites.");
    QVERIFY2(policy.contains(QStringLiteral("darkGlyph")) && policy.contains(QStringLiteral("preserveBackground")),
             "ProviderIconPolicy must define both dark glyph icons and provider icons with their own background.");
    QVERIFY2(avatar.contains(QStringLiteral("severity")),
             "ProviderAvatar must expose a severity/state decoration point for provider errors.");

    const QStringList providerIdentityFiles = {
        QStringLiteral("qml/components/ProviderSwitcher.qml"),
        QStringLiteral("qml/components/ProviderSwitcherRow.qml"),
        QStringLiteral("qml/components/TrayProviderDock.qml"),
        QStringLiteral("qml/components/ProviderListItem.qml"),
        QStringLiteral("qml/components/ProviderDetailView.qml"),
        QStringLiteral("qml/panes/TokenUsagePane.qml"),
    };

    for (const QString& path : providerIdentityFiles) {
        const QString contents = readFile(path);
        QVERIFY2(contents.contains(QStringLiteral("ProviderAvatar"))
                     || contents.contains(QStringLiteral("TrayProviderDock"))
                     || contents.contains(QStringLiteral("UsageProviderRow")),
                 qPrintable(path + QStringLiteral(" must render provider identity through ProviderAvatar, UsageProviderRow, or delegate to TrayProviderDock.")));
        QVERIFY2(!contents.contains(QStringLiteral("ProviderIcon-")),
                 qPrintable(path + QStringLiteral(" must not construct ProviderIcon resource paths directly.")));
    }
}

void QmlArchitectureTest::uiPolishRouteBComponentsAreGuarded()
{
    const QString qrc = readFile("resources/qml.qrc");
    const QStringList requiredComponents = {
        QStringLiteral("qml/components/ProviderIconVessel.qml"),
        QStringLiteral("qml/components/SurfaceCard.qml"),
        QStringLiteral("qml/components/ActionButton.qml"),
        QStringLiteral("qml/components/InlineFeedback.qml"),
        QStringLiteral("qml/components/SkeletonBlock.qml"),
        QStringLiteral("qml/components/FocusRing.qml"),
        QStringLiteral("qml/components/TrayProviderDock.qml"),
        QStringLiteral("qml/components/UsageProviderRow.qml"),
    };

    for (const QString& component : requiredComponents) {
        QVERIFY2(qrc.contains(component),
                 qPrintable(component + QStringLiteral(" must be registered in qml.qrc for the Route B UI component refresh.")));
    }
}

void QmlArchitectureTest::actionButtonSupportsKeyboardAndAccessibleFocus()
{
    const QString button = readFile("qml/components/ActionButton.qml");

    QVERIFY2(button.contains(QStringLiteral("activeFocusOnTab")),
             "ActionButton must be reachable from keyboard tab navigation.");
    QVERIFY2(button.contains(QStringLiteral("Keys.onReturnPressed")) || button.contains(QStringLiteral("Keys.onEnterPressed")),
             "ActionButton must activate from Enter/Return, not only pointer taps.");
    QVERIFY2(button.contains(QStringLiteral("Keys.onSpacePressed")),
             "ActionButton must activate from Space like a normal button.");
    QVERIFY2(button.contains(QStringLiteral("Accessible.role")),
             "ActionButton must expose button semantics to accessibility tools.");
    QVERIFY2(button.contains(QStringLiteral("Accessible.name")),
             "ActionButton must expose an accessible name.");
    QVERIFY2(button.contains(QStringLiteral("FocusRing")),
             "ActionButton must render a visible focus ring when focused.");
}

void QmlArchitectureTest::providerErrorsUseSharedErrorNotice()
{
    const QString detailCard = readFile("qml/components/ProviderDetailCard.qml");
    const QString detailView = readFile("qml/components/ProviderDetailView.qml");
    const QString browserCard = readFile("qml/components/BrowserSessionCard.qml");
    const QString providerErrorCard = readFile("qml/components/ProviderErrorCard.qml");

    QVERIFY2(detailCard.contains(QStringLiteral("ErrorNotice")),
             "ProviderDetailCard must use ErrorNotice for provider and credits errors.");
    QVERIFY2(!detailCard.contains(QStringLiteral("text: snap.error")),
             "ProviderDetailCard must not render snap.error directly as Text.");
    QVERIFY2(!detailCard.contains(QStringLiteral("text: snap.creditsError")),
             "ProviderDetailCard must not render snap.creditsError directly as Text.");
    QVERIFY2(detailView.contains(QStringLiteral("ErrorNotice")) || providerErrorCard.contains(QStringLiteral("ErrorNotice")),
             "ProviderDetailView errors must render through ErrorNotice or a ProviderErrorCard wrapper.");
    QVERIFY2(browserCard.contains(QStringLiteral("copyable: true")) || browserCard.contains(QStringLiteral("ErrorNotice")),
             "BrowserSessionCard import errors must expose a nearby copy affordance.");
}

void QmlArchitectureTest::sharedControlsDoNotKeepLocalDuplicates()
{
    const QString qrc = readFile("resources/qml.qrc");
    const QString tray = readFile("qml/TrayPanel.qml");
    const QString detailCard = readFile("qml/components/ProviderDetailCard.qml");
    const QString detailView = readFile("qml/components/ProviderDetailView.qml");
    const QString tokenUsage = readFile("qml/panes/TokenUsagePane.qml");
    const QString providerListItem = readFile("qml/components/ProviderListItem.qml");

    QVERIFY2(qrc.contains(QStringLiteral("qml/components/StatusDot.qml")),
             "StatusDot must be registered in qml.qrc once status dots are shared.");
    QVERIFY2(!tray.contains(QStringLiteral("component ActionButton")),
             "TrayPanel must use Components.ActionButton instead of a local ActionButton.");
    QVERIFY2(!detailCard.contains(QStringLiteral("component ActionButton")),
             "ProviderDetailCard must use shared ActionButton instead of a local ActionButton.");
    QVERIFY2(!detailView.contains(QStringLiteral("component StatusPill")),
             "ProviderDetailView must use shared StatusPill instead of a local StatusPill.");
    QVERIFY2(!tokenUsage.contains(QStringLiteral("component StatusPill")),
             "TokenUsagePane must use shared StatusPill instead of a local StatusPill.");
    QVERIFY2(!tokenUsage.contains(QStringLiteral("component SmallButton")),
             "TokenUsagePane must use shared ActionButton instead of a local SmallButton.");
    QVERIFY2(providerListItem.contains(QStringLiteral("StatusDot")),
             "ProviderListItem must render provider state through shared StatusDot.");
}

void QmlArchitectureTest::trayProviderDockSupportsKeyboardAndAccessibility()
{
    const QString dock = readFile("qml/components/TrayProviderDock.qml");

    QVERIFY2(dock.contains(QStringLiteral("Accessible.role")),
             "TrayProviderDock items must expose button semantics.");
    QVERIFY2(dock.contains(QStringLiteral("Accessible.name")),
             "TrayProviderDock items must expose provider names to accessibility tools.");
    QVERIFY2(dock.contains(QStringLiteral("activeFocusOnTab")),
             "TrayProviderDock items must be keyboard focusable.");
    QVERIFY2(dock.contains(QStringLiteral("Keys.onLeftPressed")) && dock.contains(QStringLiteral("Keys.onRightPressed")),
             "TrayProviderDock must support left/right keyboard provider selection.");
    QVERIFY2(dock.contains(QStringLiteral("Keys.onReturnPressed")) || dock.contains(QStringLiteral("Keys.onEnterPressed")),
             "TrayProviderDock must support Enter/Return activation.");
    QVERIFY2(dock.contains(QStringLiteral("Keys.onSpacePressed")),
             "TrayProviderDock must support Space activation.");
    QVERIFY2(dock.contains(QStringLiteral("FocusRing")),
             "TrayProviderDock must render focus visibly.");
}

void QmlArchitectureTest::tokenUsagePaneUsesProductionUsageProviderRow()
{
    const QString qrc = readFile("resources/qml.qrc");
    const QString tokenUsage = readFile("qml/panes/TokenUsagePane.qml");
    const QString usageRow = readFile("qml/components/UsageProviderRow.qml");

    QVERIFY2(qrc.contains(QStringLiteral("qml/components/UsageProviderRow.qml")),
             "UsageProviderRow must stay registered in qml.qrc when TokenUsagePane uses it in production.");
    QVERIFY2(tokenUsage.contains(QStringLiteral("UsageProviderRow")),
             "TokenUsagePane must use UsageProviderRow instead of keeping it as a qrc/smoke-only component.");
    QVERIFY2(!tokenUsage.contains(QStringLiteral("ProviderAvatar {")),
             "TokenUsagePane provider row identity must live in UsageProviderRow, not a duplicated local header.");
    QVERIFY2(usageRow.contains(QStringLiteral("property var provider")),
             "UsageProviderRow must accept the production provider row object.");
    QVERIFY2(usageRow.contains(QStringLiteral("signal toggleRequested")),
             "UsageProviderRow must expose a single toggle signal for expandable provider details.");
    QVERIFY2(usageRow.contains(QStringLiteral("ProviderAvatar")) && usageRow.contains(QStringLiteral("StatusPill")),
             "UsageProviderRow must own the shared avatar and status pill language for token usage provider rows.");
    QVERIFY2(usageRow.contains(QStringLiteral("Accessible.name")) && usageRow.contains(QStringLiteral("Keys.onSpacePressed")),
             "UsageProviderRow must be keyboard and accessibility ready when it handles expansion.");
}

void QmlArchitectureTest::finalUiPolishGuardsStayInPlace()
{
    const QString settings = readFile("qml/SettingsWindow.qml");
    const QString tokenUsage = readFile("qml/panes/TokenUsagePane.qml");

    const QStringList forbiddenSettingsGlyphs = {
        QStringLiteral("⚙"),
        QStringLiteral("☁"),
        QStringLiteral("🖵"),
        QStringLiteral("🛠"),
        QStringLiteral("🛈"),
        QStringLiteral("🐞"),
        QStringLiteral("🗗"),
        QStringLiteral("🗖"),
    };
    for (const QString& glyph : forbiddenSettingsGlyphs) {
        QVERIFY2(!settings.contains(glyph),
                 qPrintable(QStringLiteral("SettingsWindow must not rely on emoji/symbol glyph %1 for navigation or title actions.").arg(glyph)));
    }

    QVERIFY2(settings.contains(QStringLiteral("Canvas")),
             "SettingsWindow title buttons must draw stable window symbols instead of depending on emoji glyph availability.");
    QVERIFY2(settings.contains(QStringLiteral("activeFocusOnTab")) && settings.contains(QStringLiteral("Accessible.name")),
             "SettingsWindow navigation and title actions must remain keyboard/accessibility reachable.");
    QVERIFY2(tokenUsage.contains(QStringLiteral("updatedAt")) && tokenUsage.contains(QStringLiteral("formatUpdatedAt")),
             "TokenUsagePane overview must expose a data update timestamp for Release QA.");
    QVERIFY2(tokenUsage.contains(QStringLiteral("UsageProviderRow")),
             "TokenUsagePane final layout must keep the production UsageProviderRow integration.");
}

void QmlArchitectureTest::scrollBarsAvoidPermanentActiveState()
{
    const QStringList scrollFiles = {
        QStringLiteral("qml/TrayPanel.qml"),
        QStringLiteral("qml/panes/TokenUsagePane.qml"),
        QStringLiteral("qml/panes/ProvidersPane.qml"),
        QStringLiteral("qml/components/ProviderDetailView.qml"),
        QStringLiteral("qml/components/SettingsPage.qml"),
    };

    for (const QString& path : scrollFiles) {
        const QString contents = readFile(path);
        const QRegularExpression pinnedActive(QStringLiteral("\\bactive\\s*:\\s*true\\b"));
        QVERIFY2(!contents.contains(pinnedActive),
                 qPrintable(path + QStringLiteral(" must not pin ScrollBar.active to true; scrollbars should fade when idle.")));
        QVERIFY2(contents.contains(QStringLiteral("moving")) || contents.contains(QStringLiteral("flicking")),
                 qPrintable(path + QStringLiteral(" must keep scrollbars discoverable while the view is scrolling or flicking.")));

        const auto countMatches = [](const QRegularExpression& expression, const QString& text) {
            int count = 0;
            auto it = expression.globalMatch(text);
            while (it.hasNext()) {
                it.next();
                ++count;
            }
            return count;
        };
        const QRegularExpression verticalScrollBars(QStringLiteral("ScrollBar\\.vertical\\s*:\\s*ScrollBar"));
        const QRegularExpression activeOpacity(QStringLiteral("opacity\\s*:\\s*\\w+ScrollBar\\.active\\s*\\?\\s*1(?:\\.0)?\\s*:\\s*0(?:\\.0)?"));
        const int scrollBarCount = countMatches(verticalScrollBars, contents);
        const int opacityBindingCount = countMatches(activeOpacity, contents);
        QVERIFY2(opacityBindingCount >= scrollBarCount,
                 qPrintable(path + QStringLiteral(" custom ScrollBar contentItem must bind opacity to ScrollBar.active so idle scrollbars are not visible.")));
    }
}

void QmlArchitectureTest::appThemeExposesInteractionPolishTokens()
{
    const QString theme = readFile("qml/AppTheme.qml");
    const QStringList requiredTokens = {
        QStringLiteral("property color surfaceInteractive"),
        QStringLiteral("property color surfaceInteractiveHover"),
        QStringLiteral("property color surfaceInteractivePressed"),
        QStringLiteral("property color surfaceFloating"),
        QStringLiteral("property color surfaceInput"),
        QStringLiteral("property color surfaceDangerSoft"),
        QStringLiteral("property color surfaceWarningSoft"),
        QStringLiteral("property color surfaceSuccessSoft"),
        QStringLiteral("property color borderSubtle"),
        QStringLiteral("property color borderStrong"),
        QStringLiteral("property color borderFocus"),
        QStringLiteral("property int motionFast"),
        QStringLiteral("property int motionNormal"),
        QStringLiteral("property int motionSlow"),
        QStringLiteral("property int motionPanel"),
        QStringLiteral("property int avatarSizeDock"),
        QStringLiteral("property int avatarSizeList"),
        QStringLiteral("property int avatarSizeHero"),
        QStringLiteral("function duration("),
    };

    for (const QString& token : requiredTokens) {
        QVERIFY2(theme.contains(token),
                 qPrintable(QStringLiteral("AppTheme.qml must expose shared interaction polish token: %1").arg(token)));
    }
}

void QmlArchitectureTest::providerAvatarUsesPolicyDrivenIconVessel()
{
    const QString avatar = readFile("qml/components/ProviderAvatar.qml");
    const QString vessel = readFile("qml/components/ProviderIconVessel.qml");
    const QString policy = readFile("qml/components/ProviderIconPolicy.qml");

    QVERIFY2(avatar.contains(QStringLiteral("ProviderIconVessel")),
             "ProviderAvatar must delegate shape/background/ring rendering to ProviderIconVessel.");
    QVERIFY2(!avatar.contains(QStringLiteral("Rectangle {\n        id: surface")),
             "ProviderAvatar must not own the icon surface rectangle after the Route B refresh.");

    const QStringList policyFields = {
        QStringLiteral("\"shape\""),
        QStringLiteral("\"background\""),
        QStringLiteral("\"imageMode\""),
        QStringLiteral("\"selectedTreatment\""),
        QStringLiteral("\"smallSizeFallback\""),
    };
    for (const QString& field : policyFields) {
        QVERIFY2(policy.contains(field),
                 qPrintable(QStringLiteral("ProviderIconPolicy must expose policy field %1.").arg(field)));
    }

    QVERIFY2(vessel.contains(QStringLiteral("property string density")),
             "ProviderIconVessel must support compact/normal/hero density.");
    QVERIFY2(vessel.contains(QStringLiteral("shapeRadius")),
             "ProviderIconVessel must compute shape-specific radius instead of forcing every logo into one circle.");
    QVERIFY2(vessel.contains(QStringLiteral("imageMode")),
             "ProviderIconVessel must respect imageMode so native rectangular logos are not shoved into circular containers.");
}

void QmlArchitectureTest::providerSwitcherUsesAvatarDockPattern()
{
    const QString switcher = readFile("qml/components/ProviderSwitcher.qml");
    const QString dock = readFile("qml/components/TrayProviderDock.qml");

    QVERIFY2(switcher.contains(QStringLiteral("TrayProviderDock")),
             "ProviderSwitcher must become a compatibility wrapper around the new avatar dock.");
    QVERIFY2(!switcher.contains(QStringLiteral("width: 72")),
             "ProviderSwitcher must not keep the old text-tab delegate width.");
    QVERIFY2(!switcher.contains(QStringLiteral("font.pixelSize: 10")),
             "ProviderSwitcher must not render tiny always-visible labels in each dock item.");
    QVERIFY2(dock.contains(QStringLiteral("ProviderAvatar")),
             "TrayProviderDock must render provider identity through ProviderAvatar.");
    QVERIFY2(dock.contains(QStringLiteral("ToolTip")),
             "TrayProviderDock must move provider names/status into delayed tooltips.");
    QVERIFY2(dock.contains(QStringLiteral("showProgressRing")),
             "TrayProviderDock must expose usage through the avatar ring instead of a cramped mini text tab.");
    QVERIFY2(dock.contains(QStringLiteral("WheelHandler")) || dock.contains(QStringLiteral("onWheel")),
             "TrayProviderDock must support horizontal wheel scrolling for many providers.");
}

void QmlArchitectureTest::appQmlResourceBypassesMultiConfigAutoRcc()
{
    const QString cmake = readFile(QStringLiteral("CMakeLists.txt"));
    QVERIFY2(cmake.contains(QStringLiteral("CODEXBAR_QML_RESOURCE_CPP")),
             "CodexBarX must generate a single config-independent QML resource source.");
    QVERIFY2(cmake.contains(QStringLiteral("$<TARGET_FILE:Qt6::rcc>")),
             "CodexBarX must invoke rcc directly instead of relying on multi-config AUTORCC for app QML.");

    const int appSources = cmake.indexOf(QStringLiteral("set(CODEXBAR_APP_SOURCES"));
    QVERIFY(appSources >= 0);
    const int addExecutable = cmake.indexOf(QStringLiteral("add_executable(CodexBarX"), appSources);
    QVERIFY(addExecutable > appSources);
    const QString appSourceBlock = cmake.mid(appSources, addExecutable - appSources);
    QVERIFY2(!appSourceBlock.contains(QStringLiteral("resources/qml.qrc")),
             "Do not put resources/qml.qrc directly in CODEXBAR_APP_SOURCES; Visual Studio multi-config AUTORCC can leave Release QML stale after Debug refreshed the shared wrapper.");
}

QTEST_MAIN(QmlArchitectureTest)

#include "tst_QmlArchitecture.moc"

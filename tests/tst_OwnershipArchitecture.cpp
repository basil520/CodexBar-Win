#include <QtTest/QtTest>

#include "providers/IProvider.h"
#include "providers/ProviderRegistry.h"

#include <QFile>
#include <QString>
#include <utility>

namespace {

QString readProjectFile(const QString& relativePath)
{
    QFile file(QStringLiteral(PROJECT_SOURCE_DIR) + QLatin1Char('/') + relativePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

class CountingProvider : public IProvider {
    Q_OBJECT
public:
    explicit CountingProvider(QString providerId)
        : m_providerId(std::move(providerId))
    {}

    ~CountingProvider() override
    {
        ++destroyedCount;
    }

    QString id() const override { return m_providerId; }
    QString displayName() const override { return QStringLiteral("Counting Provider"); }
    QString sessionLabel() const override { return QStringLiteral("Session"); }
    QString weeklyLabel() const override { return QStringLiteral("Weekly"); }
    QVector<IFetchStrategy*> createStrategies(const ProviderFetchContext&) override { return {}; }
    bool defaultEnabled() const override { return false; }

    static int destroyedCount;

private:
    QString m_providerId;
};

int CountingProvider::destroyedCount = 0;

} // namespace

class tst_OwnershipArchitecture : public QObject {
    Q_OBJECT

private slots:
    void providerRegistryReleasesDuplicateProvider();
    void networkManagerDeletesSynchronousRepliesImmediately();
    void statusItemControllerOwnsTrayIconRenderer();
    void appSingletonStoresHaveQObjectOwners();
    void cliBackendUsesScopedStores();
};

void tst_OwnershipArchitecture::providerRegistryReleasesDuplicateProvider()
{
    CountingProvider::destroyedCount = 0;
    auto* first = new CountingProvider(QStringLiteral("ownership-duplicate-provider"));
    auto* second = new CountingProvider(QStringLiteral("ownership-duplicate-provider"));

    ProviderRegistry::instance().registerProvider(first);
    ProviderRegistry::instance().registerProvider(second);

    QCOMPARE(ProviderRegistry::instance().provider(QStringLiteral("ownership-duplicate-provider")), second);
    QCOMPARE(CountingProvider::destroyedCount, 1);
}

void tst_OwnershipArchitecture::networkManagerDeletesSynchronousRepliesImmediately()
{
    const QString source = readProjectFile(QStringLiteral("src/network/NetworkManager.cpp"));
    QVERIFY2(!source.isEmpty(), "NetworkManager.cpp must be readable.");
    QVERIFY2(!source.contains(QStringLiteral("reply->deleteLater();")),
             "Synchronous NetworkManager helpers must not rely on deleteLater() after a local event loop exits.");
    QVERIFY2(source.contains(QStringLiteral("delete reply;")),
             "Synchronous NetworkManager helpers should delete completed replies immediately.");
}

void tst_OwnershipArchitecture::statusItemControllerOwnsTrayIconRenderer()
{
    const QString header = readProjectFile(QStringLiteral("src/tray/StatusItemController.h"));
    QVERIFY2(!header.isEmpty(), "StatusItemController.h must be readable.");
    QVERIFY2(header.contains(QStringLiteral("std::unique_ptr<TrayIconRenderer>"))
             || header.contains(QStringLiteral("TrayIconRenderer m_renderer")),
             "StatusItemController must express ownership of TrayIconRenderer without a leaking raw pointer.");
}

void tst_OwnershipArchitecture::appSingletonStoresHaveQObjectOwners()
{
    const QString source = readProjectFile(QStringLiteral("src/main.cpp"));
    QVERIFY2(!source.isEmpty(), "main.cpp must be readable.");
    QVERIFY2(source.contains(QStringLiteral("new SettingsStore(&app)")),
             "The GUI SettingsStore singleton must have QApplication ownership.");
    QVERIFY2(source.contains(QStringLiteral("new UsageStore(&app)")),
             "The GUI UsageStore singleton must have QApplication ownership.");
}

void tst_OwnershipArchitecture::cliBackendUsesScopedStores()
{
    const QString source = readProjectFile(QStringLiteral("src/cli/CLIEntry.cpp"));
    QVERIFY2(!source.isEmpty(), "CLIEntry.cpp must be readable.");
    QVERIFY2(!source.contains(QStringLiteral("new SettingsStore()")),
             "CLI backend initialization should not leak a heap SettingsStore.");
    QVERIFY2(!source.contains(QStringLiteral("new UsageStore()")),
             "CLI backend initialization should not leak a heap UsageStore.");
}

QTEST_MAIN(tst_OwnershipArchitecture)
#include "tst_OwnershipArchitecture.moc"

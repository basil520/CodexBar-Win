#include <QtTest/QtTest>

#include <QDir>
#include <QFile>

class tst_MacOSAdaptationArchitecture : public QObject {
    Q_OBJECT

private slots:
    void windowsRegistryUsageIsCentralized();
    void binaryLocatorHasMacCommandSearchPaths();
    void cliCaptureIsNotConPtyGatedOnMac();
    void providersExposeMacLocalPathsAndPortablePortProbe();
    void qmlAvoidsWindowsOnlyUserFacingCopy();
    void macBundleMetadataIsConfigured();

private:
    QString readProjectFile(const QString& relativePath) const;
};

QString tst_MacOSAdaptationArchitecture::readProjectFile(const QString& relativePath) const
{
    const QString path = QDir(QStringLiteral(PROJECT_SOURCE_DIR)).filePath(relativePath);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTest::qFail(qPrintable(QStringLiteral("Could not open %1").arg(path)), __FILE__, __LINE__);
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

void tst_MacOSAdaptationArchitecture::windowsRegistryUsageIsCentralized()
{
    const QString settingsStore = readProjectFile(QStringLiteral("src/app/SettingsStore.cpp"));
    const QString bootstrap = readProjectFile(QStringLiteral("src/providers/ProviderBootstrap.cpp"));
    const QString authority = readProjectFile(QStringLiteral("src/providers/codex/CodexDashboardAuthorityContext.cpp"));
    const QString platformSettings = readProjectFile(QStringLiteral("src/app/PlatformSettings.cpp"));

    QVERIFY2(!settingsStore.contains(QStringLiteral("HKEY_CURRENT_USER")),
             "SettingsStore must use PlatformSettings instead of hard-coded registry paths.");
    QVERIFY2(!bootstrap.contains(QStringLiteral("HKEY_CURRENT_USER")),
             "ProviderBootstrap must use PlatformSettings instead of hard-coded registry paths.");
    QVERIFY2(!authority.contains(QStringLiteral("HKEY_CURRENT_USER")),
             "CodexDashboardAuthorityContext must use PlatformSettings instead of hard-coded registry paths.");
    QVERIFY2(platformSettings.contains(QStringLiteral("HKEY_CURRENT_USER\\\\Software\\\\CodexBar")),
             "The legacy Windows registry key should be centralized in PlatformSettings.");
}

void tst_MacOSAdaptationArchitecture::binaryLocatorHasMacCommandSearchPaths()
{
    const QString source = readProjectFile(QStringLiteral("src/util/BinaryLocator.cpp"));

    QVERIFY(source.contains(QStringLiteral("/opt/homebrew/bin")));
    QVERIFY(source.contains(QStringLiteral("/usr/local/bin")));
    QVERIFY(source.contains(QStringLiteral("/.npm-global/bin")));
    QVERIFY(source.contains(QStringLiteral("/.bun/bin")));
    QVERIFY(source.contains(QStringLiteral("/.codex/bin")));
    QVERIFY(source.contains(QStringLiteral("Q_OS_WIN")));
    QVERIFY(source.contains(QStringLiteral("findExecutable(name)")));
}

void tst_MacOSAdaptationArchitecture::cliCaptureIsNotConPtyGatedOnMac()
{
    const QString codexProvider = readProjectFile(QStringLiteral("src/providers/codex/CodexProvider.cpp"));
    const QString codexStatus = readProjectFile(QStringLiteral("src/providers/codex/CodexStatusProbe.cpp"));
    const QString claudeCli = readProjectFile(QStringLiteral("src/providers/claude/ClaudeCLISession.cpp"));
    const QString conpty = readProjectFile(QStringLiteral("src/providers/shared/ConPTYSession.h"));

    QVERIFY(!codexProvider.contains(QStringLiteral("ConPTY is not available")));
    QVERIFY(!codexStatus.contains(QStringLiteral("ConPTY is not available")));
    QVERIFY(!claudeCli.contains(QStringLiteral("ConPTY is not available")));
    QVERIFY(conpty.contains(QStringLiteral("isTerminalCaptureAvailable")));
    QVERIFY(codexProvider.contains(QStringLiteral("CLI terminal capture failed")));
    QVERIFY(codexStatus.contains(QStringLiteral("CLI terminal capture failed")));
    QVERIFY(claudeCli.contains(QStringLiteral("CLI terminal capture failed")));
}

void tst_MacOSAdaptationArchitecture::providersExposeMacLocalPathsAndPortablePortProbe()
{
    const QString antigravity = readProjectFile(QStringLiteral("src/providers/antigravity/AntigravityProvider.cpp"));
    const QString windsurf = readProjectFile(QStringLiteral("src/providers/windsurf/WindsurfProvider.cpp"));
    const QString jetbrains = readProjectFile(QStringLiteral("src/providers/jetbrains/JetBrainsProvider.cpp"));

    QVERIFY(antigravity.contains(QStringLiteral("ps")));
    QVERIFY(antigravity.contains(QStringLiteral("QTcpSocket")));
    QVERIFY(!antigravity.contains(QStringLiteral("Test-NetConnection")));
    QVERIFY(windsurf.contains(QStringLiteral("Library/Application Support/Windsurf/User/globalStorage/state.vscdb")));
    QVERIFY(jetbrains.contains(QStringLiteral("Library/Application Support/JetBrains")));
}

void tst_MacOSAdaptationArchitecture::qmlAvoidsWindowsOnlyUserFacingCopy()
{
    const QString about = readProjectFile(QStringLiteral("qml/panes/AboutPane.qml"));
    const QString general = readProjectFile(QStringLiteral("qml/panes/GeneralPane.qml"));
    const QString glass = readProjectFile(QStringLiteral("qml/components/display/GlassEffectCard.qml"));
    const QString secretInput = readProjectFile(QStringLiteral("qml/components/SecretInput.qml"));

    QVERIFY(!about.contains(QStringLiteral("Windows system tray")));
    QVERIFY(!general.contains(QStringLiteral("Windows starts")));
    QVERIFY(!glass.contains(QStringLiteral("native Windows acrylic")));
    QVERIFY(secretInput.contains(QStringLiteral("PlatformSettings.secureStoreDisplayName")));
}

void tst_MacOSAdaptationArchitecture::macBundleMetadataIsConfigured()
{
    const QString cmake = readProjectFile(QStringLiteral("CMakeLists.txt"));
    const QString plist = readProjectFile(QStringLiteral("resources/macos/Info.plist.in"));

    QVERIFY(cmake.contains(QStringLiteral("MACOSX_BUNDLE_INFO_PLIST")));
    QVERIFY(cmake.contains(QStringLiteral("-framework AppKit")));
    QVERIFY(cmake.contains(QStringLiteral("-framework Foundation")));
    QVERIFY(plist.contains(QStringLiteral("LSUIElement")));
    QVERIFY(plist.contains(QStringLiteral("<true/>")));
    QVERIFY(plist.contains(QStringLiteral("CFBundleIdentifier")));
    QVERIFY(plist.contains(QStringLiteral("com.codexbarx.app")));
}

QTEST_MAIN(tst_MacOSAdaptationArchitecture)
#include "tst_MacOSAdaptationArchitecture.moc"

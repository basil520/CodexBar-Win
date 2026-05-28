#include <QtTest/QtTest>

#include "../src/app/LaunchAtLoginController.h"
#include "../src/app/PlatformSettings.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>

class tst_PlatformSupport : public QObject {
    Q_OBJECT

private slots:
    void secureStoreDisplayNameIsPlatformAware();
    void appSettingsUseExpectedNativeScope();
    void macLaunchAgentCanBeWrittenToOverrideDirectory();
};

void tst_PlatformSupport::secureStoreDisplayNameIsPlatformAware()
{
#ifdef Q_OS_WIN
    QCOMPARE(PlatformSettings::secureStoreDisplayName(),
             QStringLiteral("Windows Credential Manager"));
#elif defined(Q_OS_MACOS)
    QCOMPARE(PlatformSettings::secureStoreDisplayName(),
             QStringLiteral("macOS Keychain"));
#else
    QCOMPARE(PlatformSettings::secureStoreDisplayName(),
             QStringLiteral("secure credential store"));
#endif
}

void tst_PlatformSupport::appSettingsUseExpectedNativeScope()
{
    QSettings settings = PlatformSettings::appSettings();
    QCOMPARE(settings.format(), QSettings::NativeFormat);

#ifdef Q_OS_WIN
    QVERIFY2(settings.fileName().contains(QStringLiteral("HKEY_CURRENT_USER")),
             qPrintable(settings.fileName()));
#else
    QCOMPARE(settings.organizationName(), QStringLiteral("CodexBarX"));
    QCOMPARE(settings.applicationName(), QStringLiteral("CodexBarX"));
#endif
}

void tst_PlatformSupport::macLaunchAgentCanBeWrittenToOverrideDirectory()
{
#ifdef Q_OS_MACOS
    QTemporaryDir temp;
    QVERIFY(temp.isValid());
    qputenv("CODEXBARX_LAUNCH_AGENT_DIR", QDir::toNativeSeparators(temp.path()).toUtf8());

    QString error;
    QVERIFY2(LaunchAtLoginController::setEnabled(true, &error), qPrintable(error));
    const QString path = LaunchAtLoginController::launchAgentPath();
    QVERIFY2(QFile::exists(path), qPrintable(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString plist = QString::fromUtf8(file.readAll());
    QVERIFY(plist.contains(QStringLiteral("com.codexbarx.app")));
    QVERIFY(plist.contains(QStringLiteral("ProgramArguments")));
    QVERIFY(plist.contains(QStringLiteral("RunAtLoad")));
    QVERIFY(plist.contains(QCoreApplication::applicationFilePath()));

    QVERIFY2(LaunchAtLoginController::setEnabled(false, &error), qPrintable(error));
    QVERIFY(!QFile::exists(path));
    qunsetenv("CODEXBARX_LAUNCH_AGENT_DIR");
#else
    QSKIP("LaunchAgent plist is macOS-specific.");
#endif
}

QTEST_MAIN(tst_PlatformSupport)
#include "tst_PlatformSupport.moc"

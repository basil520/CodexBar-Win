#include "LaunchAtLoginController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>

namespace {

#ifdef Q_OS_WIN
QSettings runRegistry()
{
    return QSettings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     QSettings::NativeFormat);
}
#endif

#ifdef Q_OS_MACOS
QString xmlEscaped(QString value)
{
    value.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    value.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    value.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    value.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    value.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    return value;
}

QString launchAgentDirectory()
{
    const QString overrideDir =
        QDir::fromNativeSeparators(qEnvironmentVariable("CODEXBARX_LAUNCH_AGENT_DIR")).trimmed();
    if (!overrideDir.isEmpty()) return overrideDir;
    return QDir::homePath() + QStringLiteral("/Library/LaunchAgents");
}

QString launchAgentPlist(const QString& executablePath)
{
    return QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
        "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        "<plist version=\"1.0\">\n"
        "<dict>\n"
        "  <key>Label</key>\n"
        "  <string>com.codexbarx.app</string>\n"
        "  <key>ProgramArguments</key>\n"
        "  <array>\n"
        "    <string>%1</string>\n"
        "  </array>\n"
        "  <key>RunAtLoad</key>\n"
        "  <true/>\n"
        "</dict>\n"
        "</plist>\n").arg(xmlEscaped(executablePath));
}
#endif

} // namespace

bool LaunchAtLoginController::isEnabled()
{
#ifdef Q_OS_WIN
    auto settings = runRegistry();
    return settings.contains(QStringLiteral("CodexBarX"));
#elif defined(Q_OS_MACOS)
    return QFile::exists(launchAgentPath());
#else
    return false;
#endif
}

bool LaunchAtLoginController::setEnabled(bool enabled, QString* errorMessage)
{
    if (errorMessage) errorMessage->clear();

#ifdef Q_OS_WIN
    auto settings = runRegistry();
    if (enabled) {
        settings.setValue(QStringLiteral("CodexBarX"),
                          QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        settings.remove(QStringLiteral("CodexBarX"));
    }
    return settings.status() == QSettings::NoError;
#elif defined(Q_OS_MACOS)
    const QString path = launchAgentPath();
    if (!enabled) {
        if (!QFile::exists(path)) return true;
        if (QFile::remove(path)) return true;
        if (errorMessage) *errorMessage = QStringLiteral("Could not remove launch agent: %1").arg(path);
        return false;
    }

    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not create launch agent directory: %1")
                .arg(info.absolutePath());
        }
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not write launch agent: %1").arg(path);
        }
        return false;
    }
    QTextStream stream(&file);
    stream << launchAgentPlist(QCoreApplication::applicationFilePath());
    file.close();
    return true;
#else
    if (enabled && errorMessage) {
        *errorMessage = QStringLiteral("Launch at login is not supported on this platform.");
    }
    return !enabled;
#endif
}

QString LaunchAtLoginController::launchAgentPath()
{
#ifdef Q_OS_MACOS
    return launchAgentDirectory() + QStringLiteral("/com.codexbarx.app.plist");
#else
    return {};
#endif
}

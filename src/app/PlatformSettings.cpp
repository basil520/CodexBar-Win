#include "PlatformSettings.h"

PlatformSettings::PlatformSettings(QObject* parent)
    : QObject(parent)
{
}

QSettings PlatformSettings::appSettings()
{
#ifdef Q_OS_WIN
    return QSettings(QStringLiteral("HKEY_CURRENT_USER\\Software\\CodexBar"),
                     QSettings::NativeFormat);
#else
    return QSettings(QSettings::NativeFormat,
                     QSettings::UserScope,
                     QStringLiteral("CodexBarX"),
                     QStringLiteral("CodexBarX"));
#endif
}

QString PlatformSettings::secureStoreDisplayName()
{
#ifdef Q_OS_WIN
    return QStringLiteral("Windows Credential Manager");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("macOS Keychain");
#else
    return QStringLiteral("secure credential store");
#endif
}

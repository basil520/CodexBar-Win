#include "SettingsStore.h"
#include "LaunchAtLoginController.h"
#include "PlatformSettings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QSettings>
#include <QtGlobal>

SettingsStore::SettingsStore(QObject* parent)
    : QObject(parent)
    , m_settings(PlatformSettings::appSettings())
{
    m_refreshFrequency = m_settings.value("refreshFrequency", 5).toInt();
    m_launchAtLogin = m_settings.value("launchAtLogin", false).toBool();
    m_checkForUpdates = m_settings.value("checkForUpdates", true).toBool();
    m_debugMenuEnabled = m_settings.value("debugMenuEnabled", false).toBool();
    m_codexVerboseLogging = m_settings.value("codexVerboseLogging", false).toBool();
    m_codexWebDebugDumpHTML = m_settings.value("codexWebDebugDumpHTML", false).toBool();
    m_mergeIcons = m_settings.value("mergeIcons", true).toBool();
    m_statusChecksEnabled = m_settings.value("statusChecksEnabled", true).toBool();
    m_usageBarsShowUsed = m_settings.value("usageBarsShowUsed", false).toBool();
    m_resetTimesShowAbsolute = m_settings.value("resetTimesShowAbsolute", false).toBool();
    m_showOptionalCreditsAndExtraUsage = m_settings.value("showOptionalCreditsAndExtraUsage", true).toBool();
    m_sessionQuotaNotificationsEnabled = m_settings.value("sessionQuotaNotificationsEnabled", true).toBool();
    m_claudePeakHoursEnabled = m_settings.value("claudePeakHoursEnabled", true).toBool();
#if defined(Q_OS_MACOS)
    const bool defaultBrowserSessionBridgeEnabled = false;
#else
    const bool defaultBrowserSessionBridgeEnabled = true;
#endif
    m_browserSessionBridgeEnabled = m_settings.value("browserSessionBridgeEnabled", defaultBrowserSessionBridgeEnabled).toBool();
    m_glassEffectEnabled = m_settings.value("glassEffectEnabled", true).toBool();
    m_glassEffectOpacity = qBound(5, m_settings.value("glassEffectOpacity", 40).toInt(), 95);
    m_reduceMotion = m_settings.value("reduceMotion", false).toBool();
    m_visualEffectsQuality = m_settings.value("visualEffectsQuality", "balanced").toString();
    if (m_visualEffectsQuality != QStringLiteral("high")
        && m_visualEffectsQuality != QStringLiteral("balanced")
        && m_visualEffectsQuality != QStringLiteral("low")) {
        m_visualEffectsQuality = QStringLiteral("balanced");
    }
    m_language = m_settings.value("language", "en").toString();
    m_theme = m_settings.value("theme", 0).toInt();
    m_trayDisplayMode = m_settings.value("trayDisplayMode", 0).toInt();
    m_warningThreshold = m_settings.value("warningThreshold", 20).toInt();
    m_criticalThreshold = m_settings.value("criticalThreshold", 10).toInt();
    loadConfig();

    m_saveDelayTimer.setSingleShot(true);
    m_saveDelayTimer.setInterval(500);
    connect(&m_saveDelayTimer, &QTimer::timeout, this, &SettingsStore::saveConfig);
}

QString SettingsStore::configPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json";
}

void SettingsStore::setRefreshFrequency(int minutes) {
    if (m_refreshFrequency != minutes) {
        m_refreshFrequency = minutes;
        m_settings.setValue("refreshFrequency", minutes);
        emit refreshFrequencyChanged();
    }
}

void SettingsStore::setLaunchAtLogin(bool enable) {
    if (m_launchAtLogin != enable) {
        QString error;
        const bool applied = LaunchAtLoginController::setEnabled(enable, &error);
        const bool storedValue = applied ? enable : false;
        if (!applied && !error.isEmpty()) {
            qWarning() << "Failed to update launch-at-login:" << error;
        }
        m_launchAtLogin = storedValue;
        m_settings.setValue("launchAtLogin", storedValue);
        emit launchAtLoginChanged();
    }
}

void SettingsStore::setCheckForUpdates(bool enable) {
    if (m_checkForUpdates != enable) {
        m_checkForUpdates = enable;
        m_settings.setValue("checkForUpdates", enable);
        emit checkForUpdatesChanged();
    }
}

void SettingsStore::setDebugMenuEnabled(bool enable) {
    if (m_debugMenuEnabled != enable) {
        m_debugMenuEnabled = enable;
        m_settings.setValue("debugMenuEnabled", enable);
        emit debugMenuEnabledChanged();
    }
}

void SettingsStore::setCodexVerboseLogging(bool enable) {
    if (m_codexVerboseLogging != enable) {
        m_codexVerboseLogging = enable;
        m_settings.setValue("codexVerboseLogging", enable);
        emit codexVerboseLoggingChanged();
    }
}

void SettingsStore::setCodexWebDebugDumpHTML(bool enable) {
    if (m_codexWebDebugDumpHTML != enable) {
        m_codexWebDebugDumpHTML = enable;
        m_settings.setValue("codexWebDebugDumpHTML", enable);
        emit codexWebDebugDumpHTMLChanged();
    }
}

void SettingsStore::setMergeIcons(bool enable) {
    if (m_mergeIcons != enable) {
        m_mergeIcons = enable;
        m_settings.setValue("mergeIcons", enable);
        emit mergeIconsChanged();
    }
}

void SettingsStore::setStatusChecksEnabled(bool enable) {
    if (m_statusChecksEnabled != enable) {
        m_statusChecksEnabled = enable;
        m_settings.setValue("statusChecksEnabled", enable);
        emit statusChecksEnabledChanged();
    }
}

void SettingsStore::setUsageBarsShowUsed(bool enable) {
    if (m_usageBarsShowUsed != enable) {
        m_usageBarsShowUsed = enable;
        m_settings.setValue("usageBarsShowUsed", enable);
        emit usageBarsShowUsedChanged();
    }
}

void SettingsStore::setResetTimesShowAbsolute(bool enable) {
    if (m_resetTimesShowAbsolute != enable) {
        m_resetTimesShowAbsolute = enable;
        m_settings.setValue("resetTimesShowAbsolute", enable);
        emit resetTimesShowAbsoluteChanged();
    }
}

void SettingsStore::setShowOptionalCreditsAndExtraUsage(bool enable) {
    if (m_showOptionalCreditsAndExtraUsage != enable) {
        m_showOptionalCreditsAndExtraUsage = enable;
        m_settings.setValue("showOptionalCreditsAndExtraUsage", enable);
        emit showOptionalCreditsAndExtraUsageChanged();
    }
}

void SettingsStore::setSessionQuotaNotificationsEnabled(bool enable) {
    if (m_sessionQuotaNotificationsEnabled != enable) {
        m_sessionQuotaNotificationsEnabled = enable;
        m_settings.setValue("sessionQuotaNotificationsEnabled", enable);
        emit sessionQuotaNotificationsEnabledChanged();
    }
}

void SettingsStore::setClaudePeakHoursEnabled(bool enable) {
    if (m_claudePeakHoursEnabled != enable) {
        m_claudePeakHoursEnabled = enable;
        m_settings.setValue("claudePeakHoursEnabled", enable);
        emit claudePeakHoursEnabledChanged();
    }
}

void SettingsStore::setBrowserSessionBridgeEnabled(bool enable) {
    if (m_browserSessionBridgeEnabled != enable) {
        m_browserSessionBridgeEnabled = enable;
        m_settings.setValue("browserSessionBridgeEnabled", enable);
        emit browserSessionBridgeEnabledChanged();
    }
}

void SettingsStore::setGlassEffectEnabled(bool enable) {
    if (m_glassEffectEnabled != enable) {
        m_glassEffectEnabled = enable;
        m_settings.setValue("glassEffectEnabled", enable);
        emit glassEffectEnabledChanged();
    }
}

void SettingsStore::setGlassEffectOpacity(int opacity) {
    const int bounded = qBound(5, opacity, 95);
    if (m_glassEffectOpacity != bounded) {
        m_glassEffectOpacity = bounded;
        m_settings.setValue("glassEffectOpacity", bounded);
        emit glassEffectOpacityChanged();
    }
}

void SettingsStore::setReduceMotion(bool enable) {
    if (m_reduceMotion != enable) {
        m_reduceMotion = enable;
        m_settings.setValue("reduceMotion", enable);
        emit reduceMotionChanged();
    }
}

void SettingsStore::setVisualEffectsQuality(const QString& quality) {
    QString boundedQuality = quality;
    if (boundedQuality != QStringLiteral("high")
        && boundedQuality != QStringLiteral("balanced")
        && boundedQuality != QStringLiteral("low")) {
        boundedQuality = QStringLiteral("balanced");
    }

    if (m_visualEffectsQuality != boundedQuality) {
        m_visualEffectsQuality = boundedQuality;
        m_settings.setValue("visualEffectsQuality", boundedQuality);
        emit visualEffectsQualityChanged();
    }
}

void SettingsStore::setLanguage(const QString& lang) {
    if (m_language != lang) {
        m_language = lang;
        m_settings.setValue("language", lang);
        emit languageChanged();
    }
}

void SettingsStore::setTheme(int theme) {
    if (m_theme != theme) {
        m_theme = theme;
        m_settings.setValue("theme", theme);
        emit themeChanged();
    }
}

void SettingsStore::setTrayDisplayMode(int mode) {
    if (m_trayDisplayMode != mode) {
        m_trayDisplayMode = mode;
        m_settings.setValue("trayDisplayMode", mode);
        emit trayDisplayModeChanged();
    }
}

void SettingsStore::setWarningThreshold(int val) {
    if (m_warningThreshold != val) {
        m_warningThreshold = val;
        m_settings.setValue("warningThreshold", val);
        emit warningThresholdChanged();
    }
}

void SettingsStore::setCriticalThreshold(int val) {
    if (m_criticalThreshold != val) {
        m_criticalThreshold = val;
        m_settings.setValue("criticalThreshold", val);
        emit criticalThresholdChanged();
    }
}

bool SettingsStore::isProviderEnabled(const QString& id) const {
    return m_settings.value("providers/" + id + "/enabled", false).toBool();
}

void SettingsStore::setProviderEnabled(const QString& id, bool enabled) {
    m_settings.setValue("providers/" + id + "/enabled", enabled);
}

QStringList SettingsStore::providerOrder() const {
    return m_providerOrder;
}

void SettingsStore::setProviderOrder(const QStringList& order) {
    m_providerOrder = order;
    scheduleSave();
    emit providerOrderChanged();
}

QVariant SettingsStore::providerSetting(const QString& providerID,
                                         const QString& key,
                                         const QVariant& defaultValue) const
{
    auto it = m_providerSettings.constFind(providerID);
    if (it != m_providerSettings.constEnd()) {
        auto kit = it->constFind(key);
        if (kit != it->constEnd()) {
            return kit.value();
        }
    }
    return defaultValue;
}

void SettingsStore::setProviderSetting(const QString& providerID,
                                         const QString& key,
                                         const QVariant& value)
{
    m_providerSettings[providerID][key] = value;
    scheduleSave();
    emit providerSettingChanged(providerID, key);
}

void SettingsStore::loadConfig() {
    QFile file(configPath());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        m_config = doc.object();
        file.close();

        QJsonObject providers = m_config.value("providers").toObject();
        for (auto it = providers.begin(); it != providers.end(); ++it) {
            QJsonObject settings = it.value().toObject();
            QHash<QString, QVariant> map;
            for (auto sit = settings.begin(); sit != settings.end(); ++sit) {
                map[sit.key()] = sit.value().toVariant();
            }
            m_providerSettings[it.key()] = map;
        }

        QJsonArray orderArr = m_config.value("providerOrder").toArray();
        m_providerOrder.clear();
        for (const auto& v : orderArr) {
            m_providerOrder.append(v.toString());
        }
    }
}

void SettingsStore::scheduleSave() {
    if (m_saveDelayTimer.isActive()) {
        m_saveDelayTimer.stop();
    }
    m_saveDelayTimer.start();
}

void SettingsStore::saveConfig() {
    QJsonObject obj;
    QJsonObject providers;
    for (auto it = m_providerSettings.constBegin(); it != m_providerSettings.constEnd(); ++it) {
        QJsonObject settings;
        for (auto sit = it.value().constBegin(); sit != it.value().constEnd(); ++sit) {
            settings[sit.key()] = QJsonValue::fromVariant(sit.value());
        }
        providers[it.key()] = settings;
    }
    obj["providers"] = providers;

    QJsonArray orderArr;
    for (const auto& id : m_providerOrder) {
        orderArr.append(id);
    }
    obj["providerOrder"] = orderArr;

    QDir().mkpath(QFileInfo(configPath()).absolutePath());
    QFile file(configPath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        file.close();
    }
}

void SettingsStore::resetToDefaults() {
    setRefreshFrequency(5);
    setLaunchAtLogin(false);
    setCheckForUpdates(true);
    setDebugMenuEnabled(false);
    setMergeIcons(true);
    setStatusChecksEnabled(true);
    setUsageBarsShowUsed(false);
    setResetTimesShowAbsolute(false);
    setShowOptionalCreditsAndExtraUsage(true);
    setSessionQuotaNotificationsEnabled(true);
    setClaudePeakHoursEnabled(true);
    setBrowserSessionBridgeEnabled(true);
    setGlassEffectEnabled(true);
    setGlassEffectOpacity(40);
    setReduceMotion(false);
    setVisualEffectsQuality("balanced");
    setLanguage("en");
    setTheme(0);
    setTrayDisplayMode(0);
    setWarningThreshold(20);
    setCriticalThreshold(10);
    m_providerSettings.clear();
    m_providerOrder.clear();
    saveConfig();
    emit providerOrderChanged();
}

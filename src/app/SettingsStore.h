#pragma once

#include <QObject>
#include <QSettings>
#include <QJsonObject>
#include <QStringList>
#include <QTimer>

class SettingsStore : public QObject {
    Q_OBJECT

    Q_PROPERTY(int refreshFrequency READ refreshFrequency WRITE setRefreshFrequency NOTIFY refreshFrequencyChanged)
    Q_PROPERTY(bool launchAtLogin READ launchAtLogin WRITE setLaunchAtLogin NOTIFY launchAtLoginChanged)
    Q_PROPERTY(bool checkForUpdates READ checkForUpdates WRITE setCheckForUpdates NOTIFY checkForUpdatesChanged)
    Q_PROPERTY(bool debugMenuEnabled READ debugMenuEnabled WRITE setDebugMenuEnabled NOTIFY debugMenuEnabledChanged)
    Q_PROPERTY(bool codexVerboseLogging READ codexVerboseLogging WRITE setCodexVerboseLogging NOTIFY codexVerboseLoggingChanged)
    Q_PROPERTY(bool codexWebDebugDumpHTML READ codexWebDebugDumpHTML WRITE setCodexWebDebugDumpHTML NOTIFY codexWebDebugDumpHTMLChanged)
    Q_PROPERTY(bool mergeIcons READ mergeIcons WRITE setMergeIcons NOTIFY mergeIconsChanged)
    Q_PROPERTY(bool statusChecksEnabled READ statusChecksEnabled WRITE setStatusChecksEnabled NOTIFY statusChecksEnabledChanged)
    Q_PROPERTY(bool usageBarsShowUsed READ usageBarsShowUsed WRITE setUsageBarsShowUsed NOTIFY usageBarsShowUsedChanged)
    Q_PROPERTY(bool resetTimesShowAbsolute READ resetTimesShowAbsolute WRITE setResetTimesShowAbsolute NOTIFY resetTimesShowAbsoluteChanged)
    Q_PROPERTY(bool showOptionalCreditsAndExtraUsage READ showOptionalCreditsAndExtraUsage WRITE setShowOptionalCreditsAndExtraUsage NOTIFY showOptionalCreditsAndExtraUsageChanged)
    Q_PROPERTY(bool sessionQuotaNotificationsEnabled READ sessionQuotaNotificationsEnabled WRITE setSessionQuotaNotificationsEnabled NOTIFY sessionQuotaNotificationsEnabledChanged)
    Q_PROPERTY(bool claudePeakHoursEnabled READ claudePeakHoursEnabled WRITE setClaudePeakHoursEnabled NOTIFY claudePeakHoursEnabledChanged)
    Q_PROPERTY(bool browserSessionBridgeEnabled READ browserSessionBridgeEnabled WRITE setBrowserSessionBridgeEnabled NOTIFY browserSessionBridgeEnabledChanged)
    Q_PROPERTY(bool glassEffectEnabled READ glassEffectEnabled WRITE setGlassEffectEnabled NOTIFY glassEffectEnabledChanged)
    Q_PROPERTY(int glassEffectOpacity READ glassEffectOpacity WRITE setGlassEffectOpacity NOTIFY glassEffectOpacityChanged)
    Q_PROPERTY(bool reduceMotion READ reduceMotion WRITE setReduceMotion NOTIFY reduceMotionChanged)
    Q_PROPERTY(QString visualEffectsQuality READ visualEffectsQuality WRITE setVisualEffectsQuality NOTIFY visualEffectsQualityChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(int trayDisplayMode READ trayDisplayMode WRITE setTrayDisplayMode NOTIFY trayDisplayModeChanged)
    Q_PROPERTY(int warningThreshold READ warningThreshold WRITE setWarningThreshold NOTIFY warningThresholdChanged)
    Q_PROPERTY(int criticalThreshold READ criticalThreshold WRITE setCriticalThreshold NOTIFY criticalThresholdChanged)

public:
    explicit SettingsStore(QObject* parent = nullptr);

    int refreshFrequency() const { return m_refreshFrequency; }
    void setRefreshFrequency(int minutes);

    bool launchAtLogin() const { return m_launchAtLogin; }
    void setLaunchAtLogin(bool enable);

    bool checkForUpdates() const { return m_checkForUpdates; }
    void setCheckForUpdates(bool enable);

    bool debugMenuEnabled() const { return m_debugMenuEnabled; }
    void setDebugMenuEnabled(bool enable);

    bool codexVerboseLogging() const { return m_codexVerboseLogging; }
    void setCodexVerboseLogging(bool enable);

    bool codexWebDebugDumpHTML() const { return m_codexWebDebugDumpHTML; }
    void setCodexWebDebugDumpHTML(bool enable);

    bool mergeIcons() const { return m_mergeIcons; }
    void setMergeIcons(bool enable);

    bool statusChecksEnabled() const { return m_statusChecksEnabled; }
    void setStatusChecksEnabled(bool enable);

    bool usageBarsShowUsed() const { return m_usageBarsShowUsed; }
    void setUsageBarsShowUsed(bool enable);

    bool resetTimesShowAbsolute() const { return m_resetTimesShowAbsolute; }
    void setResetTimesShowAbsolute(bool enable);

    bool showOptionalCreditsAndExtraUsage() const { return m_showOptionalCreditsAndExtraUsage; }
    void setShowOptionalCreditsAndExtraUsage(bool enable);

    bool sessionQuotaNotificationsEnabled() const { return m_sessionQuotaNotificationsEnabled; }
    void setSessionQuotaNotificationsEnabled(bool enable);

    bool claudePeakHoursEnabled() const { return m_claudePeakHoursEnabled; }
    void setClaudePeakHoursEnabled(bool enable);

    bool browserSessionBridgeEnabled() const { return m_browserSessionBridgeEnabled; }
    void setBrowserSessionBridgeEnabled(bool enable);

    bool glassEffectEnabled() const { return m_glassEffectEnabled; }
    void setGlassEffectEnabled(bool enable);

    int glassEffectOpacity() const { return m_glassEffectOpacity; }
    void setGlassEffectOpacity(int opacity);

    bool reduceMotion() const { return m_reduceMotion; }
    void setReduceMotion(bool enable);

    QString visualEffectsQuality() const { return m_visualEffectsQuality; }
    void setVisualEffectsQuality(const QString& quality);

    QString language() const { return m_language; }
    void setLanguage(const QString& lang);

    int theme() const { return m_theme; }
    void setTheme(int theme);

    int trayDisplayMode() const { return m_trayDisplayMode; }
    void setTrayDisplayMode(int mode);

    int warningThreshold() const { return m_warningThreshold; }
    void setWarningThreshold(int val);

    int criticalThreshold() const { return m_criticalThreshold; }
    void setCriticalThreshold(int val);

    Q_INVOKABLE bool isProviderEnabled(const QString& id) const;
    Q_INVOKABLE void setProviderEnabled(const QString& id, bool enabled);
    Q_INVOKABLE QStringList providerOrder() const;
    Q_INVOKABLE void setProviderOrder(const QStringList& order);

    Q_INVOKABLE QVariant providerSetting(const QString& providerID,
                                          const QString& key,
                                          const QVariant& defaultValue = {}) const;
    Q_INVOKABLE void setProviderSetting(const QString& providerID,
                                         const QString& key,
                                         const QVariant& value);

    void loadConfig();
    Q_INVOKABLE void saveConfig();
    Q_INVOKABLE void resetToDefaults();
    QString configPath() const;

private:
    void scheduleSave();

signals:
    void refreshFrequencyChanged();
    void launchAtLoginChanged();
    void checkForUpdatesChanged();
    void debugMenuEnabledChanged();
    void codexVerboseLoggingChanged();
    void codexWebDebugDumpHTMLChanged();
    void mergeIconsChanged();
    void statusChecksEnabledChanged();
    void usageBarsShowUsedChanged();
    void resetTimesShowAbsoluteChanged();
    void showOptionalCreditsAndExtraUsageChanged();
    void sessionQuotaNotificationsEnabledChanged();
    void claudePeakHoursEnabledChanged();
    void browserSessionBridgeEnabledChanged();
    void glassEffectEnabledChanged();
    void glassEffectOpacityChanged();
    void reduceMotionChanged();
    void visualEffectsQualityChanged();
    void languageChanged();
    void themeChanged();
    void trayDisplayModeChanged();
    void warningThresholdChanged();
    void criticalThresholdChanged();
    void providerOrderChanged();
    void providerSettingChanged(const QString& providerID, const QString& key);

private:
    QSettings m_settings;
    QJsonObject m_config;
    QTimer m_saveDelayTimer;

    int m_refreshFrequency = 5;
    bool m_launchAtLogin = false;
    bool m_checkForUpdates = true;
    bool m_debugMenuEnabled = false;
    bool m_codexVerboseLogging = false;
    bool m_codexWebDebugDumpHTML = false;
    bool m_mergeIcons = true;
    bool m_statusChecksEnabled = true;
    bool m_usageBarsShowUsed = false;
    bool m_resetTimesShowAbsolute = false;
    bool m_showOptionalCreditsAndExtraUsage = true;
    bool m_sessionQuotaNotificationsEnabled = true;
    bool m_claudePeakHoursEnabled = true;
    bool m_browserSessionBridgeEnabled = true;
    bool m_glassEffectEnabled = true;
    int m_glassEffectOpacity = 40;
    bool m_reduceMotion = false;
    QString m_visualEffectsQuality = QStringLiteral("balanced");
    QString m_language;
    int m_theme = 0;
    int m_trayDisplayMode = 0;
    int m_warningThreshold = 20;
    int m_criticalThreshold = 10;
    QStringList m_providerOrder;
    QHash<QString, QHash<QString, QVariant>> m_providerSettings;
};

#pragma once

#include <QObject>
#include <QString>

class PerformanceState : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool trayVisible READ trayVisible NOTIFY trayVisibleChanged)
    Q_PROPERTY(bool settingsVisible READ settingsVisible NOTIFY settingsVisibleChanged)
    Q_PROPERTY(bool usageVisible READ usageVisible NOTIFY usageVisibleChanged)
    Q_PROPERTY(bool anyUiVisible READ anyUiVisible NOTIFY anyUiVisibleChanged)
    Q_PROPERTY(bool decorativeEffectsActive READ decorativeEffectsActive NOTIFY decorativeEffectsActiveChanged)
    Q_PROPERTY(bool backgroundIdle READ backgroundIdle NOTIFY backgroundIdleChanged)

public:
    explicit PerformanceState(QObject* parent = nullptr);

    bool trayVisible() const { return m_trayVisible; }
    bool settingsVisible() const { return m_settingsVisible; }
    bool usageVisible() const { return m_usageVisible; }
    bool anyUiVisible() const;
    bool decorativeEffectsActive() const;
    bool backgroundIdle() const;
    bool reduceMotion() const { return m_reduceMotion; }
    QString visualEffectsQuality() const { return m_visualEffectsQuality; }

    void setTrayVisible(bool visible);
    void setSettingsVisible(bool visible);
    void setUsageVisible(bool visible);
    void setReduceMotion(bool reduceMotion);
    void setVisualEffectsQuality(const QString& quality);

signals:
    void trayVisibleChanged();
    void settingsVisibleChanged();
    void usageVisibleChanged();
    void anyUiVisibleChanged();
    void decorativeEffectsActiveChanged();
    void backgroundIdleChanged();
    void reduceMotionChanged();
    void visualEffectsQualityChanged();

private:
    struct DerivedState {
        bool anyUiVisible = false;
        bool decorativeEffectsActive = false;
        bool backgroundIdle = true;
    };

    DerivedState derivedState() const;
    void emitDerivedChanges(const DerivedState& before);
    static QString normalizeVisualEffectsQuality(const QString& quality);

    bool m_trayVisible = false;
    bool m_settingsVisible = false;
    bool m_usageVisible = false;
    bool m_reduceMotion = false;
    QString m_visualEffectsQuality = QStringLiteral("balanced");
};

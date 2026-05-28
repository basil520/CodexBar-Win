#include "PerformanceState.h"

PerformanceState::PerformanceState(QObject* parent)
    : QObject(parent)
{}

bool PerformanceState::anyUiVisible() const
{
    return m_trayVisible || m_settingsVisible || m_usageVisible;
}

bool PerformanceState::decorativeEffectsActive() const
{
    return anyUiVisible()
        && !m_reduceMotion
        && m_visualEffectsQuality != QLatin1String("low");
}

bool PerformanceState::backgroundIdle() const
{
    return !anyUiVisible();
}

void PerformanceState::setTrayVisible(bool visible)
{
    if (m_trayVisible == visible) {
        return;
    }
    const DerivedState before = derivedState();
    m_trayVisible = visible;
    emit trayVisibleChanged();
    emitDerivedChanges(before);
}

void PerformanceState::setSettingsVisible(bool visible)
{
    if (m_settingsVisible == visible) {
        return;
    }
    const DerivedState before = derivedState();
    m_settingsVisible = visible;
    emit settingsVisibleChanged();
    emitDerivedChanges(before);
}

void PerformanceState::setUsageVisible(bool visible)
{
    if (m_usageVisible == visible) {
        return;
    }
    const DerivedState before = derivedState();
    m_usageVisible = visible;
    emit usageVisibleChanged();
    emitDerivedChanges(before);
}

void PerformanceState::setReduceMotion(bool reduceMotion)
{
    if (m_reduceMotion == reduceMotion) {
        return;
    }
    const DerivedState before = derivedState();
    m_reduceMotion = reduceMotion;
    emit reduceMotionChanged();
    emitDerivedChanges(before);
}

void PerformanceState::setVisualEffectsQuality(const QString& quality)
{
    const QString normalized = normalizeVisualEffectsQuality(quality);
    if (m_visualEffectsQuality == normalized) {
        return;
    }
    const DerivedState before = derivedState();
    m_visualEffectsQuality = normalized;
    emit visualEffectsQualityChanged();
    emitDerivedChanges(before);
}

PerformanceState::DerivedState PerformanceState::derivedState() const
{
    return {anyUiVisible(), decorativeEffectsActive(), backgroundIdle()};
}

void PerformanceState::emitDerivedChanges(const DerivedState& before)
{
    const DerivedState after = derivedState();
    if (before.anyUiVisible != after.anyUiVisible) {
        emit anyUiVisibleChanged();
    }
    if (before.decorativeEffectsActive != after.decorativeEffectsActive) {
        emit decorativeEffectsActiveChanged();
    }
    if (before.backgroundIdle != after.backgroundIdle) {
        emit backgroundIdleChanged();
    }
}

QString PerformanceState::normalizeVisualEffectsQuality(const QString& quality)
{
    if (quality == QLatin1String("high")
        || quality == QLatin1String("balanced")
        || quality == QLatin1String("low")) {
        return quality;
    }
    return QStringLiteral("balanced");
}

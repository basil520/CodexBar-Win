#include "app/PerformanceState.h"

#include <QSignalSpy>
#include <QtTest/QtTest>

class tst_PerformanceState : public QObject {
    Q_OBJECT

private slots:
    void derivesVisibilityAndDecorativeState();
    void ignoresInvalidVisualEffectsQuality();
};

void tst_PerformanceState::derivesVisibilityAndDecorativeState()
{
    PerformanceState state;
    QSignalSpy anyUiSpy(&state, &PerformanceState::anyUiVisibleChanged);
    QSignalSpy decorativeSpy(&state, &PerformanceState::decorativeEffectsActiveChanged);
    QSignalSpy idleSpy(&state, &PerformanceState::backgroundIdleChanged);

    QVERIFY(!state.anyUiVisible());
    QVERIFY(!state.decorativeEffectsActive());
    QVERIFY(state.backgroundIdle());

    state.setTrayVisible(true);
    QVERIFY(state.trayVisible());
    QVERIFY(state.anyUiVisible());
    QVERIFY(state.decorativeEffectsActive());
    QVERIFY(!state.backgroundIdle());
    QCOMPARE(anyUiSpy.count(), 1);
    QCOMPARE(decorativeSpy.count(), 1);
    QCOMPARE(idleSpy.count(), 1);

    state.setVisualEffectsQuality(QStringLiteral("low"));
    QVERIFY(!state.decorativeEffectsActive());
    QCOMPARE(decorativeSpy.count(), 2);

    state.setUsageVisible(true);
    QVERIFY(state.anyUiVisible());
    QVERIFY(!state.decorativeEffectsActive());
    QCOMPARE(anyUiSpy.count(), 1);

    state.setVisualEffectsQuality(QStringLiteral("balanced"));
    QVERIFY(state.decorativeEffectsActive());
    QCOMPARE(decorativeSpy.count(), 3);

    state.setReduceMotion(true);
    QVERIFY(!state.decorativeEffectsActive());
    QCOMPARE(decorativeSpy.count(), 4);

    state.setTrayVisible(false);
    state.setUsageVisible(false);
    QVERIFY(!state.anyUiVisible());
    QVERIFY(state.backgroundIdle());
    QVERIFY(!state.decorativeEffectsActive());
    QCOMPARE(anyUiSpy.count(), 2);
    QCOMPARE(idleSpy.count(), 2);
}

void tst_PerformanceState::ignoresInvalidVisualEffectsQuality()
{
    PerformanceState state;
    state.setSettingsVisible(true);
    QVERIFY(state.decorativeEffectsActive());

    state.setVisualEffectsQuality(QStringLiteral("potato"));
    QCOMPARE(state.visualEffectsQuality(), QStringLiteral("balanced"));
    QVERIFY(state.decorativeEffectsActive());
}

QTEST_MAIN(tst_PerformanceState)
#include "tst_PerformanceState.moc"

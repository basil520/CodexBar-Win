#include "app/UiFreezeWatchdog.h"

#include <QtTest/QtTest>

class UiFreezeWatchdogTest : public QObject {
    Q_OBJECT

private slots:
    void phaseScopeRestoresNestedPhases();
    void releaseBuildDoesNotStartByDefault();
    void debugSettingCanEnableWatchdog();
    void environmentCanEnableWatchdog();
};

void UiFreezeWatchdogTest::phaseScopeRestoresNestedPhases()
{
    UiFreezeWatchdog::setCurrentPhase(QStringLiteral("idle"));
    QCOMPARE(UiFreezeWatchdog::currentPhase(), QStringLiteral("idle"));

    {
        UiFreezeWatchdog::PhaseScope outer(QStringLiteral("tray.load"));
        QCOMPARE(UiFreezeWatchdog::currentPhase(), QStringLiteral("tray.load"));

        {
            UiFreezeWatchdog::PhaseScope inner(QStringLiteral("usage.open"));
            QCOMPARE(UiFreezeWatchdog::currentPhase(), QStringLiteral("usage.open"));
        }

        QCOMPARE(UiFreezeWatchdog::currentPhase(), QStringLiteral("tray.load"));
    }

    QCOMPARE(UiFreezeWatchdog::currentPhase(), QStringLiteral("idle"));
}

void UiFreezeWatchdogTest::releaseBuildDoesNotStartByDefault()
{
#if defined(NDEBUG)
    QVERIFY(!UiFreezeWatchdog::shouldStartByDefault());
#else
    QVERIFY(UiFreezeWatchdog::shouldStartByDefault());
#endif
}

void UiFreezeWatchdogTest::debugSettingCanEnableWatchdog()
{
    QVERIFY(UiFreezeWatchdog::shouldStartForSettings(true));
}

void UiFreezeWatchdogTest::environmentCanEnableWatchdog()
{
    qputenv("CODEXBAR_UI_FREEZE_WATCHDOG", "1");
    QVERIFY(UiFreezeWatchdog::shouldStartForSettings(false));
    qunsetenv("CODEXBAR_UI_FREEZE_WATCHDOG");
}

QTEST_MAIN(UiFreezeWatchdogTest)

#include "tst_UiFreezeWatchdog.moc"

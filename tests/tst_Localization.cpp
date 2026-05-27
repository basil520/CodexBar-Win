#include <QtTest>

#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>

#include "app/LanguageManager.h"
#include "app/Localization.h"
#include "models/UsagePace.h"
#include "util/UsagePaceText.h"

class LocalizationTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qputenv("WINCODEXBAR_TRANSLATION_DIR", TEST_TRANSLATION_DIR);
        const QString zhPath = QStringLiteral(TEST_TRANSLATION_DIR) + "/CodexBarX_zh_CN.qm";
        if (!QFileInfo::exists(zhPath)) {
            QSKIP("Compiled zh_CN translation is not available.");
        }
    }

    void testLanguageRevisionChanges() {
        auto& manager = LanguageManager::instance();
        manager.setLanguage("en");

        const int before = manager.translationRevision();
        QSignalSpy revisionSpy(&manager, &LanguageManager::translationRevisionChanged);

        manager.setLanguage("zh_CN");

        QVERIFY(manager.translationRevision() > before);
        QCOMPARE(revisionSpy.count(), 1);
    }

    void testProviderTextTranslatesAndReturnsToEnglish() {
        auto& manager = LanguageManager::instance();
        manager.setLanguage("zh_CN");

        QCOMPARE(Localization::providerLabel("Session"), QString::fromUtf8("会话"));
        QCOMPARE(Localization::providerSettingLabel("Data source"), QString::fromUtf8("数据来源"));
        QCOMPARE(Localization::providerError("no available fetch strategy"),
                 QString::fromUtf8("没有可用的获取策略"));

        manager.setLanguage("en");

        QCOMPARE(Localization::providerLabel("Session"), QString("Session"));
        QCOMPARE(Localization::providerSettingLabel("Data source"), QString("Data source"));
        QCOMPARE(Localization::providerError("no available fetch strategy"),
                 QString("no available fetch strategy"));
    }

    void testUsagePaceTextTranslates() {
        auto& manager = LanguageManager::instance();
        manager.setLanguage("zh_CN");

        UsagePace pace;
        pace.stage = UsagePace::Stage::onTrack;
        pace.willLastToReset = true;

        auto detail = UsagePaceText::weeklyDetail(pace);
        QCOMPARE(detail.leftLabel, QString::fromUtf8("节奏正常"));
        QCOMPARE(detail.rightLabel, QString::fromUtf8("可持续到重置"));

        manager.setLanguage("en");
    }

    void testUsagePaceEnglishSourceUsesCleanSymbols() {
        auto& manager = LanguageManager::instance();
        manager.setLanguage("en");

        UsagePace pace;
        pace.stage = UsagePace::Stage::onTrack;
        pace.willLastToReset = true;
        pace.runOutProbability = 0.12;

        QCOMPARE(UsagePaceText::weeklySummary(pace),
                 QString::fromUtf8("Pace: On pace · Lasts until reset · ≈ 10% run-out risk"));
    }

    void testTranslationSourcesAreComplete() {
        auditTranslationSource("translations/CodexBarX_en.ts", true);
        auditTranslationSource("translations/CodexBarX_zh_CN.ts", false);
    }

private:
    void auditTranslationSource(const QString& relativePath, bool englishMustMatchSource) {
        QFile file(QStringLiteral(PROJECT_SOURCE_DIR "/") + relativePath);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(file.errorString()));

        QXmlStreamReader xml(&file);
        QString source;
        QString translation;
        QString translationType;
        int activeMessages = 0;

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == QLatin1String("message")) {
                source.clear();
                translation.clear();
                translationType.clear();
            } else if (xml.isStartElement() && xml.name() == QLatin1String("source")) {
                source = xml.readElementText();
            } else if (xml.isStartElement() && xml.name() == QLatin1String("translation")) {
                translationType = xml.attributes().value(QStringLiteral("type")).toString();
                translation = xml.readElementText();
            } else if (xml.isEndElement() && xml.name() == QLatin1String("message")) {
                QVERIFY2(translationType != QLatin1String("vanished"),
                         qPrintable(relativePath + QStringLiteral(" contains obsolete vanished source: ") + source));
                QVERIFY2(translationType != QLatin1String("unfinished"),
                         qPrintable(relativePath + QStringLiteral(" contains unfinished source: ") + source));
                ++activeMessages;
                QVERIFY2(!translation.trimmed().isEmpty(),
                         qPrintable(relativePath + QStringLiteral(" has an empty translation for: ") + source));
                QVERIFY2(!translation.contains(QChar(0xfffd)),
                         qPrintable(relativePath + QStringLiteral(" contains replacement characters for: ") + source));
                if (englishMustMatchSource) {
                    QCOMPARE(translation, source);
                }
            }
        }

        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY2(activeMessages > 0, qPrintable(relativePath + QStringLiteral(" must contain active messages.")));
    }
};

QTEST_MAIN(LocalizationTest)
#include "tst_Localization.moc"

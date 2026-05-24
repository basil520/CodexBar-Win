#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include <QStandardPaths>

#include "../src/browserbridge/BrowserSessionBridgeInstallService.h"

class tst_BrowserSessionBridgeInstallService : public QObject {
    Q_OBJECT

    static bool containsColorNear(const QImage& image, const QColor& expected, int tolerance = 24) {
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor actual = QColor::fromRgba(image.pixel(x, y));
                if (actual.alpha() < 180) {
                    continue;
                }
                if (qAbs(actual.red() - expected.red()) <= tolerance
                    && qAbs(actual.green() - expected.green()) <= tolerance
                    && qAbs(actual.blue() - expected.blue()) <= tolerance) {
                    return true;
                }
            }
        }
        return false;
    }

private slots:
    void initTestCase() {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init() {
        // Clean up exported files from previous tests so each test starts fresh
        BrowserSessionBridgeInstallService service;
        const QString path = service.extensionInstallPath();
        QDir(path).removeRecursively();
    }

    void exportsExtensionFiles() {
        BrowserSessionBridgeInstallService service;
        QVERIFY(service.ensureExtensionExported());

        const QString path = service.extensionInstallPath();
        QVERIFY(QFile::exists(path + "/manifest.json"));
        QVERIFY(QFile::exists(path + "/service_worker.js"));
        QVERIFY(QFile::exists(path + "/popup.html"));
        QVERIFY(QFile::exists(path + "/popup.js"));
        QVERIFY(QFile::exists(path + "/content_scripts/storage_probe.js"));
        QVERIFY(QFile::exists(path + "/runtime.json"));
    }

    void writesRuntimeConfigWithoutSecrets() {
        BrowserSessionBridgeInstallService service;
        QVERIFY(service.ensureExtensionExported());

        const QString path = service.extensionInstallPath() + "/runtime.json";
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QVERIFY(!doc.isNull());
        QJsonObject obj = doc.object();

        QVERIFY(obj.contains("serverPort"));
        QVERIFY(obj.contains("protocolVersion"));
        QVERIFY(obj.contains("allowedOrigins"));
        QCOMPARE(obj.value("serverPort").toInt(), 18765);
        QCOMPARE(obj.value("protocolVersion").toInt(), 1);

        // Must not contain any sensitive data
        QVERIFY(!obj.contains("apiKey"));
        QVERIFY(!obj.contains("cookie"));
        QVERIFY(!obj.contains("secret"));
    }

    void generatesIconsIfMissing() {
        BrowserSessionBridgeInstallService service;
        QVERIFY(service.ensureExtensionExported());

        const QString iconsDir = service.extensionInstallPath() + "/icons";
        const QStringList expectedSizes = {"icon16.png", "icon32.png", "icon48.png", "icon128.png"};
        for (const QString& name : expectedSizes) {
            const QString filePath = iconsDir + "/" + name;
            QVERIFY2(QFile::exists(filePath), qPrintable(name + " not found"));

            QImage img(filePath);
            QVERIFY(!img.isNull());
            QVERIFY(img.format() == QImage::Format_ARGB32 || img.format() == QImage::Format_RGB32);
        }

        // Verify actual pixel dimensions
        QImage img16(iconsDir + "/icon16.png");
        QCOMPARE(img16.width(), 16);
        QCOMPARE(img16.height(), 16);

        QImage img128(iconsDir + "/icon128.png");
        QCOMPARE(img128.width(), 128);
        QCOMPARE(img128.height(), 128);
    }

    void generatedIconsUseQuotaRadarPalette() {
        BrowserSessionBridgeInstallService service;
        QVERIFY(service.ensureExtensionExported());

        const QString iconPath = service.extensionInstallPath() + "/icons/icon128.png";
        QImage icon(iconPath);
        QVERIFY(!icon.isNull());

        QVERIFY2(containsColorNear(icon, QColor("#82f4ff")),
                 "Generated extension icon should include the quota radar cyan arc.");
        QVERIFY2(containsColorNear(icon, QColor("#3ae2ad")),
                 "Generated extension icon should include the quota radar green arc.");
        QVERIFY2(containsColorNear(icon, QColor("#ffc640")),
                 "Generated extension icon should include the quota radar amber arc.");
    }

    void isExtensionExportedReflectsState() {
        BrowserSessionBridgeInstallService service;
        QVERIFY(!service.isExtensionExported());
        QVERIFY(service.ensureExtensionExported());
        QVERIFY(service.isExtensionExported());
    }
};

QTEST_MAIN(tst_BrowserSessionBridgeInstallService)
#include "tst_BrowserSessionBridgeInstallService.moc"

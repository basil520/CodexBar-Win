#include <QtTest/QtTest>

#ifdef Q_OS_MACOS
#include "../src/providers/shared/MacOSChromiumCookieDecryptor.h"

class tst_MacOSChromiumCookieDecryptor : public QObject {
    Q_OBJECT

private slots:
    void mapsChromiumBrowsersToSafeStorageItems();
    void decryptsV10CookieWithInjectedSafeStoragePassword();
    void reportsDeniedKeychainAccess();
};

void tst_MacOSChromiumCookieDecryptor::mapsChromiumBrowsersToSafeStorageItems()
{
    auto chrome = MacOSChromiumCookieDecryptor::safeStorageTarget(CookieImporter::Chrome);
    QCOMPARE(chrome.service, QStringLiteral("Chrome Safe Storage"));
    QCOMPARE(chrome.account, QStringLiteral("Chrome"));

    auto edge = MacOSChromiumCookieDecryptor::safeStorageTarget(CookieImporter::Edge);
    QCOMPARE(edge.service, QStringLiteral("Microsoft Edge Safe Storage"));
    QCOMPARE(edge.account, QStringLiteral("Microsoft Edge"));

    auto brave = MacOSChromiumCookieDecryptor::safeStorageTarget(CookieImporter::Brave);
    QCOMPARE(brave.service, QStringLiteral("Brave Safe Storage"));
    QCOMPARE(brave.account, QStringLiteral("Brave"));
}

void tst_MacOSChromiumCookieDecryptor::decryptsV10CookieWithInjectedSafeStoragePassword()
{
    const QByteArray encrypted = QByteArray::fromBase64("djEwyPxuIMAwIMHJQSN8F/ErDQ==");
    MacOSChromiumCookieDecryptor::SafeStorageTarget capturedTarget;
    int readCount = 0;

    auto result = MacOSChromiumCookieDecryptor::decryptCookieForTesting(
        CookieImporter::Chrome,
        encrypted,
        [&](const MacOSChromiumCookieDecryptor::SafeStorageTarget& target, QString* error)
            -> std::optional<QByteArray> {
            Q_UNUSED(error)
            capturedTarget = target;
            ++readCount;
            return QByteArrayLiteral("test-safe-storage");
        });

    QCOMPARE(readCount, 1);
    QCOMPARE(capturedTarget.service, QStringLiteral("Chrome Safe Storage"));
    QCOMPARE(capturedTarget.account, QStringLiteral("Chrome"));
    QVERIFY2(result.success(), qPrintable(result.errorMessage));
    QCOMPARE(result.plaintext, QByteArrayLiteral("cookie-secret"));
}

void tst_MacOSChromiumCookieDecryptor::reportsDeniedKeychainAccess()
{
    const QByteArray encrypted = QByteArray::fromBase64("djEwyPxuIMAwIMHJQSN8F/ErDQ==");
    auto result = MacOSChromiumCookieDecryptor::decryptCookieForTesting(
        CookieImporter::Chrome,
        encrypted,
        [](const MacOSChromiumCookieDecryptor::SafeStorageTarget&, QString* error)
            -> std::optional<QByteArray> {
            if (error) *error = QStringLiteral("Keychain access was denied");
            return std::nullopt;
        });

    QVERIFY(!result.success());
    QVERIFY(result.plaintext.isEmpty());
    QVERIFY2(result.errorMessage.contains(QStringLiteral("Keychain access was denied")),
             qPrintable(result.errorMessage));
}

QTEST_MAIN(tst_MacOSChromiumCookieDecryptor)
#include "tst_MacOSChromiumCookieDecryptor.moc"
#else
QTEST_MAIN(QObject)
#endif

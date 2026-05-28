#include "MacOSChromiumCookieDecryptor.h"

#import <CommonCrypto/CommonCryptor.h>
#import <CommonCrypto/CommonKeyDerivation.h>
#import <Security/Security.h>

#include <QVector>
#include <QStringList>

namespace {

constexpr int kDerivedKeyBytes = 16;
constexpr int kPbkdf2Iterations = 1003;
constexpr char kSalt[] = "saltysalt";
constexpr char kVersionPrefix[] = "v10";

MacOSChromiumCookieDecryptor::DecryptResult failure(
    MacOSChromiumCookieDecryptor::Error error,
    const QString& message)
{
    MacOSChromiumCookieDecryptor::DecryptResult result;
    result.error = error;
    result.errorMessage = message;
    return result;
}

CFStringRef cfString(const QString& value)
{
    const QByteArray utf8 = value.toUtf8();
    return CFStringCreateWithBytes(kCFAllocatorDefault,
                                   reinterpret_cast<const UInt8*>(utf8.constData()),
                                   static_cast<CFIndex>(utf8.size()),
                                   kCFStringEncodingUTF8,
                                   false);
}

QString osStatusText(OSStatus status)
{
    CFStringRef message = SecCopyErrorMessageString(status, nullptr);
    if (!message) {
        return QStringLiteral("OSStatus %1").arg(static_cast<int>(status));
    }

    char buffer[512] = {};
    const bool ok = CFStringGetCString(message, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(message);
    return ok ? QString::fromUtf8(buffer) : QStringLiteral("OSStatus %1").arg(static_cast<int>(status));
}

std::optional<QByteArray> nativeSafeStorageReader(
    const MacOSChromiumCookieDecryptor::SafeStorageTarget& target,
    QString* errorMessage)
{
    if (target.service.isEmpty() || target.account.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("No macOS Safe Storage keychain target is configured for this browser.");
        }
        return std::nullopt;
    }

    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                            0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);

    CFStringRef serviceRef = cfString(target.service);
    CFStringRef accountRef = cfString(target.account);
    CFDictionarySetValue(query, kSecAttrService, serviceRef);
    CFDictionarySetValue(query, kSecAttrAccount, accountRef);
    CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);

    CFTypeRef resultRef = nullptr;
    const OSStatus status = SecItemCopyMatching(query, &resultRef);

    CFRelease(serviceRef);
    CFRelease(accountRef);
    CFRelease(query);

    if (status != errSecSuccess || !resultRef) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not read %1/%2 from macOS Keychain: %3")
                .arg(target.service, target.account, osStatusText(status));
        }
        return std::nullopt;
    }

    auto dataRef = static_cast<CFDataRef>(resultRef);
    QByteArray data(reinterpret_cast<const char*>(CFDataGetBytePtr(dataRef)),
                    static_cast<int>(CFDataGetLength(dataRef)));
    CFRelease(resultRef);
    return data;
}

std::optional<QByteArray> deriveKey(const QByteArray& password, QString* errorMessage)
{
    if (password.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("macOS Safe Storage password is empty.");
        return std::nullopt;
    }

    QByteArray key(kDerivedKeyBytes, Qt::Uninitialized);
    const int rc = CCKeyDerivationPBKDF(kCCPBKDF2,
                                        password.constData(),
                                        static_cast<size_t>(password.size()),
                                        reinterpret_cast<const uint8_t*>(kSalt),
                                        sizeof(kSalt) - 1,
                                        kCCPRFHmacAlgSHA1,
                                        kPbkdf2Iterations,
                                        reinterpret_cast<uint8_t*>(key.data()),
                                        static_cast<size_t>(key.size()));
    if (rc != kCCSuccess) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not derive macOS Chromium cookie key: %1").arg(rc);
        }
        return std::nullopt;
    }
    return key;
}

MacOSChromiumCookieDecryptor::DecryptResult decryptV10WithPassword(
    const QByteArray& password,
    const QByteArray& encryptedValue)
{
    if (!encryptedValue.startsWith(kVersionPrefix)) {
        return failure(MacOSChromiumCookieDecryptor::Error::UnsupportedCiphertext,
                       QStringLiteral("Unsupported macOS Chromium cookie format. Expected v10."));
    }

    QString keyError;
    const auto key = deriveKey(password, &keyError);
    if (!key.has_value()) {
        return failure(MacOSChromiumCookieDecryptor::Error::MissingSafeStoragePassword, keyError);
    }

    const QByteArray cipherText = encryptedValue.mid(3);
    if (cipherText.isEmpty()) {
        return failure(MacOSChromiumCookieDecryptor::Error::UnsupportedCiphertext,
                       QStringLiteral("macOS Chromium cookie ciphertext is empty."));
    }

    const QByteArray iv(kCCBlockSizeAES128, ' ');
    QByteArray plain(cipherText.size() + kCCBlockSizeAES128, Qt::Uninitialized);
    size_t plainLength = 0;

    const CCCryptorStatus status = CCCrypt(kCCDecrypt,
                                           kCCAlgorithmAES,
                                           kCCOptionPKCS7Padding,
                                           key->constData(),
                                           static_cast<size_t>(key->size()),
                                           iv.constData(),
                                           cipherText.constData(),
                                           static_cast<size_t>(cipherText.size()),
                                           plain.data(),
                                           static_cast<size_t>(plain.size()),
                                           &plainLength);
    if (status != kCCSuccess) {
        return failure(MacOSChromiumCookieDecryptor::Error::DecryptionFailed,
                       QStringLiteral("Could not decrypt macOS Chromium cookie: %1").arg(status));
    }

    plain.resize(static_cast<int>(plainLength));
    MacOSChromiumCookieDecryptor::DecryptResult result;
    result.plaintext = plain;
    return result;
}

QVector<MacOSChromiumCookieDecryptor::SafeStorageTarget> safeStorageTargets(
    CookieImporter::Browser browser)
{
    QVector<MacOSChromiumCookieDecryptor::SafeStorageTarget> targets;
    const auto primary = MacOSChromiumCookieDecryptor::safeStorageTarget(browser);
    if (!primary.service.isEmpty() && !primary.account.isEmpty()) {
        targets.append(primary);
    }

    const MacOSChromiumCookieDecryptor::SafeStorageTarget chromiumFallback = {
        QStringLiteral("Chromium Safe Storage"),
        QStringLiteral("Chromium")
    };
    if (browser != CookieImporter::Firefox
        && (primary.service != chromiumFallback.service || primary.account != chromiumFallback.account)) {
        targets.append(chromiumFallback);
    }

    return targets;
}

} // namespace

MacOSChromiumCookieDecryptor::SafeStorageTarget
MacOSChromiumCookieDecryptor::safeStorageTarget(CookieImporter::Browser browser)
{
    switch (browser) {
    case CookieImporter::Chrome:
        return {QStringLiteral("Chrome Safe Storage"), QStringLiteral("Chrome")};
    case CookieImporter::Edge:
        return {QStringLiteral("Microsoft Edge Safe Storage"), QStringLiteral("Microsoft Edge")};
    case CookieImporter::Brave:
        return {QStringLiteral("Brave Safe Storage"), QStringLiteral("Brave")};
    case CookieImporter::Opera:
        return {QStringLiteral("Opera Safe Storage"), QStringLiteral("Opera")};
    case CookieImporter::Vivaldi:
        return {QStringLiteral("Vivaldi Safe Storage"), QStringLiteral("Vivaldi")};
    case CookieImporter::Firefox:
        return {};
    }
    return {};
}

MacOSChromiumCookieDecryptor::DecryptResult
MacOSChromiumCookieDecryptor::decryptCookie(CookieImporter::Browser browser,
                                            const QByteArray& encryptedValue)
{
    return decryptCookieForTesting(browser, encryptedValue, nativeSafeStorageReader);
}

MacOSChromiumCookieDecryptor::DecryptResult
MacOSChromiumCookieDecryptor::decryptCookieForTesting(CookieImporter::Browser browser,
                                                      const QByteArray& encryptedValue,
                                                      const SafeStorageReader& reader)
{
    const auto targets = safeStorageTargets(browser);
    if (targets.isEmpty()) {
        return failure(Error::UnsupportedBrowser,
                       QStringLiteral("This browser does not use Chromium macOS Safe Storage cookies."));
    }

    QStringList keychainErrors;
    for (const auto& target : targets) {
        QString keychainError;
        const auto password = reader(target, &keychainError);
        if (!password.has_value() || password->isEmpty()) {
            if (!keychainError.isEmpty()) keychainErrors.append(keychainError);
            continue;
        }

        return decryptV10WithPassword(*password, encryptedValue);
    }

    const QString message = keychainErrors.isEmpty()
        ? QStringLiteral("macOS Keychain did not return a Chromium Safe Storage password.")
        : keychainErrors.join(QStringLiteral("; "));
    return failure(Error::MissingSafeStoragePassword, message);
}

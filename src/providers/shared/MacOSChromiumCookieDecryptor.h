#pragma once

#include "CookieImporter.h"

#include <QByteArray>
#include <QString>

#include <functional>
#include <optional>

class MacOSChromiumCookieDecryptor {
public:
    struct SafeStorageTarget {
        QString service;
        QString account;
    };

    enum class Error {
        None,
        UnsupportedBrowser,
        UnsupportedCiphertext,
        MissingSafeStoragePassword,
        DecryptionFailed
    };

    struct DecryptResult {
        QByteArray plaintext;
        Error error = Error::None;
        QString errorMessage;

        bool success() const { return error == Error::None && !plaintext.isEmpty(); }
    };

    using SafeStorageReader =
        std::function<std::optional<QByteArray>(const SafeStorageTarget& target, QString* errorMessage)>;

    static SafeStorageTarget safeStorageTarget(CookieImporter::Browser browser);
    static DecryptResult decryptCookie(CookieImporter::Browser browser, const QByteArray& encryptedValue);
    static DecryptResult decryptCookieForTesting(CookieImporter::Browser browser,
                                                 const QByteArray& encryptedValue,
                                                 const SafeStorageReader& reader);
};

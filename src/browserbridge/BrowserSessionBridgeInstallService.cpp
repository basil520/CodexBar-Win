#include "BrowserSessionBridgeInstallService.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QStandardPaths>

static constexpr int BRIDGE_PROTOCOL_VERSION = 1;
static constexpr int DEFAULT_SERVER_PORT = 18765;

BrowserSessionBridgeInstallService::BrowserSessionBridgeInstallService(QObject* parent)
    : QObject(parent)
{
}

QString BrowserSessionBridgeInstallService::extensionBasePath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return base + QStringLiteral("/browser-session-bridge/extension");
}

QString BrowserSessionBridgeInstallService::extensionInstallPath() const
{
    return QDir::toNativeSeparators(extensionBasePath());
}

bool BrowserSessionBridgeInstallService::isExtensionExported() const
{
    const QString base = extensionBasePath();
    const QStringList requiredFiles = {
        QStringLiteral("manifest.json"),
        QStringLiteral("service_worker.js"),
        QStringLiteral("popup.html"),
        QStringLiteral("popup.js"),
    };
    for (const QString& name : requiredFiles) {
        if (!QFile::exists(base + QLatin1Char('/') + name)) {
            return false;
        }
    }
    const QString iconsDir = base + QStringLiteral("/icons");
    const QStringList requiredIcons = {
        QStringLiteral("icon16.png"),
        QStringLiteral("icon32.png"),
        QStringLiteral("icon48.png"),
        QStringLiteral("icon128.png"),
    };
    for (const QString& name : requiredIcons) {
        if (!QFile::exists(iconsDir + QLatin1Char('/') + name)) {
            return false;
        }
    }
    return true;
}

bool BrowserSessionBridgeInstallService::ensureExtensionExported()
{
    const QString base = extensionBasePath();
    QDir dir(base);
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }

    // Export manifest, service worker, popup
    const QVector<QPair<QString, QString>> files = {
        { QStringLiteral(":/browser-session-bridge/manifest.json"), base + QStringLiteral("/manifest.json") },
        { QStringLiteral(":/browser-session-bridge/service_worker.js"), base + QStringLiteral("/service_worker.js") },
        { QStringLiteral(":/browser-session-bridge/popup.html"), base + QStringLiteral("/popup.html") },
        { QStringLiteral(":/browser-session-bridge/popup.js"), base + QStringLiteral("/popup.js") },
    };
    for (const auto& pair : files) {
        if (!exportFileFromQrc(pair.first, pair.second)) {
            return false;
        }
    }

    // Generate icons if needed
    const QString iconsDir = base + QStringLiteral("/icons");
    if (!ensureIconsGenerated(iconsDir)) {
        return false;
    }

    // Write runtime config
    if (!writeRuntimeConfig(base)) {
        return false;
    }

    return true;
}

bool BrowserSessionBridgeInstallService::exportFileFromQrc(const QString& qrcPath, const QString& destPath) const
{
    QFile src(qrcPath);
    if (!src.open(QIODevice::ReadOnly)) {
        // Fallback for test environments where QRC is not linked:
        // try filesystem paths relative to the application directory.
        const QString fileName = QFileInfo(qrcPath).fileName();
        const QStringList candidates = {
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../../resources/browser-session-bridge/") + fileName,
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../resources/browser-session-bridge/") + fileName,
            QCoreApplication::applicationDirPath() + QStringLiteral("/../resources/browser-session-bridge/") + fileName,
        };
        bool opened = false;
        for (const QString& path : candidates) {
            src.setFileName(path);
            if (src.open(QIODevice::ReadOnly)) {
                opened = true;
                break;
            }
        }
        if (!opened) {
            return false;
        }
    }
    const QByteArray data = src.readAll();
    src.close();

    QFile dest(destPath);
    if (!dest.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    dest.write(data);
    dest.close();
    return true;
}

bool BrowserSessionBridgeInstallService::ensureIconsGenerated(const QString& iconsDir) const
{
    QDir dir(iconsDir);
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }

    const QVector<QPair<QString, int>> icons = {
        { QStringLiteral("icon16.png"), 16 },
        { QStringLiteral("icon32.png"), 32 },
        { QStringLiteral("icon48.png"), 48 },
        { QStringLiteral("icon128.png"), 128 },
    };
    for (const auto& pair : icons) {
        const QString path = iconsDir + QLatin1Char('/') + pair.first;
        // Always regenerate icons to avoid stale or corrupt files from previous runs
        if (QFile::exists(path)) {
            QFile::remove(path);
        }
        if (!generateIconPng(path, pair.second)) {
            return false;
        }
        // Verify the generated PNG is loadable
        QImage verify(path);
        if (verify.isNull() || verify.width() != pair.second || verify.height() != pair.second) {
            return false;
        }
    }
    return true;
}

bool BrowserSessionBridgeInstallService::generateIconPng(const QString& filePath, int size) const
{
    QImage image(size, size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background rounded rect
    const qreal radius = size * 0.2;
    const QRectF rect(0, 0, size, size);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#4A90D9")));
    painter.drawRoundedRect(rect, radius, radius);

    // "CB" text
    QFont font = painter.font();
    font.setPointSizeF(qMax(6.0, size * 0.35));
    font.setBold(true);
    font.setFamily(QStringLiteral("Segoe UI"));
    painter.setFont(font);
    painter.setPen(Qt::white);

    const QString text = QStringLiteral("CB");
    const QFontMetricsF fm(font);
    const QRectF textRect = fm.boundingRect(text);
    const QPointF textPos(
        (size - textRect.width()) / 2.0,
        (size + textRect.height()) / 2.0 - fm.descent()
    );
    painter.drawText(textPos, text);
    painter.end();

    return image.save(filePath, "PNG");
}

bool BrowserSessionBridgeInstallService::writeRuntimeConfig(const QString& destDir) const
{
    QJsonObject obj;
    obj[QStringLiteral("serverPort")] = DEFAULT_SERVER_PORT;
    obj[QStringLiteral("protocolVersion")] = BRIDGE_PROTOCOL_VERSION;
    obj[QStringLiteral("allowedOrigins")] = QJsonArray::fromStringList(
        QStringList { QStringLiteral("codexbarx-browser-session-bridge") }
    );

    const QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Indented);
    const QString path = destDir + QStringLiteral("/runtime.json");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(data);
    file.close();
    return true;
}

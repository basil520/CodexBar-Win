#include "BrowserSessionBridgeInstallService.h"

#include "BrowserSessionBridgeConstants.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include <QStandardPaths>
#include <cmath>

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
        QStringLiteral("protocol.js"),
        QStringLiteral("service_worker.js"),
        QStringLiteral("popup.html"),
        QStringLiteral("popup.js"),
        QStringLiteral("content_scripts/storage_probe.js"),
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

    // Ensure content_scripts subdirectory exists
    const QString contentScriptsDir = base + QStringLiteral("/content_scripts");
    QDir csDir(contentScriptsDir);
    if (!csDir.exists()) {
        if (!csDir.mkpath(QStringLiteral("."))) {
            return false;
        }
    }

    // Export manifest, service worker, popup, content scripts
    const QVector<QPair<QString, QString>> files = {
        { QStringLiteral(":/browser-session-bridge/manifest.json"), base + QStringLiteral("/manifest.json") },
        { QStringLiteral(":/browser-session-bridge/protocol.js"), base + QStringLiteral("/protocol.js") },
        { QStringLiteral(":/browser-session-bridge/service_worker.js"), base + QStringLiteral("/service_worker.js") },
        { QStringLiteral(":/browser-session-bridge/popup.html"), base + QStringLiteral("/popup.html") },
        { QStringLiteral(":/browser-session-bridge/popup.js"), base + QStringLiteral("/popup.js") },
        { QStringLiteral(":/browser-session-bridge/content_scripts/storage_probe.js"), base + QStringLiteral("/content_scripts/storage_probe.js") },
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
        // Strip the ":/browser-session-bridge/" prefix to get the relative path
        const QString relativePath = qrcPath.mid(qrcPath.lastIndexOf(QLatin1String("browser-session-bridge/"))
                                                  + qstrlen("browser-session-bridge/"));
        const QStringList candidates = {
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../../resources/browser-session-bridge/") + relativePath,
            QCoreApplication::applicationDirPath() + QStringLiteral("/../../resources/browser-session-bridge/") + relativePath,
            QCoreApplication::applicationDirPath() + QStringLiteral("/../resources/browser-session-bridge/") + relativePath,
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

    const qreal radius = size * 0.22;
    const QRectF rect(0, 0, size, size);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#181731")));
    painter.drawRoundedRect(rect, radius, radius);

    QLinearGradient shine(rect.topLeft(), rect.bottomRight());
    shine.setColorAt(0.0, QColor(255, 255, 255, 48));
    shine.setColorAt(0.45, QColor(108, 111, 255, 18));
    shine.setColorAt(1.0, QColor(5, 7, 19, 0));
    painter.setBrush(shine);
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), radius, radius);

    const qreal cx = size * 0.5;
    const qreal cy = size * 0.5;
    const qreal outer = size * 0.33;
    const QRectF arcRect(cx - outer, cy - outer, outer * 2.0, outer * 2.0);
    const qreal arcWidth = qMax<qreal>(2.0, size * 0.07);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(QStringLiteral("#353456")), arcWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawEllipse(arcRect);

    painter.setPen(QPen(QColor(QStringLiteral("#82f4ff")), arcWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, 38 * 16, 72 * 16);
    painter.setPen(QPen(QColor(QStringLiteral("#ffc640")), arcWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, -48 * 16, -58 * 16);
    painter.setPen(QPen(QColor(QStringLiteral("#3ae2ad")), arcWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, 190 * 16, 74 * 16);

    painter.setPen(QPen(QColor(235, 248, 255, 120), qMax<qreal>(1.0, size * 0.012), Qt::SolidLine, Qt::RoundCap));
    constexpr double kPi = 3.14159265358979323846;
    for (int i = 0; i < 8; ++i) {
        const qreal angle = (i * 45.0 - 90.0) * kPi / 180.0;
        const qreal r1 = outer * 0.72;
        const qreal r2 = outer * 0.86;
        painter.drawLine(QPointF(cx + std::cos(angle) * r1, cy + std::sin(angle) * r1),
                         QPointF(cx + std::cos(angle) * r2, cy + std::sin(angle) * r2));
    }

    QRadialGradient core(cx, cy, outer * 0.64);
    core.setColorAt(0.0, QColor(QStringLiteral("#f7fbff")));
    core.setColorAt(0.26, QColor(QStringLiteral("#7ce7f0")));
    core.setColorAt(0.68, QColor(73, 163, 176, 80));
    core.setColorAt(1.0, QColor(73, 163, 176, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(core);
    painter.drawEllipse(QPointF(cx, cy), outer * 0.48, outer * 0.48);
    painter.setBrush(QColor(QStringLiteral("#49a3b0")));
    painter.drawEllipse(QPointF(cx, cy), qMax<qreal>(2.0, size * 0.09), qMax<qreal>(2.0, size * 0.09));
    painter.setBrush(QColor(QStringLiteral("#f7fbff")));
    painter.drawEllipse(QPointF(cx, cy), qMax<qreal>(1.4, size * 0.04), qMax<qreal>(1.4, size * 0.04));

    painter.setPen(Qt::NoPen);
    const qreal nodeRadius = qMax<qreal>(1.1, size * 0.028);
    const QVector<QPair<QPointF, QColor>> nodes = {
        { QPointF(size * 0.73, size * 0.25), QColor(QStringLiteral("#85f5ff")) },
        { QPointF(size * 0.82, size * 0.58), QColor(QStringLiteral("#ffc640")) },
        { QPointF(size * 0.29, size * 0.76), QColor(QStringLiteral("#34e8bb")) },
        { QPointF(size * 0.20, size * 0.42), QColor(QStringLiteral("#62a8ff")) },
    };
    for (const auto& node : nodes) {
        painter.setBrush(node.second);
        painter.drawEllipse(node.first, nodeRadius, nodeRadius);
    }
    painter.end();

    return image.save(filePath, "PNG");
}

bool BrowserSessionBridgeInstallService::writeRuntimeConfig(const QString& destDir) const
{
    QJsonObject obj;
    obj[QStringLiteral("serverPort")] = DEFAULT_SERVER_PORT;
    obj[QStringLiteral("protocolVersion")] = BRIDGE_PROTOCOL_VERSION;
    obj[QStringLiteral("allowedOrigins")] = QJsonArray::fromStringList(
        BrowserSessionBridgeConstants::allowedOrigins()
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

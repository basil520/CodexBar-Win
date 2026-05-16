#pragma once

#include <QObject>
#include <QString>

class BrowserSessionBridgeInstallService : public QObject {
    Q_OBJECT
public:
    explicit BrowserSessionBridgeInstallService(QObject* parent = nullptr);

    bool ensureExtensionExported();
    bool isExtensionExported() const;
    QString extensionInstallPath() const;

private:
    bool exportFileFromQrc(const QString& qrcPath, const QString& destPath) const;
    bool ensureIconsGenerated(const QString& iconsDir) const;
    bool writeRuntimeConfig(const QString& destDir) const;
    bool generateIconPng(const QString& filePath, int size) const;

    QString extensionBasePath() const;
};

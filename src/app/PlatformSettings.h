#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class PlatformSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString secureStoreDisplayName READ secureStoreDisplayName CONSTANT)

public:
    explicit PlatformSettings(QObject* parent = nullptr);

    static QSettings appSettings();
    static QString secureStoreDisplayName();
};

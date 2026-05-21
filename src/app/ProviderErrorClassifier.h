#pragma once

#include <QObject>
#include <QVariantMap>

class ProviderErrorClassifier : public QObject {
    Q_OBJECT

public:
    explicit ProviderErrorClassifier(QObject* parent = nullptr);

    Q_INVOKABLE QVariantMap classify(const QString& providerId, const QString& rawMessage) const;
    Q_INVOKABLE QString copyText(const QString& providerId, const QString& rawMessage) const;

private:
    static QString redacted(const QString& text);
    static QString normalized(const QString& text);
};

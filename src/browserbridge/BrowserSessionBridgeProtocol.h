#pragma once

#include "BrowserSessionBridgeTypes.h"
#include "BrowserSessionBridgeCatalog.h"

#include <QJsonObject>
#include <optional>

constexpr int BRIDGE_PROTOCOL_VERSION = 1;

enum class BridgeMessageType {
    RegisterClient,
    RegisterAck,
    RequestImport,
    ImportResult,
    SessionDirty,
    Ping,
    Pong,
    Error
};

struct BridgeMessage {
    BridgeMessageType type;
    QJsonObject payload;
};

// register_client
struct RegisterClientPayload {
    int protocolVersion = BRIDGE_PROTOCOL_VERSION;
    QString extensionId;
    QString extensionBuild;
    QString browserFamily;
    QString browserVersion;
    QString profileInstanceId;
    QString profileAlias;
    bool incognito = false;
    bool supportsCookies = true;
    bool supportsLocalStorage = false;
    bool supportsCodexUsageSnapshot = false;
    bool supportsCookieUrlQuery = false;
    bool supportsAllUrlsCookiePermission = false;
};

// register_ack
struct RegisterAckPayload {
    int protocolVersion = BRIDGE_PROTOCOL_VERSION;
    bool accepted = false;
    QString errorMessage;
    QVector<BridgeProviderSpec> providerSpecs;
};

// request_import
struct RequestImportPayload {
    QString requestId;
    QString providerId;
    BridgeMaterialKind materialKind = BridgeMaterialKind::Cookies;
    QStringList domains;
    QStringList cookieNames;
    QString localStorageOrigin;
    QStringList localStorageKeys;
};

// import_result
struct ImportResultPayload {
    QString requestId;
    QString providerId;
    bool success = true;
    QString errorCode;
    QString errorMessage;
    QDateTime capturedAtUtc;
    QVector<BridgeCookieRecord> cookies;
    QHash<QString, QString> localStorage;
};

// session_dirty
struct SessionDirtyPayload {
    QStringList providerIds;
    QString reason;     // cookie_changed, storage_changed
};

// error
struct BridgeErrorPayload {
    QString code;
    QString message;
};

Q_DECLARE_METATYPE(ImportResultPayload)
Q_DECLARE_METATYPE(SessionDirtyPayload)

namespace BridgeProtocol {

RegisterClientPayload parseRegisterClient(const QJsonObject& obj);
QJsonObject serializeRegisterClient(const RegisterClientPayload& p);

RegisterAckPayload parseRegisterAck(const QJsonObject& obj);
QJsonObject serializeRegisterAck(const RegisterAckPayload& p);

RequestImportPayload parseRequestImport(const QJsonObject& obj);
QJsonObject serializeRequestImport(const RequestImportPayload& p);

ImportResultPayload parseImportResult(const QJsonObject& obj);
QJsonObject serializeImportResult(const ImportResultPayload& p);

SessionDirtyPayload parseSessionDirty(const QJsonObject& obj);
QJsonObject serializeSessionDirty(const SessionDirtyPayload& p);

BridgeErrorPayload parseError(const QJsonObject& obj);
QJsonObject serializeError(const BridgeErrorPayload& p);

BridgeMessageType messageTypeFromString(const QString& s);
QString messageTypeToString(BridgeMessageType t);

std::optional<BridgeMessage> parseMessage(const QByteArray& data);
QByteArray serializeMessage(const BridgeMessage& msg);

}

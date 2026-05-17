#include "BrowserSessionBridgeProtocol.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace {

QJsonObject cookieToJson(const BridgeCookieRecord& c)
{
    QJsonObject o;
    o[QStringLiteral("name")] = c.name;
    o[QStringLiteral("value")] = c.value;
    o[QStringLiteral("domain")] = c.domain;
    o[QStringLiteral("path")] = c.path;
    o[QStringLiteral("sameSite")] = c.sameSite;
    o[QStringLiteral("storeId")] = c.storeId;
    o[QStringLiteral("secure")] = c.secure;
    o[QStringLiteral("httpOnly")] = c.httpOnly;
    o[QStringLiteral("hostOnly")] = c.hostOnly;
    o[QStringLiteral("session")] = c.session;
    if (c.expirationDateUtc.has_value())
        o[QStringLiteral("expirationDate")] = c.expirationDateUtc->toSecsSinceEpoch();
    if (!c.partitionKey.isEmpty())
        o[QStringLiteral("partitionKey")] = c.partitionKey;
    return o;
}

BridgeCookieRecord jsonToCookie(const QJsonObject& o)
{
    BridgeCookieRecord c;
    c.name = o[QStringLiteral("name")].toString();
    c.value = o[QStringLiteral("value")].toString();
    c.domain = o[QStringLiteral("domain")].toString();
    c.path = o[QStringLiteral("path")].toString();
    c.sameSite = o[QStringLiteral("sameSite")].toString();
    c.storeId = o[QStringLiteral("storeId")].toString();
    c.secure = o[QStringLiteral("secure")].toBool();
    c.httpOnly = o[QStringLiteral("httpOnly")].toBool();
    c.hostOnly = o[QStringLiteral("hostOnly")].toBool();
    c.session = o[QStringLiteral("session")].toBool();
    const auto expiration = o[QStringLiteral("expirationDate")];
    if (!expiration.isNull() && !expiration.isUndefined()) {
        c.expirationDateUtc = QDateTime::fromSecsSinceEpoch(expiration.toInteger());
    }
    c.partitionKey = o[QStringLiteral("partitionKey")].toString();
    return c;
}

BridgeMaterialKind materialKindFromString(const QString& s)
{
    if (s == QStringLiteral("localStorage")) return BridgeMaterialKind::LocalStorage;
    if (s == QStringLiteral("hybrid")) return BridgeMaterialKind::Hybrid;
    return BridgeMaterialKind::Cookies;
}

QString materialKindToString(BridgeMaterialKind k)
{
    switch (k) {
    case BridgeMaterialKind::Cookies: return QStringLiteral("cookies");
    case BridgeMaterialKind::LocalStorage: return QStringLiteral("localStorage");
    case BridgeMaterialKind::Hybrid: return QStringLiteral("hybrid");
    }
    return QStringLiteral("cookies");
}

QJsonObject specToJson(const BridgeProviderSpec& s)
{
    QJsonObject o;
    o[QStringLiteral("providerId")] = s.providerId;
    o[QStringLiteral("materialKind")] = materialKindToString(s.materialKind);
    {
        QJsonArray arr;
        for (const auto& d : s.domains) arr.append(d);
        o[QStringLiteral("domains")] = arr;
    }
    {
        QJsonArray arr;
        for (const auto& n : s.cookieNames) arr.append(n);
        o[QStringLiteral("cookieNames")] = arr;
    }
    if (!s.localStorageOrigin.isEmpty())
        o[QStringLiteral("origin")] = s.localStorageOrigin;
    {
        QJsonArray arr;
        for (const auto& k : s.localStorageKeys) arr.append(k);
        o[QStringLiteral("localStorageKeys")] = arr;
    }
    o[QStringLiteral("supportsAutoSync")] = s.supportsAutoSync;
    return o;
}

BridgeProviderSpec jsonToSpec(const QJsonObject& o)
{
    BridgeProviderSpec s;
    s.providerId = o[QStringLiteral("providerId")].toString();
    s.materialKind = materialKindFromString(o[QStringLiteral("materialKind")].toString());
    {
        const auto arr = o[QStringLiteral("domains")].toArray();
        for (const auto& v : arr) s.domains.append(v.toString());
    }
    {
        const auto arr = o[QStringLiteral("cookieNames")].toArray();
        for (const auto& v : arr) s.cookieNames.append(v.toString());
    }
    s.localStorageOrigin = o[QStringLiteral("origin")].toString();
    {
        const auto arr = o[QStringLiteral("localStorageKeys")].toArray();
        for (const auto& v : arr) s.localStorageKeys.append(v.toString());
    }
    s.supportsAutoSync = o[QStringLiteral("supportsAutoSync")].toBool(true);
    return s;
}

} // namespace

namespace BridgeProtocol {

RegisterClientPayload parseRegisterClient(const QJsonObject& obj)
{
    RegisterClientPayload p;
    p.protocolVersion = obj[QStringLiteral("protocolVersion")].toInt(BRIDGE_PROTOCOL_VERSION);
    p.extensionId = obj[QStringLiteral("extensionId")].toString();
    p.extensionBuild = obj[QStringLiteral("extensionBuild")].toString();
    p.browserFamily = obj[QStringLiteral("browserFamily")].toString();
    p.browserVersion = obj[QStringLiteral("browserVersion")].toString();
    p.profileInstanceId = obj[QStringLiteral("profileInstanceId")].toString();
    p.profileAlias = obj[QStringLiteral("profileAlias")].toString();
    p.incognito = obj[QStringLiteral("incognito")].toBool();
    const auto caps = obj[QStringLiteral("capabilities")].toObject();
    p.supportsCookies = caps[QStringLiteral("cookies")].toBool(true);
    p.supportsLocalStorage = caps[QStringLiteral("localStorage")].toBool(false);
    p.supportsCodexUsageSnapshot = caps[QStringLiteral("codexUsageSnapshot")].toBool(false);
    return p;
}

QJsonObject serializeRegisterClient(const RegisterClientPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("register_client");
    obj[QStringLiteral("protocolVersion")] = p.protocolVersion;
    obj[QStringLiteral("extensionId")] = p.extensionId;
    obj[QStringLiteral("extensionBuild")] = p.extensionBuild;
    obj[QStringLiteral("browserFamily")] = p.browserFamily;
    obj[QStringLiteral("browserVersion")] = p.browserVersion;
    obj[QStringLiteral("profileInstanceId")] = p.profileInstanceId;
    obj[QStringLiteral("profileAlias")] = p.profileAlias;
    obj[QStringLiteral("incognito")] = p.incognito;
    {
        QJsonObject caps;
        caps[QStringLiteral("cookies")] = p.supportsCookies;
        caps[QStringLiteral("localStorage")] = p.supportsLocalStorage;
        caps[QStringLiteral("codexUsageSnapshot")] = p.supportsCodexUsageSnapshot;
        obj[QStringLiteral("capabilities")] = caps;
    }
    return obj;
}

RegisterAckPayload parseRegisterAck(const QJsonObject& obj)
{
    RegisterAckPayload p;
    p.protocolVersion = obj[QStringLiteral("protocolVersion")].toInt(BRIDGE_PROTOCOL_VERSION);
    p.accepted = obj[QStringLiteral("accepted")].toBool();
    p.errorMessage = obj[QStringLiteral("errorMessage")].toString();
    {
        const auto arr = obj[QStringLiteral("providerSpecs")].toArray();
        for (const auto& v : arr)
            p.providerSpecs.append(jsonToSpec(v.toObject()));
    }
    return p;
}

QJsonObject serializeRegisterAck(const RegisterAckPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("register_ack");
    obj[QStringLiteral("protocolVersion")] = p.protocolVersion;
    obj[QStringLiteral("accepted")] = p.accepted;
    if (!p.errorMessage.isEmpty())
        obj[QStringLiteral("errorMessage")] = p.errorMessage;
    {
        QJsonArray arr;
        for (const auto& s : p.providerSpecs) arr.append(specToJson(s));
        obj[QStringLiteral("providerSpecs")] = arr;
    }
    return obj;
}

RequestImportPayload parseRequestImport(const QJsonObject& obj)
{
    RequestImportPayload p;
    p.requestId = obj[QStringLiteral("requestId")].toString();
    p.providerId = obj[QStringLiteral("providerId")].toString();
    p.materialKind = materialKindFromString(obj[QStringLiteral("materialKind")].toString());
    {
        const auto arr = obj[QStringLiteral("domains")].toArray();
        for (const auto& v : arr) p.domains.append(v.toString());
    }
    {
        const auto arr = obj[QStringLiteral("cookieNames")].toArray();
        for (const auto& v : arr) p.cookieNames.append(v.toString());
    }
    p.localStorageOrigin = obj[QStringLiteral("origin")].toString();
    {
        const auto arr = obj[QStringLiteral("localStorageKeys")].toArray();
        for (const auto& v : arr) p.localStorageKeys.append(v.toString());
    }
    return p;
}

QJsonObject serializeRequestImport(const RequestImportPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("request_import");
    obj[QStringLiteral("requestId")] = p.requestId;
    obj[QStringLiteral("providerId")] = p.providerId;
    obj[QStringLiteral("materialKind")] = materialKindToString(p.materialKind);
    {
        QJsonArray arr;
        for (const auto& d : p.domains) arr.append(d);
        obj[QStringLiteral("domains")] = arr;
    }
    {
        QJsonArray arr;
        for (const auto& n : p.cookieNames) arr.append(n);
        obj[QStringLiteral("cookieNames")] = arr;
    }
    if (!p.localStorageOrigin.isEmpty())
        obj[QStringLiteral("origin")] = p.localStorageOrigin;
    {
        QJsonArray arr;
        for (const auto& k : p.localStorageKeys) arr.append(k);
        obj[QStringLiteral("localStorageKeys")] = arr;
    }
    return obj;
}

ImportResultPayload parseImportResult(const QJsonObject& obj)
{
    ImportResultPayload p;
    p.requestId = obj[QStringLiteral("requestId")].toString();
    p.providerId = obj[QStringLiteral("providerId")].toString();
    p.success = obj[QStringLiteral("success")].toBool(true);
    p.errorCode = obj[QStringLiteral("errorCode")].toString();
    p.errorMessage = obj[QStringLiteral("errorMessage")].toString();
    p.capturedAtUtc = QDateTime::fromString(
        obj[QStringLiteral("capturedAtUtc")].toString(), Qt::ISODate);
    {
        const auto arr = obj[QStringLiteral("cookies")].toArray();
        for (const auto& v : arr)
            p.cookies.append(jsonToCookie(v.toObject()));
    }
    {
        const auto ls = obj[QStringLiteral("localStorage")].toObject();
        for (auto it = ls.constBegin(); it != ls.constEnd(); ++it)
            p.localStorage[it.key()] = it.value().toString();
    }
    return p;
}

QJsonObject serializeImportResult(const ImportResultPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("import_result");
    obj[QStringLiteral("requestId")] = p.requestId;
    obj[QStringLiteral("providerId")] = p.providerId;
    obj[QStringLiteral("success")] = p.success;
    if (!p.errorCode.isEmpty())
        obj[QStringLiteral("errorCode")] = p.errorCode;
    if (!p.errorMessage.isEmpty())
        obj[QStringLiteral("errorMessage")] = p.errorMessage;
    obj[QStringLiteral("capturedAtUtc")] = p.capturedAtUtc.toString(Qt::ISODate);
    {
        QJsonArray arr;
        for (const auto& c : p.cookies) arr.append(cookieToJson(c));
        obj[QStringLiteral("cookies")] = arr;
    }
    {
        QJsonObject ls;
        for (auto it = p.localStorage.constBegin(); it != p.localStorage.constEnd(); ++it)
            ls[it.key()] = it.value();
        obj[QStringLiteral("localStorage")] = ls;
    }
    return obj;
}

SessionDirtyPayload parseSessionDirty(const QJsonObject& obj)
{
    SessionDirtyPayload p;
    {
        const auto arr = obj[QStringLiteral("providerIds")].toArray();
        for (const auto& v : arr) p.providerIds.append(v.toString());
    }
    p.reason = obj[QStringLiteral("reason")].toString();
    return p;
}

QJsonObject serializeSessionDirty(const SessionDirtyPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("session_dirty");
    {
        QJsonArray arr;
        for (const auto& id : p.providerIds) arr.append(id);
        obj[QStringLiteral("providerIds")] = arr;
    }
    obj[QStringLiteral("reason")] = p.reason;
    return obj;
}

BridgeErrorPayload parseError(const QJsonObject& obj)
{
    BridgeErrorPayload p;
    p.code = obj[QStringLiteral("code")].toString();
    p.message = obj[QStringLiteral("message")].toString();
    return p;
}

QJsonObject serializeError(const BridgeErrorPayload& p)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = QStringLiteral("error");
    obj[QStringLiteral("code")] = p.code;
    obj[QStringLiteral("message")] = p.message;
    return obj;
}

BridgeMessageType messageTypeFromString(const QString& s)
{
    if (s == QStringLiteral("register_client")) return BridgeMessageType::RegisterClient;
    if (s == QStringLiteral("register_ack"))    return BridgeMessageType::RegisterAck;
    if (s == QStringLiteral("request_import"))  return BridgeMessageType::RequestImport;
    if (s == QStringLiteral("import_result"))   return BridgeMessageType::ImportResult;
    if (s == QStringLiteral("session_dirty"))   return BridgeMessageType::SessionDirty;
    if (s == QStringLiteral("ping"))            return BridgeMessageType::Ping;
    if (s == QStringLiteral("pong"))            return BridgeMessageType::Pong;
    if (s == QStringLiteral("error"))           return BridgeMessageType::Error;
    return BridgeMessageType::Error;
}

QString messageTypeToString(BridgeMessageType t)
{
    switch (t) {
    case BridgeMessageType::RegisterClient: return QStringLiteral("register_client");
    case BridgeMessageType::RegisterAck:    return QStringLiteral("register_ack");
    case BridgeMessageType::RequestImport:  return QStringLiteral("request_import");
    case BridgeMessageType::ImportResult:   return QStringLiteral("import_result");
    case BridgeMessageType::SessionDirty:   return QStringLiteral("session_dirty");
    case BridgeMessageType::Ping:           return QStringLiteral("ping");
    case BridgeMessageType::Pong:           return QStringLiteral("pong");
    case BridgeMessageType::Error:          return QStringLiteral("error");
    }
    return QStringLiteral("error");
}

std::optional<BridgeMessage> parseMessage(const QByteArray& data)
{
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
        return std::nullopt;
    if (!doc.isObject())
        return std::nullopt;
    const auto obj = doc.object();
    const auto typeStr = obj[QStringLiteral("type")].toString();
    if (typeStr.isEmpty())
        return std::nullopt;
    BridgeMessage msg;
    msg.type = messageTypeFromString(typeStr);
    msg.payload = obj;
    return msg;
}

QByteArray serializeMessage(const BridgeMessage& msg)
{
    QJsonDocument doc(msg.payload);
    return doc.toJson(QJsonDocument::Compact);
}

} // namespace BridgeProtocol

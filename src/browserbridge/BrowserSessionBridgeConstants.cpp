#include "BrowserSessionBridgeConstants.h"

namespace BrowserSessionBridgeConstants {

QString extensionId()
{
    return QStringLiteral("cnanalhpjiclhljkpnlbgiaclpbncidk");
}

QString extensionOrigin()
{
    return QStringLiteral("chrome-extension://%1").arg(extensionId());
}

QStringList allowedOrigins()
{
    return { extensionOrigin() };
}

} // namespace BrowserSessionBridgeConstants

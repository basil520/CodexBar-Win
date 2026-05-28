#pragma once

#include <QString>

class LaunchAtLoginController {
public:
    static bool isEnabled();
    static bool setEnabled(bool enabled, QString* errorMessage = nullptr);
    static QString launchAgentPath();
};

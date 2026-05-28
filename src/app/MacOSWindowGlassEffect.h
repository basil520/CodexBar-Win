#pragma once

#include <QColor>

class QWindow;

bool macOSGlassEffectIsAvailable();
bool applyMacOSGlassEffect(QWindow* window, bool enabled, QColor tint);

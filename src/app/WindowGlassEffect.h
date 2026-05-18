#pragma once

#include <QColor>

class QWindow;

namespace WindowGlassEffect {

bool isAvailable();
void prepare(QWindow* window);
QColor clearColor(bool enabled, const QColor& fallback);
bool apply(QWindow* window, bool enabled, QColor tint);

} // namespace WindowGlassEffect

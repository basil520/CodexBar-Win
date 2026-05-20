#include "TrayIconRenderer.h"

#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QtGlobal>

QColor TrayIconRenderer::s_trackColor = QColor(60, 60, 80);

void TrayIconRenderer::setTrackColor(const QColor& color) {
    s_trackColor = color;
}

QIcon TrayIconRenderer::makeDefaultIcon() {
    return makeIcon(100.0, 100.0, std::nullopt, false, IconStyle::Default, 0, 20, 10);
}

QIcon TrayIconRenderer::makeIcon(
    const std::optional<double>& primaryRemaining,
    const std::optional<double>& weeklyRemaining,
    const std::optional<double>& creditsRemaining,
    bool stale,
    IconStyle style,
    int displayMode,
    int warningThreshold,
    int criticalThreshold,
    double blink)
{
    Q_UNUSED(creditsRemaining)
    Q_UNUSED(blink)

    double p = primaryRemaining.value_or(100.0);
    double w = weeklyRemaining.value_or(100.0);

    QIcon icon;
    icon.addPixmap(renderPixmap(16, p, w, stale, style, displayMode, warningThreshold, criticalThreshold));
    icon.addPixmap(renderPixmap(24, p, w, stale, style, displayMode, warningThreshold, criticalThreshold));
    icon.addPixmap(renderPixmap(32, p, w, stale, style, displayMode, warningThreshold, criticalThreshold));
    return icon;
}

QPixmap TrayIconRenderer::renderPixmap(
    int size,
    const std::optional<double>& primary,
    const std::optional<double>& weekly,
    bool stale,
    IconStyle style,
    int displayMode,
    int warningThreshold,
    int criticalThreshold)
{
    Q_UNUSED(style)
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    double pRem = std::clamp(primary.value_or(100.0) / 100.0, 0.0, 1.0);
    double wRem = std::clamp(weekly.value_or(100.0) / 100.0, 0.0, 1.0);

    double pPct = pRem * 100.0;
    QColor primaryColor = pPct > warningThreshold ? QColor(64, 200, 64) :
                           pPct > criticalThreshold ? QColor(220, 180, 40) :
                           QColor(220, 60, 60);

    double wPct = wRem * 100.0;
    QColor weeklyColor = wPct > warningThreshold ? QColor(64, 160, 200) :
                          wPct > criticalThreshold ? QColor(200, 160, 40) :
                          QColor(200, 60, 60);

    if (stale) {
        primaryColor = primaryColor.darker(200);
        weeklyColor = weeklyColor.darker(200);
    }

    if (displayMode == 1 || displayMode == 2 || displayMode == 3) {
        // Draw standard modern squircle background
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(30, 30, 45, 180));
        painter.drawRoundedRect(0, 0, size, size, size / 4.0, size / 4.0);

        // Text formatting
        QString text;
        if (displayMode == 1) {
            text = QString::number(static_cast<int>(pPct));
            if (size >= 24) text += "%";
        } else {
            // Estimated remaining hours
            double hours = pRem * 24.0;
            if (hours >= 1.0) {
                text = QString("%1h").arg(static_cast<int>(hours));
            } else {
                text = QString("%1m").arg(qMax(1, static_cast<int>(hours * 60.0)));
            }
        }

        // Beautiful font size adjustment
        QFont font("Segoe UI", size / 3, QFont::Bold);
        painter.setFont(font);
        painter.setPen(primaryColor);
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, text);
    } else {
        // Default dual progress bars
        int barHeight = qMax(2, size / 10);
        int spacing = qMax(1, size / 24);
        int topY = spacing;
        int bottomY = spacing + barHeight + spacing;

        painter.fillRect(QRect(0, topY, size, barHeight), s_trackColor);
        painter.fillRect(QRect(0, topY, static_cast<int>(size * pRem), barHeight), primaryColor);

        barHeight = qMax(1, size / 14);
        painter.fillRect(QRect(0, bottomY, size, barHeight), s_trackColor);
        painter.fillRect(QRect(0, bottomY, static_cast<int>(size * wRem), barHeight), weeklyColor);
    }

    painter.end();
    return pixmap;
}

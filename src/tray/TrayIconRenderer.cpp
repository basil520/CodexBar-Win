#include "TrayIconRenderer.h"

#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QtGlobal>
#include <algorithm>

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
        painter.setPen(Qt::NoPen);
        QLinearGradient base(0, 0, size, size);
        base.setColorAt(0.0, QColor(43, 42, 80, 235));
        base.setColorAt(0.48, QColor(24, 23, 49, 235));
        base.setColorAt(1.0, QColor(14, 20, 38, 235));
        painter.setBrush(base);
        painter.drawRoundedRect(QRectF(0.5, 0.5, size - 1.0, size - 1.0), size * 0.22, size * 0.22);

        const qreal cx = size * 0.5;
        const qreal cy = size * 0.5;
        const qreal margin = qMax<qreal>(2.0, size * 0.18);
        const QRectF arcRect(margin, margin, size - margin * 2.0, size - margin * 2.0);
        const qreal arcWidth = qMax<qreal>(1.5, size * 0.12);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(s_trackColor, arcWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawEllipse(arcRect);

        painter.setPen(QPen(primaryColor, arcWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, 38 * 16, static_cast<int>(112 * pRem) * 16);

        painter.setPen(QPen(weeklyColor, arcWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, 190 * 16, static_cast<int>(88 * wRem) * 16);

        painter.setPen(QPen(QColor(255, 198, 64), arcWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, -50 * 16, -qMax(18, static_cast<int>(62 * std::min(pRem, wRem))) * 16);

        if (size >= 24) {
            painter.setPen(QPen(QColor(235, 248, 255, stale ? 60 : 95),
                                qMax<qreal>(1.0, size * 0.04),
                                Qt::SolidLine,
                                Qt::RoundCap));
            painter.drawLine(QPointF(cx, margin + arcWidth),
                             QPointF(cx, size - margin - arcWidth));
            painter.drawLine(QPointF(margin + arcWidth, cy),
                             QPointF(size - margin - arcWidth, cy));
        }

        QRadialGradient core(cx, cy, size * 0.28);
        core.setColorAt(0.0, QColor(247, 251, 255, stale ? 120 : 230));
        core.setColorAt(0.28, QColor(124, 231, 240, stale ? 90 : 190));
        core.setColorAt(0.78, QColor(73, 163, 176, stale ? 24 : 82));
        core.setColorAt(1.0, QColor(73, 163, 176, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(core);
        painter.drawEllipse(QPointF(cx, cy), size * 0.24, size * 0.24);
        painter.setBrush(QColor(247, 251, 255, stale ? 130 : 245));
        painter.drawEllipse(QPointF(cx, cy), qMax<qreal>(1.4, size * 0.06), qMax<qreal>(1.4, size * 0.06));

        if (size >= 24) {
            painter.setBrush(QColor(133, 245, 255, stale ? 95 : 230));
            painter.drawEllipse(QPointF(size * 0.72, size * 0.25), size * 0.055, size * 0.055);
            painter.setBrush(QColor(52, 232, 187, stale ? 95 : 230));
            painter.drawEllipse(QPointF(size * 0.28, size * 0.76), size * 0.055, size * 0.055);
        }
    }

    painter.end();
    return pixmap;
}

#include <QApplication>
#include <QFont>
#include <QClipboard>
#include <QColor>
#include <QDesktopServices>
#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QProcess>
#include <QQuickItem>
#include <QQuickView>
#include <QScreen>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>
#include <QtQml>
#include <QThreadPool>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <functional>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#else
#include <unistd.h>
#endif

#include "cli/CLIEntry.h"
#include "app/AppTheme.h"
#include "app/PlatformSettings.h"
#include "app/PerformanceState.h"
#include "app/WindowGlassEffect.h"
#include "tray/TrayIconRenderer.h"

#include <QQuickView>

static bool activateExistingInstance() {
#ifdef Q_OS_WIN
    HWND hwnd = FindWindowW(nullptr, L"CodexBarX");
    if (hwnd) {
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        return true;
    }
    // Try to find by window class if title doesn't match
    hwnd = FindWindowW(L"Qt5152QWindowIcon", nullptr);
    if (hwnd) {
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        return true;
    }
#endif
    return false;
}

static QString g_logPath;
static QtMessageHandler g_previousMessageHandler = nullptr;

static void fileMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    if (!g_logPath.isEmpty()) {
        QFile file(g_logPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            file.write(qFormatLogMessage(type, context, message).toUtf8());
            file.write("\n");
        }
    }
    if (g_previousMessageHandler) {
        g_previousMessageHandler(type, context, message);
    }
}

#ifdef QT_DEBUG
static void debugLogImpl(const QString& msg) {
    static QString logPath = QDir::tempPath() + "/CodexBarX_MonitorDebug.log";
    QFile f(logPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        f.write(QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toUtf8());
        f.write(" ");
        f.write(msg.toUtf8());
        f.write("\n");
    }
    qDebug().noquote() << msg;
}
#define debugLog(msg) debugLogImpl(msg)
#else
#define debugLog(msg) ((void)0)
#endif

#include "app/SettingsStore.h"
#include "app/SettingsProvidersModel.h"
#include "app/TrayViewModel.h"
#include "app/UiFreezeWatchdog.h"
#include "app/UsageDetailsViewModel.h"
#include "app/UsageStore.h"
#include "app/LanguageManager.h"
#include "app/ProviderErrorClassifier.h"
#include "app/SessionQuotaNotifications.h"
#include "tray/StatusItemController.h"
#include "util/SingleInstanceGuard.h"
#include "network/NetworkManager.h"
#include "util/CostUsageScanner.h"
#include "providers/ProviderBootstrap.h"
#include "account/TokenAccountStore.h"
#include "runtime/ProviderRuntimeManager.h"
#include "browserbridge/BrowserSessionBridgeStore.h"
#include "browserbridge/BrowserSessionBridgeService.h"
#include "app/BridgeViewModel.h"

#ifdef Q_OS_WIN
struct RoundedWindowAccentPolicy {
    int accentState = 0; // ACCENT_DISABLED
    int accentFlags = 0;
    int gradientColor = 0;
    int animationId = 0;
};

struct RoundedWindowCompositionAttrData {
    int attribute = 19; // WCA_ACCENT_POLICY
    void* data = nullptr;
    size_t sizeOfData = 0;
};

static void disableRectangularDwmEffects(HWND hwnd) {
    // WindowGlassEffect::apply() enables DWM layers that paint across the full
    // rectangular window and cannot be constrained to a region:
    //   - DWMWA_SYSTEMBACKDROP_TYPE (Win11 system acrylic)
    //   - ACCENT_ENABLE_ACRYLICBLURBEHIND (Win10 accent tint)
    // These bleed through transparent QML corners as a "rectangular shell".
    // Disable them here; the QML AcrylicBackdrop provides visual tint/noise.

    // Disable Win11 system backdrop
    auto* fnSetAttr = reinterpret_cast<HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD)>(
        GetProcAddress(GetModuleHandleW(L"dwmapi.dll"), "DwmSetWindowAttribute"));
    if (fnSetAttr) {
        const int none = 1; // DWMSBT_NONE
        fnSetAttr(hwnd, 38 /*DWMWA_SYSTEMBACKDROP_TYPE*/, &none, sizeof(none));
    }

    // Disable Win10 accent policy
    auto* fnSetCompAttr = reinterpret_cast<BOOL(WINAPI*)(HWND, void*)>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));
    if (fnSetCompAttr) {
        RoundedWindowAccentPolicy policy;
        RoundedWindowCompositionAttrData data;
        data.data = &policy;
        data.sizeOfData = sizeof(policy);
        fnSetCompAttr(hwnd, &data);
    }
}

static void applyRoundedWindowRegion(QWindow* window, int radius) {
    if (!window) return;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) return;

    const qreal scale = window->devicePixelRatio();
    const int width = std::max(1, static_cast<int>(std::lround(window->width() * scale)));
    const int height = std::max(1, static_cast<int>(std::lround(window->height() * scale)));
    const int diameter = std::max(1, static_cast<int>(std::lround(radius * 2 * scale)));

    HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, diameter, diameter);
    if (!region) return;

    if (SetWindowRgn(hwnd, region, TRUE) == FALSE) {
        DeleteObject(region);
        return;
    }

    // Disable rectangular DWM effects, then re-enable only the blur
    // constrained to the rounded region.
    disableRectangularDwmEffects(hwnd);

    HRGN blurRegion = CreateRoundRectRgn(0, 0, width + 1, height + 1, diameter, diameter);
    if (blurRegion) {
        DWM_BLURBEHIND blur = {};
        blur.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        blur.fEnable = TRUE;
        blur.hRgnBlur = blurRegion;
        DwmEnableBlurBehindWindow(hwnd, &blur);
        DeleteObject(blurRegion);
    }

    // Force DWM to recompose
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

static void updateTrayPanelWindowShape(QQuickView* view, bool glassEnabled)
{
    if (!view) return;
    HWND hwnd = reinterpret_cast<HWND>(view->winId());
    if (!hwnd) return;

    if (glassEnabled) {
        // Glass ON: rectangular window so DWM backdrop covers full area
        SetWindowRgn(hwnd, nullptr, TRUE);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    } else {
        // Glass OFF: rounded window shape
        applyRoundedWindowRegion(view, 12);
    }
}

// Get the work area (available geometry excluding taskbar) of the monitor
// that contains the given point, using Win32 APIs directly.
// Returns coordinates in virtual-desktop logical pixels (same as Qt uses).
static QRect win32MonitorWorkArea(const QPoint& point) {
    POINT pt = { point.x(), point.y() };
    HMONITOR hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (hmon) {
        MONITORINFOEXW mi = {};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(hmon, &mi)) {
            // rcWork is in physical pixels; convert to logical if scaled
            // MonitorFromPoint returns the same coordinate space as SetWindowPos,
            // so we return as-is. Qt and Win32 share virtual desktop coords at dpr=1.
            // For HiDPI: MONITORINFOEX rcWork is in physical pixels, but
            // Qt virtual-desktop coords are logical. We need the logical coords.
            // Use the monitor's DPI to convert.
            UINT dpiX = 96, dpiY = 96;
            // GetDpiForMonitor requires Windows 8.1+, fallback if unavailable
            typedef HRESULT(WINAPI* GetDpiForMonitorFunc)(HMONITOR, UINT, UINT*, UINT*);
            static GetDpiForMonitorFunc pGetDpiForMonitor = nullptr;
            static bool triedLoad = false;
            if (!triedLoad) {
                triedLoad = true;
                HMODULE hShcore = GetModuleHandleW(L"shcore");
                if (!hShcore) hShcore = LoadLibraryW(L"shcore");
                if (hShcore) pGetDpiForMonitor = reinterpret_cast<GetDpiForMonitorFunc>(
                    GetProcAddress(hShcore, "GetDpiForMonitor"));
            }
            if (pGetDpiForMonitor) {
                pGetDpiForMonitor(hmon, 0 /*MDT_EFFECTIVE_DPI*/, &dpiX, &dpiY);
            }
            double scale = dpiX / 96.0;
            return QRect(
                static_cast<int>(mi.rcWork.left / scale),
                static_cast<int>(mi.rcWork.top / scale),
                static_cast<int>((mi.rcWork.right - mi.rcWork.left) / scale),
                static_cast<int>((mi.rcWork.bottom - mi.rcWork.top) / scale));
        }
    }
    return QRect();
}

// Get the full geometry of the monitor that contains the given point.
static QRect win32MonitorGeometry(const QPoint& point) {
    POINT pt = { point.x(), point.y() };
    HMONITOR hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (hmon) {
        MONITORINFOEXW mi = {};
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW(hmon, &mi)) {
            UINT dpiX = 96, dpiY = 96;
            typedef HRESULT(WINAPI* GetDpiForMonitorFunc)(HMONITOR, UINT, UINT*, UINT*);
            static GetDpiForMonitorFunc pGetDpiForMonitor = nullptr;
            static bool triedLoad = false;
            if (!triedLoad) {
                triedLoad = true;
                HMODULE hShcore = GetModuleHandleW(L"shcore");
                if (!hShcore) hShcore = LoadLibraryW(L"shcore");
                if (hShcore) pGetDpiForMonitor = reinterpret_cast<GetDpiForMonitorFunc>(
                    GetProcAddress(hShcore, "GetDpiForMonitor"));
            }
            if (pGetDpiForMonitor) {
                pGetDpiForMonitor(hmon, 0 /*MDT_EFFECTIVE_DPI*/, &dpiX, &dpiY);
            }
            double scale = dpiX / 96.0;
            return QRect(
                static_cast<int>(mi.rcMonitor.left / scale),
                static_cast<int>(mi.rcMonitor.top / scale),
                static_cast<int>((mi.rcMonitor.right - mi.rcMonitor.left) / scale),
                static_cast<int>((mi.rcMonitor.bottom - mi.rcMonitor.top) / scale));
        }
    }
    return QRect();
}

static void forceWindowPosition(QWindow* window, int x, int y) {
    if (!window) return;
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) return;
    const qreal scale = window->devicePixelRatio();
    const int sx = static_cast<int>(std::lround(x * scale));
    const int sy = static_cast<int>(std::lround(y * scale));
    BOOL ok = SetWindowPos(hwnd, nullptr, sx, sy, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    RECT after;
    GetWindowRect(hwnd, &after);
    debugLog(QString("[forceWindowPosition] hwnd=%1 logical=(%2 %3) scale=%4 physical=(%5 %6) SetWindowPos=%7 actualRect=(%8 %9 %10 %11)")
        .arg(reinterpret_cast<quintptr>(hwnd)).arg(x).arg(y).arg(scale).arg(sx).arg(sy).arg(ok)
        .arg(after.left).arg(after.top).arg(after.right).arg(after.bottom));
}
#else
static void applyRoundedWindowRegion(QWindow* window, int radius) {
    Q_UNUSED(window);
    Q_UNUSED(radius);
}
static void updateTrayPanelWindowShape(QQuickView* view, bool glassEnabled)
{
    Q_UNUSED(view);
    Q_UNUSED(glassEnabled);
}
static void forceWindowPosition(QWindow* window, int x, int y) {
    Q_UNUSED(window);
    Q_UNUSED(x);
    Q_UNUSED(y);
}
static QRect win32MonitorWorkArea(const QPoint& point) {
    Q_UNUSED(point);
    return QRect();
}
static QRect win32MonitorGeometry(const QPoint& point) {
    Q_UNUSED(point);
    return QRect();
}
#endif

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool settingsVisible READ isSettingsVisible NOTIFY settingsVisibleChanged)
    Q_PROPERTY(bool settingsMaximized READ isSettingsMaximized NOTIFY settingsMaximizedChanged)
    Q_PROPERTY(bool usageVisible READ isUsageVisible NOTIFY usageVisibleChanged)
public:
    explicit AppController(QObject* parent = nullptr) : QObject(parent) {}

    QQuickView* settingsView = nullptr;
    QQuickView* trayView = nullptr;
    QQuickView* usageView = nullptr;
    std::function<void()> ensureSettingsLoaded;
    std::function<QScreen*()> screenForTray;
    std::function<QRect()> win32WorkAreaForTray;
    QPoint m_lastTargetPos;

    Q_INVOKABLE void openSettings() {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("settings.open"));
        if (!settingsView) return;
        if (ensureSettingsLoaded) ensureSettingsLoaded();
        if (!settingsView->isVisible()) {
            QRect avail = win32WorkAreaForTray ? win32WorkAreaForTray() : QRect();
            debugLog(QString("[openSettings] win32WorkArea=(%1 %2 %3 %4)")
                .arg(avail.x()).arg(avail.y()).arg(avail.width()).arg(avail.height()));
            if (avail.isEmpty()) {
                QScreen* screen = screenForTray ? screenForTray() : nullptr;
                if (screen) avail = screen->availableGeometry();
            }
            if (!avail.isEmpty()) {
                int maxH = avail.height() - 60;
                int h = qMin(960, maxH);
                int w = 900;
                int sx = (avail.width() - w) / 2 + avail.x();
                int sy = (avail.height() - h) / 2 + avail.y();
                settingsView->resize(w, h);
                m_lastTargetPos = QPoint(sx, sy);
                debugLog(QString("[openSettings] target=(%1 %2)").arg(sx).arg(sy));
            } else {
                m_lastTargetPos = QPoint();
            }
        }
        debugLog(QString("[openSettings] BEFORE show() pos=(%1 %2)").arg(settingsView->x()).arg(settingsView->y()));
        settingsView->show();
        debugLog(QString("[openSettings] AFTER show() pos=(%1 %2) screen=%3")
            .arg(settingsView->x()).arg(settingsView->y())
            .arg(settingsView->screen() ? settingsView->screen()->name() : "null"));
        if (!m_lastTargetPos.isNull()) forceWindowPosition(settingsView, m_lastTargetPos.x(), m_lastTargetPos.y());
        debugLog(QString("[openSettings] AFTER forceWindowPosition() pos=(%1 %2)").arg(settingsView->x()).arg(settingsView->y()));
        settingsView->raise();
        settingsView->requestActivate();
        QTimer::singleShot(100, [this]() {
            debugLog(QString("[openSettings] DEFERRED(100ms) pos=(%1 %2) screen=%3")
                .arg(settingsView->x()).arg(settingsView->y())
                .arg(settingsView->screen() ? settingsView->screen()->name() : "null"));
        });
        emit settingsVisibleChanged();
        emit settingsMaximizedChanged();
    }

    Q_INVOKABLE void startSettingsMove() {
        if (!settingsView) return;
        settingsView->startSystemMove();
    }

    Q_INVOKABLE void startSettingsResize(int edges) {
        if (!settingsView) return;
        settingsView->startSystemResize(Qt::Edges(edges));
    }

    Q_INVOKABLE void minimizeSettings() {
        if (!settingsView) return;
        settingsView->showMinimized();
        emit settingsVisibleChanged();
    }

    Q_INVOKABLE void toggleSettingsMaximized() {
        if (!settingsView) return;
        if (isSettingsMaximized()) {
            settingsView->showNormal();
        } else {
            settingsView->showMaximized();
        }
        emit settingsMaximizedChanged();
        emit settingsVisibleChanged();
    }

    Q_INVOKABLE void closeSettings() {
        if (!settingsView) return;
        settingsView->hide();
        emit settingsVisibleChanged();
    }

    Q_INVOKABLE void openUsage() {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("usage.open"));
        if (!usageView) return;
        if (usageView->source().isEmpty()) {
            usageView->setSource(QUrl("qrc:/qml/UsageWindow.qml"));
            if (usageView->status() == QQuickView::Error) return;
        }
        if (!usageView->isVisible()) {
            QRect avail = win32WorkAreaForTray ? win32WorkAreaForTray() : QRect();
            debugLog(QString("[openUsage] win32WorkArea=(%1 %2 %3 %4)")
                .arg(avail.x()).arg(avail.y()).arg(avail.width()).arg(avail.height()));
            if (avail.isEmpty()) {
                QScreen* screen = screenForTray ? screenForTray() : nullptr;
                if (screen) avail = screen->availableGeometry();
            }
            if (!avail.isEmpty()) {
                int w = 800, h = 600;
                int ux = (avail.width() - w) / 2 + avail.x();
                int uy = (avail.height() - h) / 2 + avail.y();
                usageView->resize(w, h);
                m_lastTargetPos = QPoint(ux, uy);
                debugLog(QString("[openUsage] target=(%1 %2)").arg(ux).arg(uy));
            } else {
                m_lastTargetPos = QPoint();
            }
        }
        debugLog(QString("[openUsage] BEFORE show() pos=(%1 %2)").arg(usageView->x()).arg(usageView->y()));
        usageView->show();
        debugLog(QString("[openUsage] AFTER show() pos=(%1 %2) screen=%3")
            .arg(usageView->x()).arg(usageView->y())
            .arg(usageView->screen() ? usageView->screen()->name() : "null"));
        if (!m_lastTargetPos.isNull()) forceWindowPosition(usageView, m_lastTargetPos.x(), m_lastTargetPos.y());
        debugLog(QString("[openUsage] AFTER forceWindowPosition() pos=(%1 %2)").arg(usageView->x()).arg(usageView->y()));
        usageView->raise();
        usageView->requestActivate();
        emit usageVisibleChanged();
    }

    Q_INVOKABLE void closeUsage() {
        if (!usageView) return;
        usageView->hide();
        emit usageVisibleChanged();
    }

    Q_INVOKABLE void toggleUsage() {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("usage.toggle"));
        if (!usageView) return;
        if (usageView->isVisible()) {
            usageView->hide();
        } else {
            openUsage();
        }
        emit usageVisibleChanged();
    }

    Q_INVOKABLE void startUsageMove() {
        if (!usageView) return;
        usageView->startSystemMove();
    }

    Q_INVOKABLE void startUsageResize(int edges) {
        if (!usageView) return;
        usageView->startSystemResize(Qt::Edges(edges));
    }

    Q_INVOKABLE void minimizeUsage() {
        if (!usageView) return;
        usageView->showMinimized();
        emit usageVisibleChanged();
    }

    Q_INVOKABLE void moveTrayPanel(int deltaX, int deltaY) {
        if (!trayView) return;
        QPoint pos = trayView->position();
        trayView->setPosition(pos.x() + deltaX, pos.y() + deltaY);
    }

    Q_INVOKABLE void toggleSettings() {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("settings.toggle"));
        if (!settingsView) return;
        if (settingsView->isVisible()) {
            settingsView->hide();
        } else {
            openSettings();
        }
        emit settingsVisibleChanged();
    }

    Q_INVOKABLE void quitApp() {
        emit forceQuitRequested();
    }

    Q_INVOKABLE void showAbout() {
        emit aboutRequested();
    }

    Q_INVOKABLE void openTerminal(const QString& command) {
#ifdef Q_OS_WIN
        QProcess::startDetached("cmd", {"/c", QString("start cmd /k %1").arg(command)});
#elif defined(Q_OS_MACOS)
        QString escaped = command;
        escaped.replace("\\", "\\\\");
        escaped.replace("\"", "\\\"");
        QProcess::startDetached(QStringLiteral("osascript"),
                                {QStringLiteral("-e"),
                                 QStringLiteral("tell application \"Terminal\" to activate"),
                                 QStringLiteral("-e"),
                                 QStringLiteral("tell application \"Terminal\" to do script \"%1\"").arg(escaped)});
#else
        QProcess::startDetached("x-terminal-emulator", {"-e", command});
#endif
    }

    Q_INVOKABLE void copyWithFeedback(const QString& text) {
        copyText(text);
        emit copyFeedbackTriggered(text);
    }

    Q_INVOKABLE void openExternalUrl(const QString& url) {
        QDesktopServices::openUrl(QUrl(url));
    }

    Q_INVOKABLE void copyText(const QString& text) {
        if (auto* clipboard = QGuiApplication::clipboard()) {
            clipboard->setText(text);
        }
    }

    bool isSettingsVisible() const {
        return settingsView && settingsView->isVisible();
    }

    bool isSettingsMaximized() const {
        return settingsView && (settingsView->windowState() & Qt::WindowMaximized);
    }

    bool isUsageVisible() const {
        return usageView && usageView->isVisible();
    }

signals:
    void settingsVisibleChanged();
    void settingsMaximizedChanged();
    void usageVisibleChanged();
    void forceQuitRequested();
    void aboutRequested();
    void copyFeedbackTriggered(const QString& text);
};

static void dumpQuickItemTree(QQuickItem* item, int depth = 0) {
    if (!item) {
        qWarning() << "Settings item tree: <null>";
        return;
    }
    const QString indent(depth * 2, QLatin1Char(' '));
    qWarning().noquote() << indent
                         << item->metaObject()->className()
                         << "objectName=" << item->objectName()
                         << "visible=" << item->isVisible()
                         << "opacity=" << item->opacity()
                         << "x=" << item->x()
                         << "y=" << item->y()
                         << "w=" << item->width()
                         << "h=" << item->height()
                         << "children=" << item->childItems().size();
    if (depth >= 4) return;
    for (auto* child : item->childItems()) {
        dumpQuickItemTree(child, depth + 1);
    }
}

int main(int argc, char* argv[]) {
    // Check for CLI subcommands before creating QApplication
    if (argc >= 2 && CLIEntry::isCliCommand(QString::fromUtf8(argv[1]))) {
        QCoreApplication app(argc, argv);
        app.setApplicationName("CodexBarX");
        app.setOrganizationName("CodexBarX");
        CLIEntry cli;
        return cli.run(argc, argv);
    }

    qputenv("QT_QUICK_CONTROLS_STYLE", QByteArray("Basic"));

    SingleInstanceGuard singleInstance(QStringLiteral("CodexBarX_SingleInstance"));
    if (singleInstance.alreadyRunning()) {
        activateExistingInstance();
        return 0;
    }
    if (!singleInstance.acquired()) {
        return 1;
    }

    QApplication app(argc, argv);
    {
        QFont defaultFont = app.font();
#if defined(Q_OS_WIN)
        defaultFont.setFamily(QStringLiteral("Segoe UI, Microsoft YaHei"));
#elif defined(Q_OS_MAC)
        defaultFont.setFamily(QStringLiteral(".AppleSystemUIFont, PingFang SC"));
#else
        defaultFont.setFamily(QStringLiteral("Inter, Noto Sans CJK SC"));
#endif
        app.setFont(defaultFont);
    }
    auto* uiFreezeWatchdog = new UiFreezeWatchdog(&app);

    app.setApplicationName("CodexBarX");
    app.setOrganizationName("CodexBarX");
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/AppIcon-codexbarx.svg")));
    app.setQuitOnLastWindowClosed(false);
    const QStringList appArgs = QCoreApplication::arguments();
    for (const auto& arg : appArgs) {
        if (arg.startsWith("--qml-log=")) {
            g_logPath = arg.mid(QStringLiteral("--qml-log=").size());
            g_previousMessageHandler = qInstallMessageHandler(fileMessageHandler);
        }
    }
    const bool showSettingsOnStartup = appArgs.contains("--show-settings");

    {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("bootstrap.registerProviders"));
        ProviderBootstrap::registerAllProviders();
    }

    // Increase global thread pool to handle concurrent provider refreshes
    // without exhausting workers. Default is QThread::idealThreadCount().
    int idealThreads = QThread::idealThreadCount();
    int poolMax = qMax(idealThreads * 2, 12);
    QThreadPool::globalInstance()->setMaxThreadCount(poolMax);

    SettingsStore* settings = new SettingsStore();
    auto syncUiFreezeWatchdog = [settings, uiFreezeWatchdog]() {
        if (UiFreezeWatchdog::shouldStartForSettings(settings->debugMenuEnabled())) {
            uiFreezeWatchdog->start();
        } else {
            uiFreezeWatchdog->stop();
        }
    };
    syncUiFreezeWatchdog();
    QObject::connect(settings, &SettingsStore::debugMenuEnabledChanged,
                     uiFreezeWatchdog, syncUiFreezeWatchdog);

    auto* performanceState = new PerformanceState(&app);
    performanceState->setReduceMotion(settings->reduceMotion());
    performanceState->setVisualEffectsQuality(settings->visualEffectsQuality());
    QObject::connect(settings, &SettingsStore::reduceMotionChanged,
                     performanceState, [settings, performanceState]() {
        performanceState->setReduceMotion(settings->reduceMotion());
    });
    QObject::connect(settings, &SettingsStore::visualEffectsQualityChanged,
                     performanceState, [settings, performanceState]() {
        performanceState->setVisualEffectsQuality(settings->visualEffectsQuality());
    });

    AppThemeManager* themeMgr = new AppThemeManager(&app);
    QObject::connect(settings, &SettingsStore::themeChanged, [themeMgr, settings]() {
        themeMgr->setCurrentTheme(settings->theme());
    });
    themeMgr->setCurrentTheme(settings->theme());
    TrayIconRenderer::setTrackColor(themeMgr->bgTrack());
    QObject::connect(themeMgr, &AppThemeManager::themeChanged, [themeMgr]() {
        TrayIconRenderer::setTrackColor(themeMgr->bgTrack());
    });

    UsageStore* usageStore = new UsageStore();
    usageStore->setSettingsStore(settings);
    usageStore->setPerformanceState(performanceState);

    // Initialize TokenAccountStore: load existing accounts or migrate from legacy config
    TokenAccountStore* tokenStore = TokenAccountStore::instance();
    {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("bootstrap.tokenAccounts"));
        tokenStore->loadFromDisk();
        tokenStore->migrateFromLegacy(settings);
        tokenStore->saveToDisk();
    }

    // Browser Session Bridge (Phase 2)
    auto* bridgeStore = new BrowserSessionBridgeStore(&app);
    auto* bridgeService = new BrowserSessionBridgeService(bridgeStore, &app);
    if (settings->browserSessionBridgeEnabled()) {
        bridgeService->start();
    }
    QObject::connect(settings, &SettingsStore::browserSessionBridgeEnabledChanged,
                     bridgeService, [settings, bridgeService]() {
        if (settings->browserSessionBridgeEnabled()) {
            bridgeService->start();
        } else {
            bridgeService->pause();
        }
    });
    usageStore->setBrowserSessionBridgeService(bridgeService);

    auto* bridgeViewModel = new BridgeViewModel(bridgeService, bridgeStore, &app);

    QObject::connect(usageStore, &UsageStore::codexAccountsChanged, [usageStore]() {
        // Refresh Codex data when accounts change
        usageStore->refreshProvider("codex");
    });
    QObject::connect(usageStore, &UsageStore::codexActiveAccountChanged, [usageStore](const QString&) {
        usageStore->refreshProvider("codex");
    });

    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "SettingsStore", settings);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "PerformanceState", performanceState);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "UsageStore", usageStore);
    registerAppThemeTypes(themeMgr);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "BridgeViewModel", bridgeViewModel);
    auto* settingsProvidersModel = new SettingsProvidersModel(usageStore, &app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "SettingsProvidersModel", settingsProvidersModel);
    auto* trayViewModel = new TrayViewModel(usageStore, &app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "TrayViewModel", trayViewModel);
    auto* usageDetailsViewModel = new UsageDetailsViewModel(usageStore, &app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "UsageDetailsViewModel", usageDetailsViewModel);
    auto* providerErrorClassifier = new ProviderErrorClassifier(&app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "ProviderErrorClassifier", providerErrorClassifier);

    auto* platformSettings = new PlatformSettings(&app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "PlatformSettings", platformSettings);

    AppController* appController = new AppController(&app);
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "AppController", appController);

    LanguageManager& langMgr = LanguageManager::instance();
    qmlRegisterSingletonInstance("CodexBarX", 1, 0, "LanguageManager", &langMgr);
    langMgr.setLanguage(settings->language());
    QObject::connect(settings, &SettingsStore::languageChanged, [&]() {
        langMgr.setLanguage(settings->language());
    });

    {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("bootstrap.providerState"));
        ProviderBootstrap::applyStoredProviderEnabledStates(settings, usageStore);
        ProviderBootstrap::syncEnabledProviderRuntimes();
    }
    QObject::connect(ProviderRuntimeManager::instance(),
                     &ProviderRuntimeManager::backgroundRefreshRequested,
                     usageStore,
                     [usageStore](const QString& providerId) {
                         usageStore->refreshProvider(providerId);
                     },
                     Qt::QueuedConnection);

    StatusItemController trayCtrl(usageStore, settings);
    if (!trayCtrl.initialize()) {
        qWarning() << "Failed to initialize system tray icon";
            QMessageBox::critical(nullptr,
                QStringLiteral("CodexBarX"),
            QCoreApplication::translate("App", "Failed to create system tray icon. Please restart the app."));
        return 1;
    }

    if (settings->refreshFrequency() > 0) {
        usageStore->startAutoRefresh(settings->refreshFrequency());
    }

    // Preload credentials on background thread to avoid blocking main thread on first refresh
    QTimer::singleShot(500, usageStore, [usageStore]() {
        usageStore->requestPreloadCredentials();
    });

    // Initial data refresh
    QTimer::singleShot(1500, usageStore, [usageStore]() {
        usageStore->refresh();
    });

    QTimer::singleShot(1500, usageStore, [usageStore]() {
        usageStore->refreshProviderStatuses();
    });
    QObject::connect(settings, &SettingsStore::refreshFrequencyChanged, usageStore, [settings, usageStore]() {
        if (settings->refreshFrequency() > 0) {
            usageStore->startAutoRefresh(settings->refreshFrequency());
        } else {
            usageStore->stopAutoRefresh();
        }
    });

    QQmlEngine qmlEngine;
    langMgr.install(&qmlEngine);
    installAppTheme(qmlEngine, themeMgr);

    auto glassTint = [settings, themeMgr]() {
        QColor tint = themeMgr->bgPrimary();
        tint.setAlpha(qBound(45, static_cast<int>(std::lround(settings->glassEffectOpacity() * 1.7)), 145));
        return tint;
    };
    auto applyGlassToView = [settings, themeMgr, glassTint](QQuickView& view) {
        view.setColor(WindowGlassEffect::clearColor(settings->glassEffectEnabled(),
                                                    themeMgr->bgPrimary()));
        WindowGlassEffect::apply(&view, settings->glassEffectEnabled(), glassTint());
    };
    auto prepareGlassView = [applyGlassToView](QQuickView& view) {
        WindowGlassEffect::prepare(&view);
        applyGlassToView(view);
    };

    QQuickView trayView(&qmlEngine, nullptr);
    trayView.setTitle("CodexBarX");
    trayView.resize(300, 520);
    trayView.setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    prepareGlassView(trayView);

    appController->trayView = &trayView;

    {
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("tray.setSource"));
        trayView.setSource(QUrl("qrc:/qml/TrayPanel.qml"));
    }

    auto positionPanel = [&]() -> QPoint {
        QRect rect = trayCtrl.trayIconRect();
        QPoint iconCenter(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);

        // Use Win32 MonitorFromPoint to find the correct monitor — Qt may not
        // detect all monitors (e.g. only sees secondary on some dual-screen setups).
        QRect avail = win32MonitorWorkArea(iconCenter);
        QRect monGeo = win32MonitorGeometry(iconCenter);
        debugLog(QString("[positionPanel] trayIconRect=(%1 %2 %3 %4) iconCenter=(%5 %6)")
            .arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height())
            .arg(iconCenter.x()).arg(iconCenter.y()));
        debugLog(QString("[positionPanel] win32 workArea=(%1 %2 %3 %4) monGeo=(%5 %6 %7 %8)")
            .arg(avail.x()).arg(avail.y()).arg(avail.width()).arg(avail.height())
            .arg(monGeo.x()).arg(monGeo.y()).arg(monGeo.width()).arg(monGeo.height()));
        // Also log Qt's view for comparison
        debugLog(QString("[positionPanel] Qt screens: %1, primaryScreen=%2")
            .arg(QGuiApplication::screens().size())
            .arg(QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->name() : "null"));
        for (auto* s : QGuiApplication::screens()) {
            debugLog(QString("  Qt screen %1 geo=(%2 %3 %4 %5) avail=(%6 %7 %8 %9) dpr=%10")
                .arg(s->name())
                .arg(s->geometry().x()).arg(s->geometry().y()).arg(s->geometry().width()).arg(s->geometry().height())
                .arg(s->availableGeometry().x()).arg(s->availableGeometry().y()).arg(s->availableGeometry().width()).arg(s->availableGeometry().height())
                .arg(s->devicePixelRatio()));
        }

        if (avail.isEmpty()) {
            // Fallback to Qt if Win32 fails
            QScreen* screen = QGuiApplication::screenAt(iconCenter);
            if (!screen) screen = QGuiApplication::primaryScreen();
            if (!screen) return {};
            avail = screen->availableGeometry();
        }

        int pw = trayView.width();
        int ph = trayView.height();
        if (pw <= 0) pw = 300;
        if (ph <= 0) ph = 420;
        int x = rect.x() + rect.width() / 2 - pw / 2;
        int y = avail.y() + avail.height() - ph - 4;
        x = qBound(avail.x(), x, avail.x() + avail.width() - pw);
        y = qMax(avail.y(), y);
        trayView.setPosition(x, y);
        debugLog(QString("[positionPanel] result=(%1 %2)").arg(x).arg(y));
        return QPoint(x, y);
    };

    auto showPanel = [&, settings]() {
        if (trayView.status() != QQuickView::Ready) return;
        QPoint pos = positionPanel();
        debugLog(QString("[showPanel] BEFORE show() trayView pos=(%1 %2)").arg(trayView.x()).arg(trayView.y()));
        trayView.show();
        debugLog(QString("[showPanel] AFTER show() trayView pos=(%1 %2) screen=%3")
            .arg(trayView.x()).arg(trayView.y())
            .arg(trayView.screen() ? trayView.screen()->name() : "null"));
        applyGlassToView(trayView);
        updateTrayPanelWindowShape(&trayView, settings->glassEffectEnabled());
        if (!pos.isNull()) forceWindowPosition(&trayView, pos.x(), pos.y());
        debugLog(QString("[showPanel] AFTER forceWindowPosition() trayView pos=(%1 %2)").arg(trayView.x()).arg(trayView.y()));
        trayView.raise();
        trayView.requestActivate();
        QTimer::singleShot(100, &trayView, [&trayView]() {
            debugLog(QString("[showPanel] DEFERRED(100ms) trayView pos=(%1 %2) screen=%3")
                .arg(trayView.x()).arg(trayView.y())
                .arg(trayView.screen() ? trayView.screen()->name() : "null"));
#ifdef Q_OS_WIN
            HWND hwnd = reinterpret_cast<HWND>(trayView.winId());
            if (hwnd) {
                RECT r;
                GetWindowRect(hwnd, &r);
                debugLog(QString("[showPanel] DEFERRED(100ms) Win32 rect=(%1 %2 %3 %4)")
                    .arg(r.left).arg(r.top).arg(r.right).arg(r.bottom));
            }
#endif
        });
    };

    bool startupPanelShown = false;
    auto showStartupPanel = [&]() {
        if (startupPanelShown || trayView.status() != QQuickView::Ready) return;
        startupPanelShown = true;
        showPanel();
    };

    QObject::connect(&trayView, &QQuickView::statusChanged, &trayView, [&](QQuickView::Status status) {
        if (status == QQuickView::Error) {
            QStringList messages;
            for (const QQmlError& error : trayView.errors()) {
                messages.append(error.toString());
            }
            const QString detail = messages.isEmpty()
                ? QCoreApplication::translate("App", "Unknown QML loading error.")
                : messages.join(QLatin1Char('\n'));
            qWarning() << "Failed to load tray panel:" << detail;
            QMessageBox::critical(nullptr, QStringLiteral("CodexBarX"), detail);
        } else if (status == QQuickView::Ready) {
            showStartupPanel();
        }
    });

    QObject::connect(&trayCtrl, &StatusItemController::trayPanelRequested, [&]() {
        if (trayView.isVisible()) {
            trayView.hide();
            return;
        }
        showPanel();
    });

    QObject::connect(&trayView, &QQuickView::activeChanged, [&]() {
        if (!trayView.isActive() && trayView.isVisible()) {
            trayView.hide();
        }
    });
    QObject::connect(&trayView, &QWindow::visibleChanged,
                     performanceState, [performanceState](bool visible) {
        performanceState->setTrayVisible(visible);
    });
    QObject::connect(&trayView, &QWindow::visibleChanged, &trayView, [&trayView, applyGlassToView, settings](bool visible) {
        if (visible) {
            applyGlassToView(trayView);
            updateTrayPanelWindowShape(&trayView, settings->glassEffectEnabled());
        }
    });

    QTimer::singleShot(0, &trayView, showStartupPanel);

    // Returns the Win32 work area of the monitor containing the tray icon.
    // Falls back to Qt's screen detection if Win32 fails.
    auto win32WorkAreaForTray = [&]() -> QRect {
        QRect iconRect = trayCtrl.trayIconRect();
        QPoint iconCenter(iconRect.x() + iconRect.width() / 2, iconRect.y() + iconRect.height() / 2);
        QRect wa = win32MonitorWorkArea(iconCenter);
        if (!wa.isEmpty()) return wa;
        QScreen* screen = QGuiApplication::screenAt(iconCenter);
        if (!screen) screen = QGuiApplication::primaryScreen();
        if (screen) return screen->availableGeometry();
        return QRect();
    };

    auto screenForTray = [&]() -> QScreen* {
        if (trayView.screen()) {
            debugLog(QString("[screenForTray] from trayView.screen() = %1").arg(trayView.screen()->name()));
            return trayView.screen();
        }
        QRect iconRect = trayCtrl.trayIconRect();
        QPoint iconCenter(iconRect.x() + iconRect.width() / 2, iconRect.y() + iconRect.height() / 2);
        if (QScreen* s = QGuiApplication::screenAt(iconCenter)) {
            debugLog(QString("[screenForTray] from screenAt(%1,%2) = %3").arg(iconCenter.x()).arg(iconCenter.y()).arg(s->name()));
            return s;
        }
        debugLog(QString("[screenForTray] fallback to primaryScreen() = %1").arg(QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->name() : "null"));
        return QGuiApplication::primaryScreen();
    };

    appController->screenForTray = screenForTray;
    appController->win32WorkAreaForTray = win32WorkAreaForTray;

    QQuickView settingsView(&qmlEngine, nullptr);
    settingsView.setTitle(QStringLiteral(" "));
    settingsView.setMinimumSize(QSize(820, 560));
    settingsView.setResizeMode(QQuickView::SizeRootObjectToView);
    settingsView.setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    prepareGlassView(settingsView);

    // Set height to fit screen and center position
    {
        QRect avail = win32WorkAreaForTray();
        if (avail.isEmpty()) {
            QScreen* s = screenForTray();
            if (s) avail = s->availableGeometry();
        }
        if (!avail.isEmpty()) {
            int maxH = avail.height() - 60;
            int h = qMin(960, maxH);
            int w = 900;
            settingsView.setPosition((avail.width() - w) / 2 + avail.x(),
                                      (avail.height() - h) / 2 + avail.y());
            settingsView.resize(w, h);
        } else {
            settingsView.resize(900, 800);
        }
    }

    QObject::connect(&settingsView, &QQuickView::statusChanged, &settingsView, [&](QQuickView::Status status) {
        if (status != QQuickView::Error) return;
        QStringList messages;
        for (const QQmlError& error : settingsView.errors()) {
            messages.append(error.toString());
        }
        const QString detail = messages.isEmpty()
            ? QCoreApplication::translate("App", "Unknown QML loading error.")
            : messages.join(QLatin1Char('\n'));
        qWarning() << "Failed to load settings window:" << detail;
    });

    appController->settingsView = &settingsView;
    bool settingsSourceLoaded = false;
    appController->ensureSettingsLoaded = [&settingsView, &settingsSourceLoaded]() {
        if (settingsSourceLoaded) return;
        UiFreezeWatchdog::PhaseScope phase(QStringLiteral("settings.setSource"));
        settingsSourceLoaded = true;
        settingsView.setSource(QUrl("qrc:/qml/SettingsWindow.qml"));
    };
    QObject::connect(&settingsView, &QWindow::windowStateChanged, appController,
                     [appController]() {
                         emit appController->settingsMaximizedChanged();
                     });
    QObject::connect(&settingsView, &QWindow::visibleChanged, &settingsView,
                     [&settingsView, applyGlassToView](bool visible) {
                         if (visible) applyGlassToView(settingsView);
                     });
    QObject::connect(&settingsView, &QWindow::visibleChanged,
                     performanceState, [performanceState](bool visible) {
        performanceState->setSettingsVisible(visible);
    });

    QQuickView usageView(&qmlEngine, nullptr);
    usageView.setTitle(QStringLiteral(" "));
    usageView.resize(800, 600);
    usageView.setMinimumSize(QSize(720, 480));
    usageView.setResizeMode(QQuickView::SizeRootObjectToView);
    usageView.setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    prepareGlassView(usageView);

    QObject::connect(&usageView, &QQuickView::statusChanged, &usageView, [&usageView](QQuickView::Status status) {
        if (status != QQuickView::Error) return;
        QStringList messages;
        for (const QQmlError& error : usageView.errors()) {
            messages.append(error.toString());
        }
        const QString detail = messages.isEmpty()
            ? QCoreApplication::translate("App", "Unknown QML loading error.")
            : messages.join(QLatin1Char('\n'));
        qWarning() << "Failed to load usage window:" << detail;
    });
    appController->usageView = &usageView;
    QObject::connect(&usageView, &QWindow::windowStateChanged, appController,
                     [appController]() {
                         emit appController->usageVisibleChanged();
                     });
    QObject::connect(&usageView, &QWindow::visibleChanged, appController,
                     [appController]() {
                         emit appController->usageVisibleChanged();
                     });
    QObject::connect(&usageView, &QWindow::visibleChanged,
                     performanceState, [performanceState](bool visible) {
        performanceState->setUsageVisible(visible);
    });
    QObject::connect(&usageView, &QWindow::visibleChanged, &usageView,
                     [&usageView, applyGlassToView](bool visible) {
                         if (visible) applyGlassToView(usageView);
                     });

    QObject::connect(&settingsView, &QWindow::windowStateChanged, appController,
                     [appController]() {
                         emit appController->settingsMaximizedChanged();
                     });

    auto applyGlassToAllViews = [&]() {
        applyGlassToView(trayView);
        applyGlassToView(settingsView);
        applyGlassToView(usageView);
        updateTrayPanelWindowShape(&trayView, settings->glassEffectEnabled());
    };
    QObject::connect(themeMgr, &AppThemeManager::themeChanged, applyGlassToAllViews);
    QObject::connect(settings, &SettingsStore::glassEffectEnabledChanged, applyGlassToAllViews);
    QObject::connect(settings, &SettingsStore::glassEffectOpacityChanged, applyGlassToAllViews);

    if (showSettingsOnStartup) {
        QTimer::singleShot(0, appController, &AppController::openSettings);
        if (!g_logPath.isEmpty()) {
            QTimer::singleShot(1000, &settingsView, [&settingsView]() {
                qWarning() << "Settings view status=" << settingsView.status()
                           << "size=" << settingsView.size()
                           << "root=" << settingsView.rootObject();
                dumpQuickItemTree(settingsView.rootObject());
            });
        }
    }

    QObject::connect(&trayCtrl, &StatusItemController::settingsRequested, appController,
                     &AppController::toggleSettings);

    auto showAboutDialog = [&app]() {
        const QString body = QString("%1\n\n%2\n\n%3\n%4")
            .arg(QCoreApplication::translate("App", "CodexBarX v0.1.0"),
                 QCoreApplication::translate(
                     "App",
                     "Menu bar and tray app for tracking AI provider usage limits."),
                 QCoreApplication::translate("App", "Built with Qt 6.5 + QML"),
                 QStringLiteral("github.com/basil520/CodexBarX"));
        QMessageBox::about(nullptr,
                QCoreApplication::translate("App", "About CodexBarX"),
            body);
    };

    QObject::connect(&trayCtrl, &StatusItemController::aboutRequested, &app, showAboutDialog);
    QObject::connect(appController, &AppController::aboutRequested, &app, showAboutDialog);

    QObject::connect(&SessionQuotaNotifier::instance(), &SessionQuotaNotifier::notificationRequested,
                     &trayCtrl, [&trayCtrl](const QString& title, const QString& body) {
        trayCtrl.showBalloon(title, body);
    });

    // Fast quit path: immediately hide all UI, destroy tray icon, and force exit.
    // Do NOT go through QCoreApplication::quit() → app.exec() return, because Qt's
    // event-loop shutdown can be blocked by background threads for 20+ seconds.
    auto forceQuit = [&app, &trayView, &settingsView, &usageView, &trayCtrl, usageStore, bridgeService]() {
        // Set shuttingDown flags so background threads can check and exit early
        if (bridgeService) bridgeService->stop();
        usageStore->shutdown();
        trayView.hide();
        settingsView.hide();
        usageView.hide();
        trayCtrl.destroyTrayIcon();
#ifdef Q_OS_WIN
        ExitProcess(0);
#elif defined(Q_OS_MACOS)
        QTimer::singleShot(5000, []() { _exit(0); });
        app.quit();
#else
        _exit(0);
#endif
    };
    QObject::connect(&trayCtrl, &StatusItemController::quitRequested, &app, forceQuit);
    QObject::connect(appController, &AppController::forceQuitRequested, &app, forceQuit);

    int exitCode = app.exec();

#ifdef QT_DEBUG
    // Exit timing diagnostics (debug builds only)
    QString diagPath = QDir::tempPath() + "/CodexBarX_ExitDiag.log";
    auto diagLog = [&](const QString& msg) {
        QFile f(diagPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            f.write(QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toUtf8());
            f.write(" ");
            f.write(msg.toUtf8());
            f.write("\n");
        }
    };
    diagLog("[ExitDiag] app.exec() returned (non-fast-quit path)");
#endif

    // Non-fast-quit fallback: brief wait then force exit.
    // In normal operation, forceQuit lambda handles exit via ExitProcess(0).
    // This path is only reached if app.exec() returns naturally (e.g. last window closed).
    if (bridgeService) {
        bridgeService->stop();
    }
    if (usageStore) {
        usageStore->shutdown();
    }
    QThreadPool::globalInstance()->waitForDone(50);

#ifdef Q_OS_WIN
    ExitProcess(static_cast<UINT>(exitCode));
#elif defined(Q_OS_MACOS)
    return exitCode;
#else
    _exit(exitCode);
#endif
    return exitCode; // unreachable
}

#include "main.moc"

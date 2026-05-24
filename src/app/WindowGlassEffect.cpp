#include "WindowGlassEffect.h"

#include <QSurfaceFormat>
#include <QWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

namespace WindowGlassEffect {

void prepare(QWindow* window)
{
    if (!window) return;

    QSurfaceFormat format = window->format();
    if (format.alphaBufferSize() >= 8) return;

    format.setAlphaBufferSize(8);
    window->setFormat(format);
}

QColor clearColor(bool enabled, const QColor& fallback)
{
    if (!enabled || !isAvailable()) {
        return fallback;
    }
    return QColor(0, 0, 0, 0);
}

#ifdef Q_OS_WIN

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_DONOTROUND
#define DWMWCP_DONOTROUND 1
#endif

namespace {

enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};

struct AccentPolicy {
    int accentState = ACCENT_DISABLED;
    int accentFlags = 0;
    int gradientColor = 0;
    int animationId = 0;
};

struct WindowCompositionAttributeData {
    int attribute = 0;
    void* data = nullptr;
    size_t sizeOfData = 0;
};

using SetWindowCompositionAttributeFunc = BOOL(WINAPI*)(HWND, WindowCompositionAttributeData*);
using DwmSetWindowAttributeFunc = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);

constexpr int WCA_ACCENT_POLICY = 19;
constexpr DWORD DWMWA_SYSTEMBACKDROP_TYPE_FALLBACK = 38;
constexpr int DWMSBT_NONE = 1;
constexpr int DWMSBT_TRANSIENTWINDOW = 3;

HMODULE moduleHandle(const wchar_t* name)
{
    HMODULE module = GetModuleHandleW(name);
    if (!module) {
        module = LoadLibraryW(name);
    }
    return module;
}

SetWindowCompositionAttributeFunc setWindowCompositionAttribute()
{
    static auto* fn = []() -> SetWindowCompositionAttributeFunc {
        HMODULE user32 = moduleHandle(L"user32.dll");
        if (!user32) return nullptr;
        return reinterpret_cast<SetWindowCompositionAttributeFunc>(
            GetProcAddress(user32, "SetWindowCompositionAttribute"));
    }();
    return fn;
}

DwmSetWindowAttributeFunc dwmSetWindowAttribute()
{
    static auto* fn = []() -> DwmSetWindowAttributeFunc {
        HMODULE dwmapi = moduleHandle(L"dwmapi.dll");
        if (!dwmapi) return nullptr;
        return reinterpret_cast<DwmSetWindowAttributeFunc>(
            GetProcAddress(dwmapi, "DwmSetWindowAttribute"));
    }();
    return fn;
}

int tintToAbgr(QColor tint)
{
    if (!tint.isValid()) {
        tint = QColor(20, 24, 38);
    }
    if (tint.alpha() > 220) {
        tint.setAlpha(150);
    }

    return ((tint.alpha() & 0xff) << 24)
        | ((tint.blue() & 0xff) << 16)
        | ((tint.green() & 0xff) << 8)
        | (tint.red() & 0xff);
}

bool applyAccentPolicy(HWND hwnd, bool enabled, const QColor& tint)
{
    auto* fn = setWindowCompositionAttribute();
    if (!fn) return false;

    AccentPolicy policy;
    policy.accentState = enabled ? ACCENT_ENABLE_ACRYLICBLURBEHIND : ACCENT_DISABLED;
    policy.gradientColor = enabled ? tintToAbgr(tint) : 0;

    WindowCompositionAttributeData data;
    data.attribute = WCA_ACCENT_POLICY;
    data.data = &policy;
    data.sizeOfData = sizeof(policy);

    return fn(hwnd, &data) == TRUE;
}

bool applySystemBackdrop(HWND hwnd, bool enabled)
{
    auto* fn = dwmSetWindowAttribute();
    if (!fn) return false;

    const int backdrop = enabled ? DWMSBT_TRANSIENTWINDOW : DWMSBT_NONE;
    return SUCCEEDED(fn(hwnd,
                        DWMWA_SYSTEMBACKDROP_TYPE_FALLBACK,
                        &backdrop,
                        sizeof(backdrop)));
}

bool extendFrameIntoClientArea(HWND hwnd, bool enabled)
{
    MARGINS margins = enabled
        ? MARGINS{-1, -1, -1, -1}
        : MARGINS{0, 0, 0, 0};
    return SUCCEEDED(DwmExtendFrameIntoClientArea(hwnd, &margins));
}

bool enableBlurBehindWindow(HWND hwnd, bool enabled)
{
    DWM_BLURBEHIND blur = {};
    blur.dwFlags = DWM_BB_ENABLE;
    blur.fEnable = enabled ? TRUE : FALSE;
    return SUCCEEDED(DwmEnableBlurBehindWindow(hwnd, &blur));
}

} // namespace

bool isAvailable()
{
    return setWindowCompositionAttribute() != nullptr || dwmSetWindowAttribute() != nullptr;
}

bool apply(QWindow* window, bool enabled, QColor tint)
{
    if (!window) return false;

    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd) return false;

    // Force square corners (no rounded corners) on Windows 11 to keep it strictly frameless and square
    auto* fnDwm = dwmSetWindowAttribute();
    if (fnDwm) {
        const int preference = DWMWCP_DONOTROUND;
        fnDwm(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
    }

    // Always strip WS_CAPTION and WS_SYSMENU to ensure it is completely frameless and has no native system titlebar
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    if ((style & WS_CAPTION) || (style & WS_SYSMENU)) {
        style &= ~(WS_CAPTION | WS_SYSMENU);
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    if (!enabled) {
        const bool blurCleared = enableBlurBehindWindow(hwnd, false);
        const bool frameCleared = extendFrameIntoClientArea(hwnd, false);
        const bool accentCleared = applyAccentPolicy(hwnd, false, tint);
        const bool backdropCleared = applySystemBackdrop(hwnd, false);
        return blurCleared || frameCleared || accentCleared || backdropCleared;
    }

    const bool frameExtended = extendFrameIntoClientArea(hwnd, true);
    const bool blurEnabled = enableBlurBehindWindow(hwnd, true);
    const bool backdropApplied = applySystemBackdrop(hwnd, true);
    const bool accentApplied = applyAccentPolicy(hwnd, true, tint);

    return frameExtended || blurEnabled || backdropApplied || accentApplied;
}

#else

bool isAvailable()
{
    return false;
}

bool apply(QWindow* window, bool enabled, QColor tint)
{
    Q_UNUSED(window);
    Q_UNUSED(enabled);
    Q_UNUSED(tint);
    return false;
}

#endif

} // namespace WindowGlassEffect

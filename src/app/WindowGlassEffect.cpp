#include "WindowGlassEffect.h"

#include <QSurfaceFormat>
#include <QWindow>

#ifdef Q_OS_WIN
#include <windows.h>
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

    if (!enabled) {
        const bool accentCleared = applyAccentPolicy(hwnd, false, tint);
        const bool backdropCleared = applySystemBackdrop(hwnd, false);
        return accentCleared || backdropCleared;
    }

    if (applyAccentPolicy(hwnd, true, tint)) {
        return true;
    }
    return applySystemBackdrop(hwnd, true);
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

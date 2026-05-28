#include "MacOSWindowGlassEffect.h"

#include <QWindow>

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

namespace {

char kCodexBarXVisualEffectViewKey;

NSView* contentViewForWindow(QWindow* window)
{
    if (!window) return nil;

    NSView* qtView = reinterpret_cast<NSView*>(window->winId());
    if (![qtView isKindOfClass:[NSView class]]) return nil;

    NSWindow* nsWindow = [qtView window];
    return nsWindow ? [nsWindow contentView] : qtView;
}

NSWindow* nativeWindowForContentView(NSView* contentView)
{
    return contentView ? [contentView window] : nil;
}

NSVisualEffectView* associatedEffectView(NSView* contentView)
{
    return contentView
        ? (__bridge NSVisualEffectView*)objc_getAssociatedObject(contentView, &kCodexBarXVisualEffectViewKey)
        : nil;
}

void setAssociatedEffectView(NSView* contentView, NSVisualEffectView* effectView)
{
    if (!contentView) return;
    objc_setAssociatedObject(contentView,
                             &kCodexBarXVisualEffectViewKey,
                             effectView,
                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

} // namespace

bool macOSGlassEffectIsAvailable()
{
    return true;
}

bool applyMacOSGlassEffect(QWindow* window, bool enabled, QColor tint)
{
    Q_UNUSED(tint)

    NSView* contentView = contentViewForWindow(window);
    if (!contentView) return false;

    NSVisualEffectView* effectView = associatedEffectView(contentView);

    if (!enabled) {
        if (effectView) {
            [effectView removeFromSuperview];
            setAssociatedEffectView(contentView, nil);
        }

        if (NSWindow* nsWindow = nativeWindowForContentView(contentView)) {
            [nsWindow setOpaque:YES];
        }
        return true;
    }

    if (!effectView) {
        effectView = [[NSVisualEffectView alloc] initWithFrame:[contentView bounds]];
        [effectView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [effectView setBlendingMode:NSVisualEffectBlendingModeBehindWindow];
        [effectView setMaterial:NSVisualEffectMaterialHUDWindow];
        [effectView setState:NSVisualEffectStateActive];
        [contentView addSubview:effectView positioned:NSWindowBelow relativeTo:nil];
        setAssociatedEffectView(contentView, effectView);
#if !__has_feature(objc_arc)
        [effectView release];
#endif
    } else {
        [effectView setFrame:[contentView bounds]];
        [effectView setHidden:NO];
    }

    if (NSWindow* nsWindow = nativeWindowForContentView(contentView)) {
        [nsWindow setOpaque:NO];
        [nsWindow setBackgroundColor:[NSColor clearColor]];
    }

    return true;
}

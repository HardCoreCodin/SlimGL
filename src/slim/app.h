#pragma once

#include "./core/base.h"
#include "./gl/glad.h"

struct SlimApp {
    timers::Timer update_timer, render_timer;
    bool is_running{true};
    bool is_minimized{false};
    bool blit{false};
    bool suspend_when_minimized{true};

    virtual void OnInit() {};
    virtual void OnShutdown() {};
    virtual void OnWindowMinimized() {};
    virtual void OnWindowRestored() {};
    virtual void OnWindowResize(u16 width, u16 height) {};
    virtual void OnKeyChanged(  u8 key, bool pressed) {};
    virtual void OnMouseButtonUp(  mouse::Button &mouse_button) {};
    virtual void OnMouseButtonDown(mouse::Button &mouse_button) {};
    virtual void OnMouseButtonDoubleClicked(mouse::Button &mouse_button) {};
    virtual void OnMouseWheelScrolled(f32 amount) {};
    virtual void OnMousePositionSet(i32 x, i32 y) {};
    virtual void OnMouseMovementSet(i32 x, i32 y) {};
    virtual void OnMouseRawMovementSet(i32 x, i32 y) {};
    virtual void OnRender() {};
    virtual void OnUpdate(f32 delta_time) {};
    virtual void OnWindowRedraw() {
        render_timer.beginFrame();
        OnRender();
        render_timer.endFrame();
    };

    INLINE void _minimize() {
        is_minimized = true;
        OnWindowMinimized();
    }

    INLINE void _restore() {
        is_minimized = false;
        OnWindowRestored();
    }

    INLINE void _resize(u16 width, u16 height) {
        window::width = width;
        window::height = height;
        glViewport(0, 0, width, height);
        OnWindowResize(width, height);
        OnWindowRedraw();
    }

    INLINE void _redraw() {
        if (suspend_when_minimized && is_minimized)
            return;

        OnWindowRedraw();
        mouse::resetChanges();
    }

    INLINE void _update() {
        if (suspend_when_minimized && is_minimized)
            return;

        update_timer.beginFrame();
        OnUpdate(update_timer.delta_time);
        update_timer.endFrame();
        mouse::resetChanges();
    }

    INLINE void _init() {
        glEnable(GL_DEPTH_TEST);
        OnInit();
    }
    INLINE void _mouseButtonUp(  mouse::Button &mouse_button) { OnMouseButtonUp(mouse_button); };
    INLINE void _mouseButtonDown(mouse::Button &mouse_button) { OnMouseButtonDown(mouse_button); };
    INLINE void _mouseButtonDoubleClicked(mouse::Button &mouse_button) { OnMouseButtonDoubleClicked(mouse_button); };
    INLINE void _mouseWheelScrolled(f32 amount) { OnMouseWheelScrolled(amount); };
    INLINE void _mousePositionSet(i32 x, i32 y) { OnMousePositionSet(x, y); };
    INLINE void _mouseMovementSet(i32 x, i32 y) { OnMouseMovementSet(x, y); };
    INLINE void _mouseRawMovementSet(i32 x, i32 y) { OnMouseRawMovementSet(x, y); };
    INLINE void _shutdown() {
        OnShutdown();

    }
    INLINE void _keyChanged(u8 key, bool pressed) { OnKeyChanged(key, pressed); }
};

SlimApp* createApp();

#include "./platforms/win32.h"
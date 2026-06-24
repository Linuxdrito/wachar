#include "wayland.hpp"
#include <iostream>
#include <cstring>
#include <linux/input-event-codes.h>

WaylandContext::WaylandContext() {}

WaylandContext::~WaylandContext() {
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    if (xdg_top) xdg_toplevel_destroy(xdg_top);
    if (xdg_surf) xdg_surface_destroy(xdg_surf);
    if (surface) wl_surface_destroy(surface);
    if (xdg_wm_base_ptr) xdg_wm_base_destroy(xdg_wm_base_ptr);
    if (compositor) wl_compositor_destroy(compositor);
    if (registry) wl_registry_destroy(registry);
    if (display) wl_display_disconnect(display);
}

void WaylandContext::registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    auto* ctx = static_cast<WaylandContext*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        ctx->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        ctx->xdg_wm_base_ptr = static_cast<xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        ctx->seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        static const wl_seat_listener seat_listener = { seat_capabilities, nullptr };
        wl_seat_add_listener(ctx->seat, &seat_listener, ctx);
    }
}

void WaylandContext::registry_remover(void*, wl_registry*, uint32_t) {}

void WaylandContext::xdg_wm_base_ping(void*, xdg_wm_base* xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void WaylandContext::xdg_surface_configure(void* data, xdg_surface* xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);
    static_cast<WaylandContext*>(data)->needs_redraw = true;
}

void WaylandContext::xdg_toplevel_configure(void* data, xdg_toplevel*, int32_t width, int32_t height, wl_array*) {
    auto* ctx = static_cast<WaylandContext*>(data);
    if (width > 0 && height > 0) {
        ctx->width = width;
        ctx->height = height;
    }
}

void WaylandContext::xdg_toplevel_close(void* data, xdg_toplevel*) {
    static_cast<WaylandContext*>(data)->stop();
}

void WaylandContext::keyboard_key(void* data, wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
    auto* ctx = static_cast<WaylandContext*>(data);
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        // En Wayland/evdev: ESC es 1, 'q' es 16
        if (key == KEY_ESC || key == KEY_Q) {
            ctx->stop();
        }
    }
}

static void keyboard_keymap(void*, wl_keyboard*, uint32_t, int32_t, uint32_t) {}
static void keyboard_enter(void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*) {}
static void keyboard_leave(void*, wl_keyboard*, uint32_t, wl_surface*) {}
static void keyboard_modifiers(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) {}
static void keyboard_repeat_info(void*, wl_keyboard*, int32_t, int32_t) {}

void WaylandContext::seat_capabilities(void* data, wl_seat* seat, uint32_t capabilities) {
    auto* ctx = static_cast<WaylandContext*>(data);
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        ctx->keyboard = wl_seat_get_keyboard(seat);
        static const wl_keyboard_listener kbd_listener = { keyboard_keymap, keyboard_enter, keyboard_leave, keyboard_key, keyboard_modifiers, keyboard_repeat_info };
        wl_keyboard_add_listener(ctx->keyboard, &kbd_listener, ctx);
    }
}

bool WaylandContext::init(int initial_width, int initial_height, const std::string& title) {
    width = initial_width;
    height = initial_height;

    display = wl_display_connect(nullptr);
    if (!display) return false;

    registry = wl_display_get_registry(display);
    static const wl_registry_listener registry_listener = { registry_handler, registry_remover };
    wl_registry_add_listener(registry, &registry_listener, this);
    wl_display_roundtrip(display);

    if (!compositor || !xdg_wm_base_ptr) return false;

    static const xdg_wm_base_listener wm_base_listener = { xdg_wm_base_ping };
    xdg_wm_base_add_listener(xdg_wm_base_ptr, &wm_base_listener, this);

    surface = wl_compositor_create_surface(compositor);
    xdg_surf = xdg_wm_base_get_xdg_surface(xdg_wm_base_ptr, surface);
    
    static const xdg_surface_listener surface_listener = { xdg_surface_configure };
    xdg_surface_add_listener(xdg_surf, &surface_listener, this);

    xdg_top = xdg_surface_get_toplevel(xdg_surf);
    static const xdg_toplevel_listener toplevel_listener = { xdg_toplevel_configure, xdg_toplevel_close };
    xdg_toplevel_add_listener(xdg_top, &toplevel_listener, this);
    
    xdg_toplevel_set_title(xdg_top, title.c_str());
    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    return true;
}

bool WaylandContext::dispatch() {
    return wl_display_dispatch(display) != -1;
}

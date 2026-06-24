#pragma once
#include <wayland-client.h>
#include "xdg-shell-protocol.h"
#include <string>

class WaylandContext {
public:
    WaylandContext();
    ~WaylandContext();

    bool init(int initial_width, int initial_height, const std::string& title);
    bool dispatch();
    
    wl_display* get_display() const { return display; }
    wl_surface* get_surface() const { return surface; }
    
    bool is_running() const { return running; }
    void stop() { running = false; }
    
    bool needs_redraw = true;
    int width = 0;
    int height = 0;

private:
    wl_display* display = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_surface* surface = nullptr;
    xdg_wm_base* xdg_wm_base_ptr = nullptr;
    xdg_surface* xdg_surf = nullptr;
    xdg_toplevel* xdg_top = nullptr;
    wl_seat* seat = nullptr;
    wl_keyboard* keyboard = nullptr;

    bool running = true;

    // Callbacks de Wayland
    static void registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
    static void registry_remover(void* data, wl_registry* registry, uint32_t id);
    static void xdg_wm_base_ping(void* data, xdg_wm_base* xdg_wm_base, uint32_t serial);
    static void xdg_surface_configure(void* data, xdg_surface* xdg_surface, uint32_t serial);
    static void xdg_toplevel_configure(void* data, xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, wl_array* states);
    static void xdg_toplevel_close(void* data, xdg_toplevel* xdg_toplevel);
    static void seat_capabilities(void* data, wl_seat* seat, uint32_t capabilities);
    static void keyboard_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
};

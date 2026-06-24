#include <iostream>
#include "wayland.hpp"
#include "renderer.hpp"
#include "image.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Uso: wachar <imagen>\n"; // <-- Cambiado aquí
        return 1;
    }

    // 1. Cargar la imagen usando stb_image
    Image img = load_image(argv[1]);

    // 2. Inicializar Wayland
    WaylandContext wl_ctx;
    if (!wl_ctx.init(img.width, img.height, "wachar")) { // <-- Cambiado aquí
        std::cerr << "Error: No se pudo inicializar Wayland.\n";
        return 1;
    }

    // 3. Inicializar EGL/GLES y el Renderizador
    Renderer renderer;
    renderer.init(wl_ctx.get_display(), wl_ctx.get_surface(), wl_ctx.width, wl_ctx.height);
    renderer.setup_image(img);

    // 4. Bucle principal
    while (wl_ctx.is_running() && wl_ctx.dispatch()) {
        if (wl_ctx.needs_redraw) {
            renderer.resize(wl_ctx.width, wl_ctx.height);
            renderer.render();
            wl_ctx.needs_redraw = false;
        }
    }

    return 0;
}

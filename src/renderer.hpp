#pragma once
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "image.hpp"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init(wl_display* display, wl_surface* surface, int width, int height);
    void setup_image(const Image& img);
    void resize(int width, int height);
    void render();

private:
    wl_egl_window* egl_window = nullptr;
    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLContext egl_context = EGL_NO_CONTEXT;
    EGLSurface egl_surface = EGL_NO_SURFACE;
    
    GLuint program;
    GLuint texture;
    GLint scale_loc;
    
    int win_w = 0;
    int win_h = 0;
    int img_w = 0;
    int img_h = 0;

    GLuint compile_shader(GLenum type, const char* source);
};

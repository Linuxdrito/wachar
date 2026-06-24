#include "renderer.hpp"
#include <iostream>

const char* vshader_src = R"(
    attribute vec2 position;
    attribute vec2 texcoord;
    varying vec2 v_texcoord;
    uniform vec2 scale;
    void main() {
        gl_Position = vec4(position * scale, 0.0, 1.0);
        v_texcoord = texcoord;
    }
)";

const char* fshader_src = R"(
    precision mediump float;
    varying vec2 v_texcoord;
    uniform sampler2D tex;
    void main() {
        gl_FragColor = texture2D(tex, v_texcoord);
    }
)";

Renderer::Renderer() {}

Renderer::~Renderer() {
    if (egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(egl_display, egl_surface);
        eglDestroyContext(egl_display, egl_context);
        eglTerminate(egl_display);
    }
    if (egl_window) wl_egl_window_destroy(egl_window);
}

GLuint Renderer::compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void Renderer::init(wl_display* display, wl_surface* surface, int width, int height) {
    win_w = width;
    win_h = height;

    egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    eglInitialize(egl_display, nullptr, nullptr);

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_config;
    eglChooseConfig(egl_display, config_attribs, &config, 1, &num_config);

    const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    egl_context = eglCreateContext(egl_display, config, EGL_NO_CONTEXT, context_attribs);

    egl_window = wl_egl_window_create(surface, width, height);
    egl_surface = eglCreateWindowSurface(egl_display, config, (EGLNativeWindowType)egl_window, nullptr);
    
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vshader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fshader_src);
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);

    scale_loc = glGetUniformLocation(program, "scale");
}

void Renderer::setup_image(const Image& img) {
    img_w = img.width;
    img_h = img.height;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
}

void Renderer::resize(int width, int height) {
    win_w = width;
    win_h = height;
    if (egl_window) {
        wl_egl_window_resize(egl_window, width, height, 0, 0);
    }
    glViewport(0, 0, width, height);
}

void Renderer::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float win_aspect = (float)win_w / win_h;
    float img_aspect = (float)img_w / img_h;
    float scale_x = 1.0f, scale_y = 1.0f;

    if (win_aspect > img_aspect) {
        scale_x = img_aspect / win_aspect;
    } else {
        scale_y = win_aspect / img_aspect;
    }

    glUniform2f(scale_loc, scale_x, scale_y);

    GLfloat vertices[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, // pos x, y, tex u, v
         1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };

    GLint pos_loc = glGetAttribLocation(program, "position");
    GLint tex_loc = glGetAttribLocation(program, "texcoord");

    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices + 2);
    
    glEnableVertexAttribArray(pos_loc);
    glEnableVertexAttribArray(tex_loc);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    eglSwapBuffers(egl_display, egl_surface);
}

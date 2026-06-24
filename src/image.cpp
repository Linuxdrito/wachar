#include "image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Image::~Image() {
    if (data) stbi_image_free(data);
}

Image load_image(const char* filepath) {
    Image img;
    // Forzar 4 canales (RGBA) para OpenGL
    img.data = stbi_load(filepath, &img.width, &img.height, &img.channels, 4);
    if (!img.data) {
        std::cerr << "Error: No se pudo cargar la imagen " << filepath << "\n";
        exit(1);
    }
    return img;
}

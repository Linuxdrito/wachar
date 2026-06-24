#pragma once

struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = nullptr;

    ~Image();
};

Image load_image(const char* filepath);

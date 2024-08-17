#ifndef IMAGE_H
#define IMAGE_H

#include <glad/glad.h>
#include <stb_image.h>

#include <iostream>

class Image {
public:
    int width;
    int height;
    int channels;
    GLint internalFormat;
    GLint dataFormat;
    stbi_uc *data {nullptr};

    explicit Image(const char *path) {
        data = stbi_load(path, &width, &height, &channels, 0);
        if (data) {
            switch (channels) {
                case 1:
                    internalFormat = GL_RED;
                    dataFormat = GL_RED;
                    break;
                case 3:
                    internalFormat = GL_SRGB;
                    dataFormat = GL_RGB;
                break;
                case 4:
                    internalFormat = GL_SRGB_ALPHA;
                    dataFormat = GL_RGBA;
                break;
                default:
                    std::cerr << "Image::" << path << "::Unknwon number of channels: " << channels << std::endl;
                return;
            }
        }
    }

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    ~Image() {
        if (data) {
            stbi_image_free(data);
        }
    }
};

#endif //IMAGE_H

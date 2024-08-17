#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <glad/glad.h>
#include <learnopengl/shader.h>
#include <learnopengl/image.h>
#include <learnopengl/texture.h>

#include <iostream>

class Texture2D : public AbstractTexture {

public:
    explicit Texture2D(const char *path, const bool gammaCorrection = false) : AbstractTexture{GL_TEXTURE_2D} {
        const Image image {path};
        if (image.data) {
            glTexImage2D(
                m_target,
                0,
                gammaCorrection ? image.internalFormat : image.dataFormat,
                image.width,
                image.height,
                0,
                image.dataFormat,
                GL_UNSIGNED_BYTE,
                image.data
            );
            glGenerateMipmap(m_target);
        } else {
            std::cerr << "Texture2D::Failed to load at path: " << path << std::endl;
        }
    }
};

#endif //TEXTURE2D_H

#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <learnopengl/image.h>
#include <learnopengl/shader.h>
#include <learnopengl/texture.h>

#include <glm/glm.hpp>

#include <iostream>
#include <vector>

class CubeMap {

  public:
    explicit CubeMap(
        Shader &shader, const std::vector<std::string> &faces,
        const bool gammaCorrection = false)
        : m_texture { GL_TEXTURE_CUBE_MAP }
        , m_shader { shader } {
        stbi_set_flip_vertically_on_load(false);
        for (auto i { 0u }; i < faces.size(); ++i) {
            const Image face { faces[i].c_str() };
            if (!face.data) {
                std::cerr << "CubeMap::Failed to load at path: "
                          << faces[i].c_str() << std::endl;
                stbi_set_flip_vertically_on_load(true);
                return;
            }
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                gammaCorrection ? face.internalFormat : face.dataFormat,
                face.width, face.height, 0, face.dataFormat, GL_UNSIGNED_BYTE,
                face.data);
        }
        stbi_set_flip_vertically_on_load(true);

        m_texture.set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        m_texture.set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        m_texture.set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        m_texture.set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_texture.set(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        float cubeMapVertices[] = {
            // positions
            -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

            -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
            -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

            1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

            -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
            1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(cubeMapVertices), &cubeMapVertices,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        shader.uniform("skybox", 0);
    }

    void draw(const glm::mat4 &view, const glm::mat4 &projection) const {
        glDepthFunc(GL_LEQUAL);
        m_shader.uniform("view", glm::mat4(glm::mat3(view)));
        m_shader.uniform("projection", projection);

        glBindVertexArray(VAO);
        m_texture.activate(0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

  private:
    GLuint VAO;
    GLuint VBO;
    AbstractTexture m_texture;
    Shader &m_shader;
};

#endif // CUBEMAP_H

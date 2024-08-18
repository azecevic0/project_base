#ifndef HDR_H
#define HDR_H

#include <glad/glad.h>

#include <learnopengl/shader.h>

#include <iostream>

class HDR {

public:
    HDR(const unsigned width, const unsigned height) {
        resize(width, height);

        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FB0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void render(Shader &shader) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
        shader.uniform("hdr", m_is_hdr);
        shader.uniform("exposure", m_exposure);

        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void resize(const unsigned width, const unsigned height) {
        glDeleteFramebuffers(1, &m_FB0);
        glDeleteTextures(1, &m_colorBuffer);
        glDeleteRenderbuffers(1, &m_rboDepth);

        glGenFramebuffers(1, &m_FB0);
        // create floating point color buffer
        glGenTextures(1, &m_colorBuffer);
        glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // create depth buffer (renderbuffer)
        glGenRenderbuffers(1, &m_rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        // attach buffers
        glBindFramebuffer(GL_FRAMEBUFFER, m_FB0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    float exposure() const {
        return m_exposure;
    }

    void setExposure(const float exposure) {
        m_exposure = exposure;
    }

    bool mode() const {
        return m_is_hdr;
    }

    void setMode(const bool mode) {
        m_is_hdr = mode;
    }

private:
    unsigned m_width;
    unsigned m_height;
    GLuint m_FB0 {0u};
    GLuint m_colorBuffer {0u};
    GLuint m_rboDepth {0u};

    GLuint m_quadVAO;
    GLuint m_quadVBO;

    bool m_is_hdr {true};
    float m_exposure {1.0f};
};

#endif //HDR_H

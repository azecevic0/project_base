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
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void *) (3 * sizeof(float)));
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void render(Shader &shader) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorBuffer[0]);
        shader.uniform("hdr", m_is_hdr);
        shader.uniform("exposure", m_exposure);

        renderQuad();
    }

    void resize(const unsigned width, const unsigned height) {
        glDeleteFramebuffers(1, &m_FBO);
        glDeleteTextures(2, m_colorBuffer);
        glDeleteRenderbuffers(1, &m_rboDepth);

        glDeleteFramebuffers(2, m_pingpongFBO);
        glDeleteTextures(2, m_pingpongColorbuffers);

        // configure (floating point) framebuffers
        // ---------------------------------------
        glGenFramebuffers(1, &m_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        // create 2 floating point color buffers (1 for normal rendering, other
        // for brightness threshold values)
        glGenTextures(2, m_colorBuffer);
        for (unsigned int i = 0; i < 2; i++) {
            glBindTexture(GL_TEXTURE_2D, m_colorBuffer[i]);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter
                                   // would otherwise sample repeated texture
                                   // values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // attach texture to framebuffer
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                m_colorBuffer[i], 0);
        }
        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &m_rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
        glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);
        // tell OpenGL which color attachments we'll use (of this framebuffer)
        // for rendering
        unsigned attachments[2] = { GL_COLOR_ATTACHMENT0,
                                    GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ping-pong-framebuffer for blurring
        glGenFramebuffers(2, m_pingpongFBO);
        glGenTextures(2, m_pingpongColorbuffers);
        for (unsigned int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[i]);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
                GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter
                                   // would otherwise sample repeated texture
                                   // values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                m_pingpongColorbuffers[i], 0);
            // also check if framebuffers are complete (no need for depth
            // buffer)
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
                GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    float exposure() const { return m_exposure; }

    void setExposure(const float exposure) { m_exposure = exposure; }

    bool mode() const { return m_is_hdr; }

    void setMode(const bool mode) { m_is_hdr = mode; }

    bool bloomState() const { return m_is_bloom; }

    void setBloomState(const bool state) { m_is_bloom = state; }

    GLuint buffer() const { return m_FBO; }

    void blur(Shader &shaderBlur) {
        m_horizontal = true;
        bool first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[m_horizontal]);
            shaderBlur.uniform("horizontal", m_horizontal);
            glBindTexture(
                GL_TEXTURE_2D,
                first_iteration
                    ? m_colorBuffer[1]
                    : m_pingpongColorbuffers
                          [!m_horizontal]); // bind texture of other framebuffer
                                            // (or scene if first iteration)
            renderQuad();
            m_horizontal = !m_horizontal;
            first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bloom(Shader &shaderBloom) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloom.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorBuffer[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[!m_horizontal]);
        shaderBloom.uniform("bloom", m_is_bloom);
        shaderBloom.uniform("exposure", m_exposure);
        renderQuad();
    }

  private:
    void renderQuad() const {
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    GLuint m_FBO { 0u };
    GLuint m_colorBuffer[2] { 0u, 0u };
    GLuint m_rboDepth { 0u };

    GLuint m_pingpongFBO[2] { 0u, 0u };
    GLuint m_pingpongColorbuffers[2] { 0u, 0u };
    bool m_horizontal { true };

    GLuint m_quadVAO { 0u };
    GLuint m_quadVBO { 0u };

    bool m_is_hdr { true };
    bool m_is_bloom { true };
    float m_exposure { 1.0f };
};

#endif // HDR_H

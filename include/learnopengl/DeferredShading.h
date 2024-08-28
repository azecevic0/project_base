#ifndef DEFERREDSHADING_H
#define DEFERREDSHADING_H

#include <glad/glad.h>

#include <learnopengl/shader.h>

#include <iostream>

class DeferredShading {

  public:
    DeferredShading(
        const unsigned width, const unsigned height, Shader &geometryPass,
        Shader &lightingPass)
        : m_geometryPass { geometryPass }
        , m_lightingPass { lightingPass } {
        resize(width, height);
        m_lightingPass.uniform("gPosition", 0);
        m_lightingPass.uniform("gNormal", 1);
        m_lightingPass.uniform("gAlbedoSpec", 2);

        float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
            GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            (void *) (3 * sizeof(float)));
    }

    void resize(const unsigned width, const unsigned height) {
        glDeleteFramebuffers(1, &m_gBuffer);
        glDeleteTextures(1, &m_gPosition);
        glDeleteTextures(1, &m_gNormal);
        glDeleteTextures(1, &m_gAlbedoSpec);
        glDeleteRenderbuffers(1, &m_rboDepth);
        // configure g-buffer framebuffer
        // ------------------------------
        glGenFramebuffers(1, &m_gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        // position color buffer
        glGenTextures(1, &m_gPosition);
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT,
            nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition,
            0);
        // normal color buffer
        glGenTextures(1, &m_gNormal);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT,
            nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);
        // color + specular color buffer
        glGenTextures(1, &m_gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedoSpec,
            0);
        // tell OpenGL which color attachments we'll use (of this framebuffer)
        // for rendering
        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0,
                                        GL_COLOR_ATTACHMENT1,
                                        GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
        // create and attach depth buffer (renderbuffer)
        glGenRenderbuffers(1, &m_rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rboDepth);
        glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboDepth);
        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_width = width;
        m_height = height;
    }

    void render(GLuint fbo) {
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer);
        glBindFramebuffer(
            GL_DRAW_FRAMEBUFFER, fbo); // write to default framebuffer
        glBlitFramebuffer(
            0, 0, m_width, m_height, 0, 0, m_width, m_height,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }

    void bindGBuffer() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void bindTextures() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_lightingPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_gAlbedoSpec);
    }

    void unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    Shader &geometryPassShader() { return m_geometryPass; }

    Shader &lightingPassShader() { return m_lightingPass; }

  private:
    GLuint m_gBuffer { 0u };
    GLuint m_gPosition { 0u };
    GLuint m_gNormal { 0u };
    GLuint m_gAlbedoSpec { 0u };
    GLuint m_rboDepth { 0u };

    unsigned m_width {};
    unsigned m_height {};

    GLuint m_quadVAO {};
    GLuint m_quadVBO {};

    Shader &m_geometryPass;
    Shader &m_lightingPass;
};

#endif // DEFERREDSHADING_H

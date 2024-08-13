#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class AbstractTexture {

public:
    void set(GLenum param, GLint value) {
        bind();
        glTexParameteri(m_target, param, value);
    }

    void bind() const {
        glBindTexture(m_target, m_texture);
    }

    void activate(unsigned location) const {
        glActiveTexture(GL_TEXTURE0 + location);
        bind();
    }

protected:
    explicit AbstractTexture(const GLuint target) : m_target{target} {
        glGenTextures(1, &m_texture);
        glBindTexture(m_target, m_texture);
    }

    GLuint m_texture {};
    GLuint m_target {};
};

#endif //TEXTURE_H

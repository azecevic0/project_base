#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <common.h>
class Shader
{
public:
    // TODO: move to private section, has dependency in Mesh
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        auto vertex = loadShader(vertexPath, GL_VERTEX_SHADER);
        auto fragment = loadShader(fragmentPath, GL_FRAGMENT_SHADER);

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkLinkingErrors(ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    { 
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, bool value)
    {
        use();
        glUniform1i(location(name), (int)value);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, int value)
    {
        use();
        glUniform1i(location(name), value);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, float value)
    {
        use();
        glUniform1f(location(name), value);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::vec2 &value)
    {
        use();
        glUniform2fv(location(name), 1, &value[0]);
    }
    void uniform(const std::string &name, float x, float y)
    {
        use();
        glUniform2f(location(name), x, y);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::vec3 &value)
    { 
        glUniform3fv(location(name), 1, &value[0]);
    }
    void uniform(const std::string &name, float x, float y, float z)
    {
        use();
        glUniform3f(location(name), x, y, z);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::vec4 &value)
    {
        use();
        glUniform4fv(location(name), 1, &value[0]);
    }
    void uniform(const std::string &name, float x, float y, float z, float w)
    {
        use();
        glUniform4f(location(name), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::mat2 &mat)
    {
        use();
        glUniformMatrix2fv(location(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::mat3 &mat)
    {
        use();
        glUniformMatrix3fv(location(name), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void uniform(const std::string &name, const glm::mat4 &mat)
    {
        use();
        glUniformMatrix4fv(location(name), 1, GL_FALSE, &mat[0][0]);
    }

private:

    GLint location(const std::string &name) {
        const auto iloc = m_location.find(name);
        if (iloc == m_location.end()) {
            return m_location[name] = glGetUniformLocation(ID, name.c_str());
        }
        return iloc->second;
    }

    static GLuint loadShader(const char *path, GLenum type) {
        std::string code;
        try {
            std::ifstream file;
            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            file.open(path);

            std::stringstream stream;
            stream << file.rdbuf();
            code = stream.str();
        } catch (std::ifstream::failure&) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
            return 0;
        }

        auto ccode = code.c_str();

        auto shader = glCreateShader(type);
        glShaderSource(shader, 1, &ccode, nullptr);
        glCompileShader(shader);
        checkCompileErrors(shader, type);

        return shader;
    }

    // utility function for checking shader compilation errors.
    // ------------------------------------------------------------------------
    static void checkCompileErrors(GLuint shader, GLenum type) {
        GLint success;
        GLchar infoLog[1024];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR::"
                << (type == GL_VERTEX_SHADER ? "VERTEX_SHADER" : "FRAGMENT_SHADER") << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }

    // utility function for checking shader linking errors.
    // ------------------------------------------------------------------------
    static void checkLinkingErrors(GLuint program) {
        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(program, 1024, nullptr, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR\n"
                << infoLog << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }

    std::unordered_map<std::string, GLint> m_location;
};
#endif

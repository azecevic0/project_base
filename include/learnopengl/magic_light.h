#ifndef MAGIC_LIGHT_H
#define MAGIC_LIGHT_H

#include <glad/glad.h>

#include <learnopengl/shader.h>

#include <glm/glm.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

class MagicLight {

public:
    MagicLight(const glm::vec3 &position, const glm::vec3 &color, const unsigned index, Shader &shader)
        : m_position {position},
          m_index {index},
          m_shader {shader},
          m_speed {random()},
          m_direction {direction()}
    {
        shader.uniform("magicLights[" + std::to_string(index) + "].ambient", 0.1f * color);
        shader.uniform("magicLights[" + std::to_string(index) + "].diffuse", color);
        shader.uniform("magicLights[" + std::to_string(index) + "].specular", color);
        // update attenuation parameters and calculate radius
        const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
        const float linear = 0.14f;
        const float quadratic = 0.07f;
        shader.uniform("magicLights[" + std::to_string(index) + "].constant", linear);
        shader.uniform("magicLights[" + std::to_string(index) + "].linear", linear);
        shader.uniform("magicLights[" + std::to_string(index) + "].quadratic", quadratic);
        // then calculate radius of light volume/sphere
        const float maxBrightness = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (65536.0f / 128.0f) * maxBrightness))) / (2.0f * quadratic);
        shader.uniform("magicLights[" + std::to_string(index) + "].radius", radius);
    }

    void nextFrame(const float currentFrame) {
        auto nextPosition = m_position + glm::vec3(4.0 * cos(m_direction * m_speed * currentFrame), 4.0f, 4.0 * sin(m_direction * m_speed * currentFrame));
        m_shader.uniform("magicLights[" + std::to_string(m_index) + "].position", nextPosition);
    }

private:

    double random() const {
        double min {1.0};
        double max {4.0};

        double weight {static_cast<double>(std::rand()) / RAND_MAX};
        std::cout << weight << std::endl;
        return min + weight * (max - min);
    }

    int direction() {
        return (std::rand() % 2) * 2 - 1;
    }

    glm::vec3 m_position;
    const unsigned m_index;
    Shader &m_shader;
    double m_speed;
    int m_direction;
};

#endif //MAGIC_LIGHT_H

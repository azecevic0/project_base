#ifndef VAMPIRE_H
#define VAMPIRE_H

#include <learnopengl/model.h>
#include <learnopengl/shader.h>

#include <deque>

enum VampireState {
    APPROACHING,
    ROTATING,
    AWAY
};

class Vampire {

public:
    Vampire()
        : m_vampireModel{"resources/objects/dracula/dracula.obj"},
          m_garlicModel{"resources/objects/garlic/garlic.obj"}
    {
        m_vampireModel.SetShaderTextureNamePrefix("material.");
        m_garlicModel.SetShaderTextureNamePrefix("material.");
    }

    void draw(Shader &shader, const float frameTime, const float delta) {
        switch (m_state) {
            case APPROACHING:
                handleApproaching(delta);
                break;
            case ROTATING:
                handleRotating(delta);
                break;
            case AWAY:
                handleAway(delta);
                break;
        }

        while (!m_garlicTime.empty() && frameTime - m_garlicTime.front() > 3.0f) {
            m_garlicPosition.pop_front();
            m_garlicModelMatrix.pop_front();
            m_garlicTime.pop_front();
        }

        for (const auto &modelMatrix : m_garlicModelMatrix) {
            shader.uniform("model", modelMatrix);
            m_garlicModel.Draw(shader);
        }

        shader.uniform("material.shininess", 4.0f);
        shader.uniform("model", m_vampireModelMatrix);
        m_vampireModel.Draw(shader);
    }

    void attack(const glm::vec3 &position, const glm::vec3 &direction, const float time) {
        auto planarDirection = 10.0f * glm::normalize(glm::vec2(direction.x, direction.z));
        glm::vec3 garlicPosition {position.x + planarDirection.x, 2.2f, position.z + planarDirection.y};

        auto model = glm::translate(glm::mat4(1.0f), garlicPosition);
        model = glm::scale(model, glm::vec3(0.2f));

        m_garlicPosition.push_back(garlicPosition);
        m_garlicModelMatrix.push_back(model);
        m_garlicTime.push_back(time);
    }

private:
    void handleApproaching(const float delta) {
        m_vampirePosition.z -= 5.0f * delta;
        if (isAttacked() || m_vampirePosition.z < -30.0f) {
            m_state = ROTATING;
        }
        setupVampireModelMatrix();
    }

    void handleRotating(const float delta) {
        m_angle += delta * 0.5f * 360.0f;
        if (m_angle > 540.0f) {
            m_angle = 180.0f;
            m_state = AWAY;
        }
        setupVampireModelMatrix();
    }

    void handleAway(const float delta) {
        m_vampirePosition.z += 5.0f * delta;
        if (m_vampirePosition.z > 70.0f) {
            m_state = APPROACHING;
        }
        setupVampireModelMatrix();
    }

    void setupVampireModelMatrix() {
        m_vampireModelMatrix = glm::mat4(1.0f);
        m_vampireModelMatrix = glm::translate(m_vampireModelMatrix, m_vampirePosition);
        m_vampireModelMatrix = glm::rotate(m_vampireModelMatrix, glm::radians(m_angle), glm::vec3(0, 1, 0));
        m_vampireModelMatrix = glm::scale(m_vampireModelMatrix, glm::vec3(VAMPIRE_SCALE));
    }

    bool isAttacked() {
        const auto x {m_vampirePosition.x};
        const auto z {m_vampirePosition.z};

        for (const auto &pos : m_garlicPosition) {
            if ((pos.x - x) * (pos.x - x) + (pos.z - z) * (pos.z - z) < 100.0f) {
                return true;
            }
        }
        return false;
    }

    Model m_vampireModel;
    Model m_garlicModel;
    VampireState m_state {APPROACHING};

    glm::vec3 m_vampirePosition {0.0f, 2.0f, 70.0f};
    float m_angle {180.0f};
    const float VAMPIRE_SCALE = 2.5f;
    glm::mat4 m_vampireModelMatrix;

    std::deque<glm::vec3> m_garlicPosition;
    std::deque<glm::mat4> m_garlicModelMatrix;
    std::deque<float> m_garlicTime;
};

#endif //VAMPIRE_H

#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Canis {
    struct Transform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };

    static glm::mat4 Matrix(Transform& _transform) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, _transform.position);
        transform = glm::rotate(transform, _transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, _transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, _transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, _transform.scale);
        return transform;
    }
}
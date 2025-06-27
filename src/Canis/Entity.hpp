#pragma once
#include <string>
#include "Model.hpp"
#include "Shader.hpp"
#include "Data/Transform.hpp"
#include "Data/GLTexture.hpp"
#include "ScriptInstance.hpp"

namespace Canis
{
    class World;

    struct Entity
    {
        bool active = true;
        std::string name;
        std::string tag;
        Transform transform;
        Model *model;
        Shader *shader;
        glm::vec3 color = glm::vec3(1.0f);
        GLTexture *albedo;
        GLTexture *specular;
        ScriptInstance* script = nullptr;
    };
}
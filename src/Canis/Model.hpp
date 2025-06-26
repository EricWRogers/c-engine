#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace Canis
{
    struct Model
    {
        std::string path;
        unsigned int VAO;
        unsigned int VBO;
        std::vector<float> vertices = {};
        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec2> uvs = {};
        std::vector<glm::vec3> normals = {};
    };

    extern Model LoadModel(std::string _path);

    extern void Draw(Model &_model);
} // end of Canis namespace
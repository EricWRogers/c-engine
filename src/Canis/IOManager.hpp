#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Data/GLTexture.hpp"

namespace Canis
{
    extern GLTexture LoadImageGL(std::string _path, bool _wrap);

    extern GLTexture LoadImageGL(std::string _path, int _sourceFormat, int _format, bool _wrap);

    extern unsigned int LoadImageToCubemap(std::vector<std::string> _faces, int _sourceFormat);

    extern bool LoadOBJ(std::string _path,
                        std::vector<glm::vec3> &_positions,
                        std::vector<glm::vec2> &_uvs,
                        std::vector<glm::vec3> &_normals);

    extern std::vector<float> LoadOBJ(std::string _path);
} // end of Canis namespace
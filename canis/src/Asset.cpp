#include <Canis/Asset.hpp>
#include <Canis/Canis.hpp>
#include <Canis/Debug.hpp>
#include <Canis/IOManager.hpp>
#include <Canis/External/OpenGl.hpp>
#include <memory>
#include <string.h>
#include <unordered_map>
#define TINYGLTF_IMPLEMENTATION

namespace Canis
{
    Asset::Asset() {}

    Asset::~Asset() {}

    bool TextureAsset::Load(std::string _path)
    {
        m_path = _path;
        m_texture = LoadImageToGLTexture(_path, GL_RGBA, GL_RGBA);
        return true;
    }

    bool TextureAsset::Free()
    {
        glDeleteTextures(1, &m_texture.id);
        return true;
    }
}
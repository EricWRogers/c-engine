#pragma once
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include <Canis/Shader.hpp>
#include <Canis/Data/GLTexture.hpp>
#include <Canis/Data/Character.hpp>
#include <Canis/Data/Vertex.hpp>
#include <Canis/Data/DefaultMeshData.hpp>

#include <yaml-cpp/node/node.h>

namespace Canis
{
    class Asset
    {
    public:
        Asset();
        ~Asset();

        Asset(const Asset &) = delete;
        Asset &operator=(const Asset &) = delete;

        virtual bool Load(std::string _path) = 0;
        virtual bool Free() = 0;
    };

    class TextureAsset : public Asset
    {
    private:
        GLTexture m_texture;
        std::string m_path = "";

    public:
        bool Load(std::string _path) override;
        bool Free() override;
        GLTexture GetGLTexture() { return m_texture; }
        GLTexture *GetPointerToTexture() { return &m_texture; }
        std::string GetPath() { return m_path; }
    };
} // end of Canis namespace
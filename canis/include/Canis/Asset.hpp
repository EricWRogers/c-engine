#pragma once
#include <string>

#include <Canis/Shader.hpp>
#include <Canis/Data/GLTexture.hpp>

#include <Canis/UUID.hpp>
#include <Canis/Data/Types.hpp>

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

    class ShaderAsset : public Asset
    {
    private:
        Canis::Shader *m_shader;

    public:
        ShaderAsset() { m_shader = new Canis::Shader(); }

        bool Load(std::string _path) override;
        bool Free() override;

        Canis::Shader *GetShader() { return m_shader; }
    };

    class MetaFileAsset : public Asset
    {
    private:
    public:
        enum FileType {
            FILE_UNKNOWN,
            FRAGMENT,
            VERTEX,
            TEXTURE,
            SCENE,
        };

        MetaFileAsset() {}

        void CreateMetaFile(std::string _path);

        bool Load(std::string _path) override;
        bool Free() override { return true; }

        FileType type = FILE_UNKNOWN;
        UUID uuid;
        std::string path = "";
        std::string name = "";
        std::string extension = "";
        u64 size = 0;
        // i64 created
        i64 modified = 0; // nanoseconds since the Unix epoch (Jan 1, 1970).
        // i64 accessed
    };
} // end of Canis namespace
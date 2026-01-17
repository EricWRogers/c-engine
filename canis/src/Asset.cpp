#include <Canis/Asset.hpp>
#include <Canis/Yaml.hpp>
#include <Canis/Debug.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/IOManager.hpp>
#include <memory>
#include <string.h>
#include <unordered_map>

#include <SDL3/SDL.h>
#include <SDL3/SDL_filesystem.h>

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

    bool ShaderAsset::Load(std::string _path)
    {
        m_shader->Compile(
            _path + ".vs",
            _path + ".fs");

        return true;
    }

    bool ShaderAsset::Free()
    {
        delete m_shader;
        return true;
    }

    std::string FileTypeToString(MetaFileAsset::FileType _type)
    {
        switch (_type)
        {
            case MetaFileAsset::FileType::FRAGMENT:
                return "FRAGMENT";
            case MetaFileAsset::FileType::VERTEX:
                return "VERTEX";
            case MetaFileAsset::FileType::TEXTURE:
                return "TEXTURE";
            case MetaFileAsset::FileType::SCENE:
                return "SCENE";
            default:
                return "FILE_UNKNOWN";
        }
    }

    MetaFileAsset::FileType StringToFileType(std::string _type)
    {
        if (_type == "FRAGMENT")
            return MetaFileAsset::FileType::FRAGMENT;
        else if (_type == "VERTEX")
            return MetaFileAsset::FileType::VERTEX;
        else if (_type == "TEXTURE")
            return MetaFileAsset::FileType::TEXTURE;
        else if (_type == "SCENE")
            return MetaFileAsset::FileType::SCENE;
        else
            return MetaFileAsset::FileType::FILE_UNKNOWN;
    }
    
    void MetaFileAsset::CreateMetaFile(std::string _path)
    {
        SDL_PathInfo info;

        if (SDL_GetPathInfo(_path.c_str(), &info))
        {
            path = _path;
            name = GetFileName(_path);
            extension = GetFileExtension(_path);

            if (extension == "png")
                type = FileType::TEXTURE;
            else if (extension == "scene")
                type = FileType::SCENE;
            else if (extension == "fs")
                type = FileType::FRAGMENT;
            else if (extension == "vs")
                type = FileType::VERTEX;
            else
                type = FileType::FILE_UNKNOWN;

            size = info.size;
            modified = info.modify_time;

            Save();
        }
    }

    void MetaFileAsset::Save()
    {
        YAML::Node node;
        node["FileType"] = FileTypeToString(type);
        node["UUID"] = std::to_string(uuid);
        node["name"] = name;
        node["extension"] = extension;
        node["size"] = size;
        node["modified"] = modified;

        std::ofstream fout(path + ".meta");
        fout << node;
    }

    bool MetaFileAsset::Load(std::string _path)
    {
        if (FileExists((_path + ".meta").c_str()))
        {
            YAML::Node root = YAML::LoadFile(_path+".meta");
            type = StringToFileType(root["FileType"].as<std::string>());
            uuid = root["UUID"].as<uint64_t>(0);
            path = _path;
            name = root["name"].as<std::string>();
            extension = root["extension"].as<std::string>();
            size = root["size"].as<u64>();
            modified = root["modified"].as<i64>();
        }
        else
        {
            CreateMetaFile(_path);
        }

        return false;
    }

    bool SpriteAnimationAsset::Load(std::string _path)
    {
        return true;
    }

    bool SpriteAnimationAsset::Free()
    {
        return true;
    }
}
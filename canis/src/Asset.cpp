#include <Canis/Asset.hpp>
#include <Canis/Yaml.hpp>
#include <Canis/Debug.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/IOManager.hpp>
#include <memory>
#include <algorithm>
#include <string.h>
#include <unordered_map>

#include <SDL3/SDL.h>
#include <SDL3/SDL_filesystem.h>

#if defined(CANIS_HAS_FREETYPE)
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

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

    bool TextAsset::Load(std::string _path)
    {
        m_path = _path;

#if defined(CANIS_HAS_FREETYPE)
        FT_Library fontLibrary;
        if (FT_Init_FreeType(&fontLibrary))
        {
            Debug::Error("ERROR::FREETYPE: Could not init FreeType Library");
            return false;
        }

        FT_Face fontFace;
        if (FT_New_Face(fontLibrary, _path.c_str(), 0, &fontFace))
        {
            Debug::Error("ERROR::FREETYPE: Failed to load font: %s", _path.c_str());
            FT_Done_FreeType(fontLibrary);
            return false;
        }

        FT_Set_Pixel_Sizes(fontFace, 0, m_fontSize);

        int currentX = 0;
        int currentY = 0;
        int maxHeightInRow = 0;
        memset(m_atlasData, 0, sizeof(m_atlasData));

        for (unsigned char c = 32; c < 127; c++)
        {
            if (FT_Load_Char(fontFace, c, FT_LOAD_RENDER))
            {
                Debug::Warning("Failed to load glyph: %d", c);
                continue;
            }

            if (currentX + fontFace->glyph->bitmap.width > atlasWidth)
            {
                currentX = 0;
                currentY += maxHeightInRow;
                maxHeightInRow = 0;
            }

            if (currentY + fontFace->glyph->bitmap.rows > atlasHeight)
            {
                Debug::Error("Text atlas is too small for font '%s' at size %u", _path.c_str(), m_fontSize);
                break;
            }

            for (int y = 0; y < fontFace->glyph->bitmap.rows; y++)
            {
                for (int x = 0; x < fontFace->glyph->bitmap.width; x++)
                {
                    m_atlasData[(currentY + y) * atlasWidth + (currentX + x)] =
                        fontFace->glyph->bitmap.buffer[y * fontFace->glyph->bitmap.width + x];
                }
            }

            Character character = {};
            character.sizeX = fontFace->glyph->bitmap.width;
            character.sizeY = fontFace->glyph->bitmap.rows;
            character.bearingX = fontFace->glyph->bitmap_left;
            character.bearingY = fontFace->glyph->bitmap_top;
            character.advance = static_cast<unsigned int>(fontFace->glyph->advance.x);
            character.atlasPos = Vector2((float)currentX / atlasWidth, (float)currentY / atlasHeight);
            character.atlasSize = Vector2((float)fontFace->glyph->bitmap.width / atlasWidth, (float)fontFace->glyph->bitmap.rows / atlasHeight);

            characters[c] = character;

            currentX += fontFace->glyph->bitmap.width;
            maxHeightInRow = std::max(maxHeightInRow, (int)fontFace->glyph->bitmap.rows);
        }

        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, m_atlasData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Sample RED channel as alpha so text blends like a normal glyph mask.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        glBindTexture(GL_TEXTURE_2D, 0);

        FT_Done_Face(fontFace);
        FT_Done_FreeType(fontLibrary);

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
#else
        Debug::Error("TextAsset::Load called but engine was built without FreeType support.");
        return false;
#endif
    }

    bool TextAsset::Free()
    {
        if (m_vbo != 0)
            glDeleteBuffers(1, &m_vbo);

        if (m_vao != 0)
            glDeleteVertexArrays(1, &m_vao);

        if (m_texture != 0)
            glDeleteTextures(1, &m_texture);

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
            case MetaFileAsset::FileType::ANIMATIONCLIP2D:
                return "ANIMATIONCLIP2D";
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
        else if (_type == "ANIMATIONCLIP2D")
            return MetaFileAsset::FileType::ANIMATIONCLIP2D;
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
            else if (extension == "ac2d")
                type = FileType::ANIMATIONCLIP2D;
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

#pragma once
#include <string>
#include <map>

#include <Canis/Asset.hpp>
#include <Canis/Debug.hpp>
#include <Canis/AssetHandle.hpp>

namespace Canis
{
    namespace AssetManager
    {
        struct AssetLibrary
        {
            int nextId{0};
            std::map<int, void *> assets{};
            std::map<std::string, int> assetPath{};
        };

        AssetLibrary &GetAssetLibrary();

        bool Has(std::string _name);

        template <typename T>
        T *Get(int id)
        {
            auto &assetLibrary = GetAssetLibrary();
            return (T *)assetLibrary.assets[id];
        }

        inline std::string GetPath(int _id)
        {
            auto &assetLibrary = GetAssetLibrary();

            for (const auto &[key, value] : assetLibrary.assetPath)
                if (value == _id)
                    return key;

            return std::string("Path was not found in AssetLibrary");
        }

        template <typename T>
        void Free(std::string _name)
        {
            auto &assetLibrary = GetAssetLibrary();
            if (!Has(_name))
                return;

            int assetId = assetLibrary.assetPath[_name];

            {
                std::map<std::string, int>::iterator it;
                it = assetLibrary.assetPath.find(_name);

                assetLibrary.assetPath.erase(it);
            }

            if (!assetLibrary.assets.contains(assetId))
                return;

            ((T *)assetLibrary.assets[assetId])->Free();
            delete ((T *)assetLibrary.assets[assetId]);

            {
                std::map<int, void *>::iterator it;
                it = assetLibrary.assets.find(assetId);

                assetLibrary.assets.erase(it);
            }
        }

        int LoadTexture(const std::string &_path);
        TextureAsset *GetTexture(const std::string &_path);
        TextureAsset *GetTexture(const int _textureID);

        TextureHandle GetTextureHandle(const std::string &_path);
        TextureHandle GetTextureHandle(const int _textureID);

        int LoadShader(const std::string &_pathWithOutExtension);
    } // end of AssetManager namespace
} // end of Canis namespace
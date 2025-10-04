#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>

namespace Canis
{
    namespace AssetManager
    {
        AssetLibrary &GetAssetLibrary()
        {
            static AssetLibrary assetLibrary = {};
            return assetLibrary;
        }

        bool Has(std::string _name)
        {
            auto &assetLibrary = GetAssetLibrary();
            return assetLibrary.assetPath.contains(_name);
        }

        int LoadTexture(const std::string &_path)
        {
            auto &assetLibrary = GetAssetLibrary();

            std::map<std::string, int>::iterator it;
            it = assetLibrary.assetPath.find(_path);

            // check if texture already exist
            if (it != assetLibrary.assetPath.end()) // found
            {
                return it->second;
            }

            // create texture
            Asset *texture = new TextureAsset();
            texture->Load(_path);
            int id = assetLibrary.nextId;

            // cache texture
            assetLibrary.assets[id] = texture;

            // cache id
            assetLibrary.assetPath[_path] = id;

            // increment id
            assetLibrary.nextId++;

            return id;
        }

        TextureAsset *GetTexture(const int _textureID)
        {
            return (TextureAsset *)GetAssetLibrary().assets[_textureID];
        }

        TextureAsset *GetTexture(const std::string &_path)
        {
            return GetTexture(LoadTexture(_path));
        }

        TextureHandle GetTextureHandle(const std::string &_path)
        {
            TextureHandle handle;
            handle.id = LoadTexture(_path);
            handle.texture = GetTexture(handle.id)->GetGLTexture();
            return handle;
        }

        TextureHandle GetTextureHandle(const int _textureID)
        {
            TextureHandle handle;
            handle.id = _textureID;
            handle.texture = GetTexture(_textureID)->GetGLTexture();
            return handle;
        }

        int LoadShader(const std::string &_pathWithOutExtension)
        {
            auto &assetLibrary = GetAssetLibrary();
            std::map<std::string, int>::iterator it;
            it = assetLibrary.assetPath.find(_pathWithOutExtension);

            // check if shader already exist
            if (it != assetLibrary.assetPath.end()) // found
            {
                return it->second;
            }

            // create shader
            Asset *shader = new ShaderAsset();
            shader->Load(_pathWithOutExtension);
            int id = assetLibrary.nextId;

            // cache shader
            assetLibrary.assets[id] = shader;

            // cache id
            assetLibrary.assetPath[_pathWithOutExtension] = id;

            // increment id
            assetLibrary.nextId++;

            return id;
        }
    } // end of AssetManager namespace

} // end of Canis namespace
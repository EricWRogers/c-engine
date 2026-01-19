#include <Canis/AssetManager.hpp>
#include <Canis/Debug.hpp>

#include <filesystem>

#include <Canis/Yaml.hpp>

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

        void MoveAsset(std::string _source, std::string _target)
        {
            namespace fs = std::filesystem;
            fs::path src = _source;
            fs::path dst = _target;

            auto &assetLibrary = GetAssetLibrary();

            // update meta
            MetaFileAsset* meta = GetMetaFile(_source);

            // name 
            meta->name = dst.filename().string();
            
            // path
            meta->path = _target;

            // assetPath
            int id = assetLibrary.assetPath[_source];
            assetLibrary.assetPath.erase(_source);
            assetLibrary.assetPath[_target] = id;

            // uuidAssetPath
            assetLibrary.uuidAssetPath[meta->uuid] = _target;

            // move asset
            std::error_code ec;
            fs::rename(src, dst, ec);
            // TODO: check error

            // move asset meta
            fs::path srcMeta = _source + ".meta";
            fs::path dstMeta = _target + ".meta";

            std::error_code ec2;
            fs::rename(srcMeta, dstMeta, ec2);
            // TODO: check error

            // save updated meta
            meta->Save();
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

        int LoadMetaFile(const std::string &_path)
        {
            auto &assetLibrary = GetAssetLibrary();

            std::map<std::string, int>::iterator it;
            it = assetLibrary.assetPath.find(_path+".meta");

            // check if meta file already exist
            if (it != assetLibrary.assetPath.end()) // found
            {
                return it->second;
            }

            // create texture
            Asset *metaFile = new MetaFileAsset();
            metaFile->Load(_path);
            int id = assetLibrary.nextId;

            // cache texture
            assetLibrary.assets[id] = metaFile;

            // cache id
            assetLibrary.assetPath[_path+".meta"] = id;

            // uuid
            assetLibrary.uuidAssetPath[((MetaFileAsset*)metaFile)->uuid] = _path;

            // increment id
            assetLibrary.nextId++;

            return id;
        }

        MetaFileAsset* GetMetaFile(const std::string &_path)
        {
            return GetMetaFile(LoadMetaFile(_path));
        }

        MetaFileAsset* GetMetaFile(const int _metaID)
        {
            if (GetAssetLibrary().assets.contains(_metaID))
                return (MetaFileAsset *)GetAssetLibrary().assets[_metaID];
            else
                return nullptr;
        }

        int LoadSpriteAnimation(const std::string &_path)
        {
            auto &assetLibrary = GetAssetLibrary();
            std::map<std::string, int>::iterator it;
            it = assetLibrary.assetPath.find(_path);

            // check if animation already exist
            if (it != assetLibrary.assetPath.end()) // found
            {
                return it->second;
            }

            // create animation
            Asset *anim = new SpriteAnimationAsset();
            anim->Load(_path);

            YAML::Node root = YAML::LoadFile(_path);

            if (YAML::Node animation = root["Animation"])
            {
                for (auto animationFrame : animation)
                {
                    SpriteFrame frame = {};
                    frame.timeOnFrame = animationFrame["timeOnFrame"].as<float>();
                    frame.textureId = LoadTexture(animationFrame["textureAssetId"].as<std::string>());
                    Vector2 offset = animationFrame["offset"].as<Vector2>();
                    frame.offsetX = offset.x;
                    frame.offsetY = offset.y;
                    Vector2 index = animationFrame["index"].as<Vector2>();
                    frame.row = index.x;
                    frame.col = index.y;
                    Vector2 size = animationFrame["size"].as<Vector2>();
                    frame.width = size.x;
                    frame.height = size.y;
                    ((SpriteAnimationAsset *)anim)->frames.push_back(frame);
                }
            }

            int id = assetLibrary.nextId;

            // cache animation
            assetLibrary.assets[id] = anim;

            // cache id
            assetLibrary.assetPath[_path] = id;

            // increment id
            assetLibrary.nextId++;

            return id;
        }
    
        SpriteAnimationAsset* GetSpriteAnimation(const std::string &_path)
        {
            return GetSpriteAnimation(LoadSpriteAnimation(_path));
        }

        SpriteAnimationAsset* GetSpriteAnimation(i32 _animationID)
        {
            return (SpriteAnimationAsset *)GetAssetLibrary().assets[_animationID];
        }
    } // end of AssetManager namespace

} // end of Canis namespace
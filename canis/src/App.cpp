#include "yaml-cpp/emittermanip.h"
#include <Canis/App.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_loadso.h>

#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/IOManager.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/AssetManager.hpp>

namespace Canis
{
    void App::Run()
    {
        Debug::Log("App Run");
#ifdef Win32
        const char *sharedObjectPath = "./libGameCode.dll";
#elif __APPLE__
        const char *sharedObjectPath = "./libGameCode.dylib";
#else
        const char *sharedObjectPath = "./libGameCode.so";
#endif

        // init window
        Window window("Canis Beta", 512, 512);
        window.SetClearColor(Color(1.0f));
        window.SetSync(Window::Sync::IMMEDIATE);

        // find all the meta files and load to asset manager
        std::vector<std::string> paths = FindFilesInFolder("assets", "");

        for (std::string path : paths)
            MetaFileAsset *meta = AssetManager::GetMetaFile(path);

        Editor editor;
        editor.Init(&window);

        RegisterDefaults(editor);

        InputManager inputManager;

        Time::Init(120.0f);

        scene.Init(this, &window, &inputManager, "assets/scenes/main.scene");

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);

        // call after all the systems are added
        // and after script from the game lib have been registered
        scene.Load(m_scriptRegistry);

        while (inputManager.Update((void *)&window))
        {
            window.Clear();

            float deltaTime = Time::StartFrame();

            if (editor.m_mode == EditorMode::PLAY)
                scene.Update(deltaTime);

            // call the dynamically loaded function
            if (editor.m_mode == EditorMode::PLAY)
                GameCodeObjectUpdateFunction(&gameCodeObject, this, deltaTime);
            
            // GameCodeObjectWatchFile(&gameCodeObject, this);

            scene.Render(deltaTime);
            editor.Draw(&scene, &window, this, &gameCodeObject);
            window.SwapBuffer();

            Time::EndFrame();
        }

        scene.Unload();
        Time::Quit();
        gameCodeObject.GameShutdownFunction((void *)this, gameCodeObject.gameData);
        SDL_UnloadObject(gameCodeObject.sharedObjectHandle);
    }

    void App::RegisterDefaults(Editor& _editor)
    {
        ScriptConf rectTransformConf = {
            .name = "Canis::RectTransform",
            .Add = [this](Entity& _entity) -> void {
                _entity.AddScript<RectTransform>();
            },
            .Has = [this](Entity& _entity) -> bool { return (_entity.GetScript<RectTransform>() != nullptr); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveScript<RectTransform>(); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.GetScript<RectTransform>())
                {
                    RectTransform& transform = *_entity.GetScript<RectTransform>();

                    YAML::Node comp;
                    comp["active"] = transform.active;
                    comp["position"] = transform.position;
                    comp["size"] = transform.size;
                    comp["scale"] = transform.scale;
                    comp["originOffset"] = transform.originOffset;
                    comp["depth"] = transform.depth;
                    comp["rotation"] = transform.rotation;

                    _node["Canis::RectTransform"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (auto rectTransform = _node["Canis::RectTransform"])
                {
                    auto &rt = *_entity.AddScript<Canis::RectTransform>(false);
                    rt.active = rectTransform["active"].as<bool>();
                    //rt.anchor = (Canis::RectAnchor)rectTransform["anchor"].as<int>();
                    rt.position = rectTransform["position"].as<Vector2>();
                    rt.size = rectTransform["size"].as<Vector2>();
                    rt.scale = rectTransform["scale"].as<Vector2>();
                    rt.originOffset = rectTransform["originOffset"].as<Vector2>();
                    rt.depth = rectTransform["depth"].as<float>();
                    rt.rotation = rectTransform["rotation"].as<float>();
                    //rt.scaleWithScreen = (ScaleWithScreen)rectTransform["scaleWithScreen"].as<int>(0);
                    rt.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                RectTransform* transform = nullptr;
                if ((transform = _entity.GetScript<RectTransform>()) != nullptr)
                {
                    ImGui::InputFloat2("position", &transform->position.x, "%.3f");
                    ImGui::InputFloat2("size", &transform->size.x, "%.3f");
                    ImGui::InputFloat2("scale", &transform->scale.x, "%.3f");
                    ImGui::InputFloat2("originOffset", &transform->originOffset.x, "%.3f");
                    ImGui::InputFloat("depth", &transform->depth);
                    // let user work with degrees
                    float degrees = RAD2DEG * transform->rotation;
                    ImGui::InputFloat("rotation", &degrees);
                    transform->rotation = DEG2RAD * degrees;
                }
            },
        };

        RegisterScript(rectTransformConf);

        ScriptConf sprite2DConf = {
            .name = "Canis::Sprite2D",
            .Add = [this](Entity& _entity) -> void {
                // TODO: require a RectTransform component
                Sprite2D* sprite = _entity.AddScript<Sprite2D>();
                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                //sprite->size.x = sprite->textureHandle.texture.width;
                //sprite->size.y = sprite->textureHandle.texture.height;
            },
            .Has = [this](Entity& _entity) -> bool { return (_entity.GetScript<Sprite2D>() != nullptr); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveScript<Sprite2D>(); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.GetScript<Canis::Sprite2D>())
                {
                    Sprite2D& sprite = *_entity.GetScript<Sprite2D>();

                    YAML::Node comp;
                    comp["color"] = sprite.color;
                    comp["uv"] = sprite.uv;

                    YAML::Node textureAsset;
                    textureAsset["uuid"] = (uint64_t)AssetManager::GetMetaFile(AssetManager::Get<TextureAsset>(sprite.textureHandle.id)->GetPath())->uuid;
                    
                    comp["TextureAsset"] = textureAsset;
                    _node["Canis::Sprite2D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (auto sprite2DComponent = _node["Canis::Sprite2D"])
                {
                    auto &sprite = *_entity.AddScript<Canis::Sprite2D>(false);
                    sprite.color = sprite2DComponent["color"].as<Vector4>();
                    sprite.uv = sprite2DComponent["uv"].as<Vector4>();
                    if (auto textureAsset = sprite2DComponent["TextureAsset"])
                    {
                        UUID uuid = textureAsset["uuid"].as<uint64_t>();
                        std::string path = AssetManager::GetPath(uuid);
                        Debug::Log("Path: %s", path.c_str());
                        sprite.textureHandle = AssetManager::GetTextureHandle(path);
                    }
                    //sprite.textureHandle = sprite2DComponent["TextureHandle"].as<TextureHandle>();//AssetManager::GetTextureHandle(sprite2DComponent["textureHandle"].as<std::string>());
                    sprite.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Sprite2D* sprite = nullptr;
                if ((sprite = _entity.GetScript<Sprite2D>()) != nullptr)
                {
                    // textureHandle
                    ImGui::ColorEdit4("color", &sprite->color.r);
                    ImGui::InputFloat4("uv", &sprite->uv.x, "%.3f");

                    ImGui::Text("texture");

                    ImGui::SameLine();

                    ImGui::Button(AssetManager::GetMetaFile(AssetManager::GetPath(sprite->textureHandle.id))->name.c_str(), ImVec2(150, 0));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
                        {
                            const AssetDragData dropped = *static_cast<const AssetDragData*>(payload->Data);
                            std::string path = AssetManager::GetPath(dropped.uuid);
                            TextureAsset* asset = AssetManager::GetTexture(path);

                            if (asset)
                            {
                                sprite->textureHandle = AssetManager::GetTextureHandle(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
            },
        };

        RegisterScript(sprite2DConf);

        ScriptConf camera2DConf = {
            .name = "Canis::Camera2D",
            .Add = [this](Entity& _entity) -> void { _entity.AddScript<Camera2D>(); },
            .Has = [this](Entity& _entity) -> bool { return (_entity.GetScript<Camera2D>() != nullptr); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveScript<Camera2D>(); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.GetScript<Canis::Camera2D>())
                {
                    Camera2D& camera = *_entity.GetScript<Camera2D>();

                    YAML::Node comp;
                    comp["position"] = camera.GetPosition();
                    comp["scale"] = camera.GetScale();

                    _node["Canis::Camera2D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (auto camera2DComponent = _node["Canis::Camera2D"])
                {
                    auto &camera = *_entity.AddScript<Canis::Camera2D>(false);
                    camera.SetPosition(camera2DComponent["position"].as<Vector2>(camera.GetPosition()));
                    camera.SetScale(camera2DComponent["scale"].as<float>(camera.GetScale()));
                    camera.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Camera2D* camera = nullptr;
                if ((camera = _entity.GetScript<Camera2D>()) != nullptr)
                {
                    Vector2 lastPosition = camera->GetPosition();
                    float lastScale = camera->GetScale();

                    ImGui::InputFloat2(("position##" + _conf.name).c_str(), &lastPosition.x, "%.3f");
                    ImGui::InputFloat(("scale##" + _conf.name).c_str(), &lastScale);

                    if (lastPosition != camera->GetPosition())
                        camera->SetPosition(lastPosition);
                    
                    if (lastScale != camera->GetScale())
                        camera->SetScale(lastScale);
                }
            },
        };

        RegisterScript(camera2DConf);

        // register inspector items
        InspectorItemRightClick inspectorCreateSquare = {
            .name = "Create Square",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *entityOne = _app.scene.CreateEntity("Square");
                RectTransform * transform = entityOne->AddScript<Canis::RectTransform>();
                Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();

                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                transform->size = Vector2(64.0f);
            }
        };

        RegisterInspectorItem(inspectorCreateSquare);

        InspectorItemRightClick inspectorCreateCircle = {
            .name = "Create Circle",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *entityOne = _app.scene.CreateEntity("Circle");
                RectTransform * transform = entityOne->AddScript<Canis::RectTransform>();
                Canis::Sprite2D *sprite = entityOne->AddScript<Canis::Sprite2D>();

                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/circle.png");
                transform->size = Vector2(64.0f);
            }
        };

        RegisterInspectorItem(inspectorCreateCircle);
    }

    float App::FPS()
    {
        return Time::FPS();
    }

    float App::DeltaTime()
    {
        return Time::DeltaTime();
    }

    void App::SetTargetFPS(float _targetFPS)
    {
        Time::SetTargetFPS(_targetFPS);
    }

    void App::RegisterScript(ScriptConf &_conf)
    {
        for (ScriptConf &sc : m_scriptRegistry)
            if (_conf.name == sc.name)
                return;

        m_scriptRegistry.push_back(_conf);
    }

    void App::UnregisterScript(ScriptConf &_conf)
    {
        for (int i = 0; i < m_scriptRegistry.size(); i++)
        {
            if (_conf.name == m_scriptRegistry[i].name)
            {
                m_scriptRegistry.erase(m_scriptRegistry.begin() + i);
                i--;
            }
        }
    }

    void App::RegisterInspectorItem(InspectorItemRightClick& _item)
    {
        for (InspectorItemRightClick &item : m_inspectorItemRegistry)
            if (item.name == _item.name)
                return;

        m_inspectorItemRegistry.push_back(_item);
    }

    void App::UnregisterInspectorItem(InspectorItemRightClick& _item)
    {
        for (int i = 0; i < m_inspectorItemRegistry.size(); i++)
        {
            if (_item.name == m_inspectorItemRegistry[i].name)
            {
                m_inspectorItemRegistry.erase(m_inspectorItemRegistry.begin() + i);
                i--;
            }
        }
    }

} // namespace Canis

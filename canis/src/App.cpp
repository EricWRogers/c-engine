#include "yaml-cpp/emittermanip.h"
#include <Canis/App.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_loadso.h>

#include <Canis/Canis.hpp>
#include <Canis/GameCodeObject.hpp>
#include <Canis/Time.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Window.hpp>
#include <Canis/Editor.hpp>
#include <Canis/IOManager.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/ConfigHelper.hpp>

#include <algorithm>

namespace Canis
{
    void App::Run()
    {
        Debug::Log("App Run");
#ifdef _WIN32
        const char *sharedObjectPath = "./libGameCode.dll";
#elif __APPLE__
        const char *sharedObjectPath = "./libGameCode.dylib";
#elif __linux__
        const char *sharedObjectPath = "./libGameCode.so";
#endif

        // find all the meta files and load to asset manager
        std::vector<std::string> paths = FindFilesInFolder("assets", "");

        for (std::string path : paths)
            MetaFileAsset *meta = AssetManager::GetMetaFile(path);
        
        Canis::Init();

        bool editorRuntimeEnabled = false;
#if CANIS_EDITOR
        editorRuntimeEnabled = Canis::GetProjectConfig().editor;
#endif

        // init window
        const int startupWidth = std::max(320, editorRuntimeEnabled ? GetProjectConfig().editorWindowWidth
                                                                     : GetProjectConfig().targetGameWidth);
        const int startupHeight = std::max(240, editorRuntimeEnabled ? GetProjectConfig().editorWindowHeight
                                                                      : GetProjectConfig().targetGameHeight);
        Window window("Canis Beta", startupWidth, startupHeight);
        window.SetClearColor(Color(1.0f));
        window.SetSync(static_cast<Window::Sync>(GetProjectConfig().syncMode));
        
        // get icon
        if (GetProjectConfig().iconUUID == UUID(0))
        {
            GetProjectConfig().iconUUID = AssetManager::GetMetaFile("assets/defaults/textures/engine_icon.png")->uuid;
            SaveProjectConfig();
        }

        window.SetWindowIcon(AssetManager::GetPath(GetProjectConfig().iconUUID));

        Editor editor;
        m_editor = &editor;
#if CANIS_EDITOR
        if (editorRuntimeEnabled)
            editor.Init(&window);
#endif

        RegisterDefaults(editor);

        InputManager inputManager;
        inputManager.SetGameInputWindowID(SDL_GetWindowID((SDL_Window*)window.GetSDLWindow()));

        if (Canis::GetProjectConfig().useFrameLimit)
            Time::Init(Canis::GetProjectConfig().frameLimit + 0.0f);
        else
            Time::Init(100000.0f);
        
#if CANIS_EDITOR
        if (editorRuntimeEnabled)
            Time::SetTargetFPS(Canis::GetProjectConfig().frameLimitEditor + 0.0f);
#endif

        scene.Init(this, &window, &inputManager, "assets/scenes/roll_a_ball.scene");

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);

        // call after all the systems are added
        // and after script from the game lib have been registered
        scene.Load(m_scriptRegistry);

        while (inputManager.Update((void *)&window))
        {
            if (window.IsResized())
            {
                if (editorRuntimeEnabled)
                {
                    GetProjectConfig().editorWindowWidth = window.GetWindowWidth();
                    GetProjectConfig().editorWindowHeight = window.GetWindowHeight();
                }
                else
                {
                    GetProjectConfig().targetGameWidth = window.GetWindowWidth();
                    GetProjectConfig().targetGameHeight = window.GetWindowHeight();
                }
            }

            f32 deltaTime = Time::StartFrame();

            bool runGameTick = true;
#if CANIS_EDITOR
            if (editorRuntimeEnabled)
                runGameTick = (editor.m_mode == EditorMode::PLAY);
#endif

            if (runGameTick)
            {
                Uint64 sceneUpdateStart = SDL_GetTicksNS();
                scene.Update(deltaTime);
                m_sceneUpdateTimeMs = static_cast<float>(SDL_GetTicksNS() - sceneUpdateStart) / 1000000.0f;

                Uint64 gameCodeUpdateStart = SDL_GetTicksNS();
                // call the dynamically loaded function
                GameCodeObjectUpdateFunction(&gameCodeObject, this, deltaTime);
                m_gameCodeUpdateTimeMs = static_cast<float>(SDL_GetTicksNS() - gameCodeUpdateStart) / 1000000.0f;

                m_updateTimeMs = m_sceneUpdateTimeMs + m_gameCodeUpdateTimeMs;
            }
            else
            {
                m_updateTimeMs = 0.0f;
                m_sceneUpdateTimeMs = 0.0f;
                m_gameCodeUpdateTimeMs = 0.0f;
            }
            
            // GameCodeObjectWatchFile(&gameCodeObject, this);

            Uint64 renderStart = SDL_GetTicksNS();
            window.Clear();
#if CANIS_EDITOR
            if (editorRuntimeEnabled)
            {
                editor.Draw(&scene, &window, this, &gameCodeObject, deltaTime);
                inputManager.SetGameInputWindowID(editor.GetGameInputWindowID());
            }
            else
#endif
            {
                scene.Render(deltaTime);
                inputManager.SetGameInputWindowID(SDL_GetWindowID((SDL_Window*)window.GetSDLWindow()));
            }
            window.SwapBuffer();
            m_renderTimeMs = static_cast<float>(SDL_GetTicksNS() - renderStart) / 1000000.0f;

            Time::EndFrame();
        }

        if (editorRuntimeEnabled)
        {
            GetProjectConfig().editorWindowWidth = window.GetWindowWidth();
            GetProjectConfig().editorWindowHeight = window.GetWindowHeight();
        }
        else
        {
            GetProjectConfig().targetGameWidth = window.GetWindowWidth();
            GetProjectConfig().targetGameHeight = window.GetWindowHeight();
        }
        SaveProjectConfig();

        scene.Unload();
        Time::Quit();
        gameCodeObject.GameShutdownFunction((void *)this, gameCodeObject.gameData);
        SDL_UnloadObject(gameCodeObject.sharedObjectHandle);
    }

    void App::RegisterDefaults(Editor& _editor)
    {
        ScriptConf rectTransformConf = {
            .name = "Canis::RectTransform",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                _entity.AddComponent<RectTransform>();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<RectTransform>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<RectTransform>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<RectTransform>() ? (void*)(&_entity.GetComponent<RectTransform>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<RectTransform>())
                {
                    RectTransform& transform = _entity.GetComponent<RectTransform>();

                    YAML::Node comp;
                    comp["active"] = transform.active;
                    comp["position"] = transform.position;
                    comp["size"] = transform.size;
                    comp["scale"] = transform.scale;
                    comp["originOffset"] = transform.originOffset;
                    comp["depth"] = transform.depth;
                    comp["rotation"] = transform.rotation;
                    comp["parent"] = (transform.parent == nullptr) ? Canis::UUID(0) : transform.parent->uuid;
                    // children
                    YAML::Node children = YAML::Node(YAML::NodeType::Sequence);

                    for (Canis::Entity* c : transform.children)
                    {
                        children.push_back(c->uuid);
                    }

                    comp["children"] = children;

                    _node["Canis::RectTransform"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto rectTransform = _node["Canis::RectTransform"])
                {
                    auto &rt = *_entity.AddComponent<RectTransform>();
                    rt.active = rectTransform["active"].as<bool>();
                    //rt.anchor = (Canis::RectAnchor)rectTransform["anchor"].as<int>();
                    rt.position = rectTransform["position"].as<Vector2>();
                    rt.size = rectTransform["size"].as<Vector2>();
                    rt.scale = rectTransform["scale"].as<Vector2>();
                    rt.originOffset = rectTransform["originOffset"].as<Vector2>();
                    rt.depth = rectTransform["depth"].as<float>();
                    rt.rotation = rectTransform["rotation"].as<float>();

                    if (rectTransform["parent"].as<Canis::UUID>(0) != Canis::UUID(0))
                        _entity.scene->GetEntityAfterLoad(rectTransform["parent"].as<Canis::UUID>(0), rt.parent);
                    
                    if (auto children = rectTransform["children"]; children && children.IsSequence())
                    {
                        const std::size_t count = children.size();
                        rt.children.clear();
                        rt.children.resize(count);

                        std::size_t i = 0;
                        for (const auto &e : children)
                        {
                            auto uuid = e.as<Canis::UUID>(Canis::UUID(0));
                            _entity.scene->GetEntityAfterLoad(uuid, rt.children[i++]);
                        }
                    }
                    
                        //rt.scaleWithScreen = (ScaleWithScreen)rectTransform["scaleWithScreen"].as<int>(0);
                    if (_callCreate)
                        rt.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                RectTransform* transform = nullptr;
                if (_entity.HasComponent<RectTransform>() && ((transform = &_entity.GetComponent<RectTransform>()), true))
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
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                // TODO: require a RectTransform component
                Sprite2D& sprite = *_entity.AddComponent<Sprite2D>();
                sprite.textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                //sprite->size.x = sprite->textureHandle.texture.width;
                //sprite->size.y = sprite->textureHandle.texture.height;
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Sprite2D>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Sprite2D>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Sprite2D>() ? (void*)(&_entity.GetComponent<Sprite2D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Sprite2D>())
                {
                    Sprite2D& sprite = _entity.GetComponent<Sprite2D>();

                    YAML::Node comp;
                    comp["color"] = sprite.color;
                    comp["uv"] = sprite.uv;
                    comp["flipX"] = sprite.flipX;
                    comp["flipY"] = sprite.flipY;

                    YAML::Node textureAsset;
                    textureAsset["uuid"] = (uint64_t)AssetManager::GetMetaFile(AssetManager::Get<TextureAsset>(sprite.textureHandle.id)->GetPath())->uuid;
                    
                    comp["TextureAsset"] = textureAsset;
                    _node["Canis::Sprite2D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Sprite2D"])
                {
                    auto &sprite = *_entity.AddComponent<Sprite2D>();
                    sprite.color = comp["color"].as<Vector4>();
                    sprite.uv = comp["uv"].as<Vector4>();
                    sprite.flipX = comp["flipX"].as<bool>(false);
                    sprite.flipY = comp["flipY"].as<bool>(false);
                    if (auto textureAsset = comp["TextureAsset"])
                    {
                        UUID uuid = textureAsset["uuid"].as<uint64_t>();
                        std::string path = AssetManager::GetPath(uuid);
                        sprite.textureHandle = AssetManager::GetTextureHandle(path);
                    }
                    //sprite.textureHandle = sprite2DComponent["TextureHandle"].as<TextureHandle>();//AssetManager::GetTextureHandle(sprite2DComponent["textureHandle"].as<std::string>());
                    if (_callCreate)
                        sprite.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Sprite2D* sprite = nullptr;
                if (_entity.HasComponent<Sprite2D>() && ((sprite = &_entity.GetComponent<Sprite2D>()), true))
                {
                    // textureHandle
                    ImGui::ColorEdit4("color", &sprite->color.r);
                    ImGui::InputFloat4("uv", &sprite->uv.x, "%.3f");

                    bool updateUV = false;
                    
                    if (ImGui::Checkbox("flipX", &sprite->flipX))
                        updateUV = true;
                    if (ImGui::Checkbox("flipY", &sprite->flipY))
                        updateUV = true;
                    
                    if (updateUV)
                    {
                        if (SpriteAnimation* animation = _entity.HasComponent<SpriteAnimation>() ? &_entity.GetComponent<SpriteAnimation>() : nullptr)
                        {
                            if (SpriteAnimationAsset* animationAsset = AssetManager::Get<SpriteAnimationAsset>(animation->id))
                            {
                                sprite->GetSpriteFromTextureAtlas(
                                    animationAsset->frames[animation->index].offsetX,
                                    animationAsset->frames[animation->index].offsetY,
                                    animationAsset->frames[animation->index].row,
                                    animationAsset->frames[animation->index].col,
                                    animationAsset->frames[animation->index].width,
                                    animationAsset->frames[animation->index].height);
                            }
                        }
                        else
                        {
                            sprite->GetSpriteFromTextureAtlas(0, 0, 0, 0, sprite->textureHandle.texture.width, sprite->textureHandle.texture.height);
                        }
                    }

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

        ScriptConf textConf = {
            .name = "Canis::Text",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {

                _entity.AddComponent<RectTransform>();

                Text& text = *_entity.AddComponent<Text>();
                text.assetId = AssetManager::LoadText("assets/fonts/Antonio-Bold.ttf", 32);
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Text>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Text>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Text>() ? (void*)(&_entity.GetComponent<Text>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Text>())
                {
                    Text& text = _entity.GetComponent<Text>();
                    YAML::Node comp;
                    comp["text"] = text.text;
                    comp["color"] = text.color;
                    comp["alignment"] = text.alignment;
                    comp["horizontalBoundary"] = text.horizontalBoundary;

                    if (text.assetId > -1)
                    {
                        if (TextAsset* textAsset = AssetManager::GetText(text.assetId))
                        {
                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(textAsset->GetPath()))
                            {
                                YAML::Node fontAsset;
                                fontAsset["uuid"] = (uint64_t)meta->uuid;
                                fontAsset["fontSize"] = textAsset->GetFontSize();
                                comp["FontAsset"] = fontAsset;
                            }
                        }
                    }

                    _node["Canis::Text"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Text"])
                {
                    Text& text = *_entity.AddComponent<Text>();
                    text.text = comp["text"].as<std::string>("");
                    text.color = comp["color"].as<Vector4>(Color(1.0f));
                    text.alignment = comp["alignment"].as<unsigned int>(Canis::TextAlignment::LEFT);
                    text.horizontalBoundary = comp["horizontalBoundary"].as<unsigned int>(Canis::TextBoundary::TB_OVERFLOW);
                    text._status = BIT::ONE;

                    if (auto fontAsset = comp["FontAsset"])
                    {
                        UUID uuid = fontAsset["uuid"].as<uint64_t>();
                        std::string path = AssetManager::GetPath(uuid);
                        const unsigned int fontSize = fontAsset["fontSize"].as<unsigned int>(32u);
                        text.assetId = AssetManager::LoadText(path, fontSize);
                    }

                    if (_callCreate)
                        text.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Text* text = nullptr;
                if (_entity.HasComponent<Text>() && ((text = &_entity.GetComponent<Text>()), true))
                {
                    static const char *alignmentLabels[] = {"Left", "Right", "Center"};
                    static const char *horizontalBoundaryLabels[] = {"Overflow", "Wrap"};

                    if (ImGui::InputText("text", &text->text))
                    {
                        text->_status |= BIT::ONE;
                    }

                    ImGui::ColorEdit4("color", &text->color.r);

                    int alignment = (int)text->alignment;
                    if (ImGui::Combo("alignment", &alignment, alignmentLabels, IM_ARRAYSIZE(alignmentLabels)))
                    {
                        text->alignment = (unsigned int)alignment;
                        text->_status |= BIT::ONE;
                    }

                    int boundary = (int)text->horizontalBoundary;
                    if (ImGui::Combo("horizontalBoundary", &boundary, horizontalBoundaryLabels, IM_ARRAYSIZE(horizontalBoundaryLabels)))
                    {
                        text->horizontalBoundary = (unsigned int)boundary;
                        text->_status |= BIT::ONE;
                    }

                    if (text->assetId > -1)
                    {
                        ImGui::Text("font");
                        ImGui::SameLine();
                        TextAsset* textAsset = AssetManager::GetText(text->assetId);

                        if (textAsset != nullptr)
                        {
                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(textAsset->GetPath()))
                            {
                                ImGui::Button(meta->name.c_str(), ImVec2(150, 0));
                            }
                            else
                            {
                                ImGui::Button("Missing Font", ImVec2(150, 0));
                            }
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
                            {
                                const AssetDragData dropped = *static_cast<const AssetDragData*>(payload->Data);
                                std::string path = AssetManager::GetPath(dropped.uuid);
                                const unsigned int fontSize = (textAsset == nullptr) ? 32u : textAsset->GetFontSize();
                                text->assetId = AssetManager::LoadText(path, fontSize);
                                text->_status |= BIT::ONE;
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                }
            },
        };

        RegisterScript(textConf);

        ScriptConf camera2DConf = {
            .name = "Canis::Camera2D",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                Camera2D& camera = *_entity.AddComponent<Camera2D>();
                camera.Create();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Camera2D>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Camera2D>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Camera2D>() ? (void*)(&_entity.GetComponent<Camera2D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Camera2D>())
                {
                    Camera2D& camera = _entity.GetComponent<Camera2D>();

                    YAML::Node comp;
                    comp["position"] = camera.GetPosition();
                    comp["scale"] = camera.GetScale();

                    _node["Canis::Camera2D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto camera2DComponent = _node["Canis::Camera2D"])
                {
                    Camera2D& camera = *_entity.AddComponent<Camera2D>();
                    camera.SetPosition(camera2DComponent["position"].as<Vector2>(camera.GetPosition()));
                    camera.SetScale(camera2DComponent["scale"].as<float>(camera.GetScale()));
                    if (_callCreate)
                        camera.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Camera2D* camera = nullptr;
                if (_entity.HasComponent<Camera2D>() && ((camera = &_entity.GetComponent<Camera2D>()), true))
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

        ScriptConf transform3DConf = {
            .name = "Canis::Transform3D",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void { _entity.AddComponent<Transform3D>(); },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Transform3D>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Transform3D>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Transform3D>() ? (void*)(&_entity.GetComponent<Transform3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Transform3D>())
                {
                    Transform3D& transform = _entity.GetComponent<Transform3D>();
                    YAML::Node comp;
                    comp["active"] = transform.active;
                    comp["position"] = transform.position;
                    comp["rotation"] = transform.rotation;
                    comp["scale"] = transform.scale;
                    comp["parent"] = (transform.parent == nullptr) ? Canis::UUID(0) : transform.parent->uuid;

                    YAML::Node children = YAML::Node(YAML::NodeType::Sequence);
                    for (Canis::Entity* child : transform.children)
                    {
                        children.push_back(child ? child->uuid : Canis::UUID(0));
                    }
                    comp["children"] = children;

                    _node["Canis::Transform3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Transform3D"])
                {
                    auto &transform = *_entity.AddComponent<Transform3D>();
                    transform.active = comp["active"].as<bool>(true);
                    transform.position = comp["position"].as<Vector3>(Vector3(0.0f));
                    transform.rotation = comp["rotation"].as<Vector3>(Vector3(0.0f));
                    transform.scale = comp["scale"].as<Vector3>(Vector3(1.0f));

                    if (comp["parent"].as<Canis::UUID>(0) != Canis::UUID(0))
                        _entity.scene->GetEntityAfterLoad(comp["parent"].as<Canis::UUID>(0), transform.parent);

                    if (auto children = comp["children"]; children && children.IsSequence())
                    {
                        const std::size_t count = children.size();
                        transform.children.clear();
                        transform.children.resize(count);

                        std::size_t i = 0;
                        for (const auto &entry : children)
                        {
                            auto uuid = entry.as<Canis::UUID>(Canis::UUID(0));
                            _entity.scene->GetEntityAfterLoad(uuid, transform.children[i++]);
                        }
                    }

                    if (_callCreate)
                        transform.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Transform3D* transform = nullptr;
                if (_entity.HasComponent<Transform3D>() && ((transform = &_entity.GetComponent<Transform3D>()), true))
                {
                    ImGui::InputFloat3("position", &transform->position.x, "%.3f");

                    Vector3 degrees = transform->rotation * RAD2DEG;
                    if (ImGui::InputFloat3("rotation", &degrees.x, "%.3f"))
                    {
                        transform->rotation = degrees * DEG2RAD;
                    }

                    ImGui::InputFloat3("scale", &transform->scale.x, "%.3f");

                    if (transform->parent != nullptr)
                    {
                        ImGui::Text("parent: %s", transform->parent->name.c_str());
                        if (ImGui::Button("Unparent##Transform3D"))
                            transform->Unparent();
                    }
                    else
                    {
                        ImGui::Text("parent: [none]");
                    }
                }
            },
        };

        RegisterScript(transform3DConf);

        ScriptConf rigidbody3DConf = {
            .name = "Canis::Rigidbody3D",
            .Construct = nullptr,
            .Add = [this](Entity &_entity) -> void {
                _entity.AddComponent<Transform3D>();

                if (!_entity.HasComponent<BoxCollider3D>()
                    && !_entity.HasComponent<SphereCollider3D>()
                    && !_entity.HasComponent<CapsuleCollider3D>())
                {
                    _entity.AddComponent<BoxCollider3D>();
                }

                _entity.AddComponent<Rigidbody3D>();
            },
            .Has = [this](Entity &_entity) -> bool { return _entity.HasComponent<Rigidbody3D>(); },
            .Remove = [this](Entity &_entity) -> void { _entity.RemoveComponent<Rigidbody3D>(); },
            .Get = [this](Entity &_entity) -> void* { return _entity.HasComponent<Rigidbody3D>() ? (void*)(&_entity.GetComponent<Rigidbody3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (Rigidbody3D *rigidbody = _entity.HasComponent<Rigidbody3D>() ? &_entity.GetComponent<Rigidbody3D>() : nullptr)
                {
                    YAML::Node comp;
                    comp["active"] = rigidbody->active;
                    comp["motionType"] = rigidbody->motionType;
                    comp["mass"] = rigidbody->mass;
                    comp["friction"] = rigidbody->friction;
                    comp["restitution"] = rigidbody->restitution;
                    comp["linearDamping"] = rigidbody->linearDamping;
                    comp["angularDamping"] = rigidbody->angularDamping;
                    comp["useGravity"] = rigidbody->useGravity;
                    comp["isSensor"] = rigidbody->isSensor;
                    comp["allowSleeping"] = rigidbody->allowSleeping;
                    comp["lockRotationX"] = rigidbody->lockRotationX;
                    comp["lockRotationY"] = rigidbody->lockRotationY;
                    comp["lockRotationZ"] = rigidbody->lockRotationZ;
                    comp["linearVelocity"] = rigidbody->linearVelocity;
                    comp["angularVelocity"] = rigidbody->angularVelocity;
                    _node["Canis::Rigidbody3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Rigidbody3D"])
                {
                    auto &rigidbody = *_entity.AddComponent<Rigidbody3D>();
                    rigidbody.active = comp["active"].as<bool>(true);
                    rigidbody.motionType = comp["motionType"].as<int>(Rigidbody3DMotionType::DYNAMIC);
                    rigidbody.mass = comp["mass"].as<float>(1.0f);
                    rigidbody.friction = comp["friction"].as<float>(0.2f);
                    rigidbody.restitution = comp["restitution"].as<float>(0.0f);
                    rigidbody.linearDamping = comp["linearDamping"].as<float>(0.05f);
                    rigidbody.angularDamping = comp["angularDamping"].as<float>(0.05f);
                    rigidbody.useGravity = comp["useGravity"].as<bool>(true);
                    rigidbody.isSensor = comp["isSensor"].as<bool>(false);
                    rigidbody.allowSleeping = comp["allowSleeping"].as<bool>(true);
                    rigidbody.lockRotationX = comp["lockRotationX"].as<bool>(false);
                    rigidbody.lockRotationY = comp["lockRotationY"].as<bool>(false);
                    rigidbody.lockRotationZ = comp["lockRotationZ"].as<bool>(false);
                    rigidbody.linearVelocity = comp["linearVelocity"].as<Vector3>(Vector3(0.0f));
                    rigidbody.angularVelocity = comp["angularVelocity"].as<Vector3>(Vector3(0.0f));
                    if (_callCreate)
                        rigidbody.Create();
                }
            },
            .DrawInspector = [this](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void {
                (void)_editor;
                Rigidbody3D *rigidbody = _entity.HasComponent<Rigidbody3D>() ? &_entity.GetComponent<Rigidbody3D>() : nullptr;
                if (rigidbody == nullptr)
                    return;

                const char *motionTypeLabels[] = {"Static", "Kinematic", "Dynamic"};
                if (rigidbody->motionType < Rigidbody3DMotionType::STATIC
                    || rigidbody->motionType > Rigidbody3DMotionType::DYNAMIC)
                {
                    rigidbody->motionType = Rigidbody3DMotionType::DYNAMIC;
                }

                ImGui::Checkbox(("active##" + _conf.name).c_str(), &rigidbody->active);
                ImGui::Combo(("motionType##" + _conf.name).c_str(), &rigidbody->motionType, motionTypeLabels, IM_ARRAYSIZE(motionTypeLabels));
                ImGui::InputFloat(("mass##" + _conf.name).c_str(), &rigidbody->mass);
                ImGui::InputFloat(("friction##" + _conf.name).c_str(), &rigidbody->friction);
                ImGui::InputFloat(("restitution##" + _conf.name).c_str(), &rigidbody->restitution);
                ImGui::InputFloat(("linearDamping##" + _conf.name).c_str(), &rigidbody->linearDamping);
                ImGui::InputFloat(("angularDamping##" + _conf.name).c_str(), &rigidbody->angularDamping);
                ImGui::Checkbox(("useGravity##" + _conf.name).c_str(), &rigidbody->useGravity);
                ImGui::Checkbox(("isSensor##" + _conf.name).c_str(), &rigidbody->isSensor);
                ImGui::Checkbox(("allowSleeping##" + _conf.name).c_str(), &rigidbody->allowSleeping);
                ImGui::Checkbox(("lockRotationX##" + _conf.name).c_str(), &rigidbody->lockRotationX);
                ImGui::Checkbox(("lockRotationY##" + _conf.name).c_str(), &rigidbody->lockRotationY);
                ImGui::Checkbox(("lockRotationZ##" + _conf.name).c_str(), &rigidbody->lockRotationZ);
                ImGui::InputFloat3(("linearVelocity##" + _conf.name).c_str(), &rigidbody->linearVelocity.x, "%.3f");
                ImGui::InputFloat3(("angularVelocity##" + _conf.name).c_str(), &rigidbody->angularVelocity.x, "%.3f");
            },
        };

        RegisterScript(rigidbody3DConf);

        ScriptConf boxCollider3DConf = {
            .name = "Canis::BoxCollider3D",
            .Construct = nullptr,
            .Add = [this](Entity &_entity) -> void {
                _entity.AddComponent<Transform3D>();

                _entity.RemoveComponent<SphereCollider3D>();
                _entity.RemoveComponent<CapsuleCollider3D>();
                _entity.AddComponent<BoxCollider3D>();
            },
            .Has = [this](Entity &_entity) -> bool { return _entity.HasComponent<BoxCollider3D>(); },
            .Remove = [this](Entity &_entity) -> void { _entity.RemoveComponent<BoxCollider3D>(); },
            .Get = [this](Entity &_entity) -> void* { return _entity.HasComponent<BoxCollider3D>() ? (void*)(&_entity.GetComponent<BoxCollider3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (BoxCollider3D *boxCollider = _entity.HasComponent<BoxCollider3D>() ? &_entity.GetComponent<BoxCollider3D>() : nullptr)
                {
                    YAML::Node comp;
                    comp["active"] = boxCollider->active;
                    comp["size"] = boxCollider->size;
                    _node["Canis::BoxCollider3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::BoxCollider3D"])
                {
                    auto &boxCollider = *_entity.AddComponent<BoxCollider3D>();
                    boxCollider.active = comp["active"].as<bool>(true);
                    boxCollider.size = comp["size"].as<Vector3>(Vector3(1.0f));
                    if (_callCreate)
                        boxCollider.Create();
                }
            },
            .DrawInspector = [this](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void {
                (void)_editor;
                BoxCollider3D *boxCollider = _entity.HasComponent<BoxCollider3D>() ? &_entity.GetComponent<BoxCollider3D>() : nullptr;
                if (boxCollider == nullptr)
                    return;

                ImGui::Checkbox(("active##" + _conf.name).c_str(), &boxCollider->active);
                ImGui::InputFloat3(("size##" + _conf.name).c_str(), &boxCollider->size.x, "%.3f");
            },
        };

        RegisterScript(boxCollider3DConf);

        ScriptConf sphereCollider3DConf = {
            .name = "Canis::SphereCollider3D",
            .Construct = nullptr,
            .Add = [this](Entity &_entity) -> void {
                if (!_entity.HasComponent<Transform3D>())
                    _entity.AddComponent<Transform3D>();

                _entity.RemoveComponent<BoxCollider3D>();
                _entity.RemoveComponent<CapsuleCollider3D>();
                _entity.AddComponent<SphereCollider3D>();
            },
            .Has = [this](Entity &_entity) -> bool { return _entity.HasComponent<SphereCollider3D>(); },
            .Remove = [this](Entity &_entity) -> void { _entity.RemoveComponent<SphereCollider3D>(); },
            .Get = [this](Entity &_entity) -> void* { return _entity.HasComponent<SphereCollider3D>() ? (void*)(&_entity.GetComponent<SphereCollider3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (SphereCollider3D *sphereCollider = _entity.HasComponent<SphereCollider3D>() ? &_entity.GetComponent<SphereCollider3D>() : nullptr)
                {
                    YAML::Node comp;
                    comp["active"] = sphereCollider->active;
                    comp["radius"] = sphereCollider->radius;
                    _node["Canis::SphereCollider3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::SphereCollider3D"])
                {
                    auto &sphereCollider = *_entity.AddComponent<SphereCollider3D>();
                    sphereCollider.active = comp["active"].as<bool>(true);
                    sphereCollider.radius = comp["radius"].as<float>(0.5f);
                    if (_callCreate)
                        sphereCollider.Create();
                }
            },
            .DrawInspector = [this](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void {
                (void)_editor;
                SphereCollider3D *sphereCollider = _entity.HasComponent<SphereCollider3D>() ? &_entity.GetComponent<SphereCollider3D>() : nullptr;
                if (sphereCollider == nullptr)
                    return;

                ImGui::Checkbox(("active##" + _conf.name).c_str(), &sphereCollider->active);
                ImGui::InputFloat(("radius##" + _conf.name).c_str(), &sphereCollider->radius);
            },
        };

        RegisterScript(sphereCollider3DConf);

        ScriptConf capsuleCollider3DConf = {
            .name = "Canis::CapsuleCollider3D",
            .Construct = nullptr,
            .Add = [this](Entity &_entity) -> void {
                if (!_entity.HasComponent<Transform3D>())
                    _entity.AddComponent<Transform3D>();

                _entity.RemoveComponent<BoxCollider3D>();
                _entity.RemoveComponent<SphereCollider3D>();
                _entity.AddComponent<CapsuleCollider3D>();
            },
            .Has = [this](Entity &_entity) -> bool { return _entity.HasComponent<CapsuleCollider3D>(); },
            .Remove = [this](Entity &_entity) -> void { _entity.RemoveComponent<CapsuleCollider3D>(); },
            .Get = [this](Entity &_entity) -> void* { return _entity.HasComponent<CapsuleCollider3D>() ? (void*)(&_entity.GetComponent<CapsuleCollider3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CapsuleCollider3D *capsuleCollider = _entity.HasComponent<CapsuleCollider3D>() ? &_entity.GetComponent<CapsuleCollider3D>() : nullptr)
                {
                    YAML::Node comp;
                    comp["active"] = capsuleCollider->active;
                    comp["halfHeight"] = capsuleCollider->halfHeight;
                    comp["radius"] = capsuleCollider->radius;
                    _node["Canis::CapsuleCollider3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::CapsuleCollider3D"])
                {
                    auto &capsuleCollider = *_entity.AddComponent<CapsuleCollider3D>();
                    capsuleCollider.active = comp["active"].as<bool>(true);
                    capsuleCollider.halfHeight = comp["halfHeight"].as<float>(0.5f);
                    capsuleCollider.radius = comp["radius"].as<float>(0.25f);
                    if (_callCreate)
                        capsuleCollider.Create();
                }
            },
            .DrawInspector = [this](Editor &_editor, Entity &_entity, const ScriptConf &_conf) -> void {
                (void)_editor;
                CapsuleCollider3D *capsuleCollider = _entity.HasComponent<CapsuleCollider3D>() ? &_entity.GetComponent<CapsuleCollider3D>() : nullptr;
                if (capsuleCollider == nullptr)
                    return;

                ImGui::Checkbox(("active##" + _conf.name).c_str(), &capsuleCollider->active);
                ImGui::InputFloat(("halfHeight##" + _conf.name).c_str(), &capsuleCollider->halfHeight);
                ImGui::InputFloat(("radius##" + _conf.name).c_str(), &capsuleCollider->radius);
            },
        };

        RegisterScript(capsuleCollider3DConf);

        ScriptConf camera3DConf = {
            .name = "Canis::Camera3D",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Transform3D>())
                    _entity.AddComponent<Transform3D>();

                _entity.AddComponent<Camera3D>();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Camera3D>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Camera3D>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Camera3D>() ? (void*)(&_entity.GetComponent<Camera3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Camera3D>())
                {
                    Camera3D& camera = _entity.GetComponent<Camera3D>();
                    YAML::Node comp;
                    comp["primary"] = camera.primary;
                    comp["fovDegrees"] = camera.fovDegrees;
                    comp["nearClip"] = camera.nearClip;
                    comp["farClip"] = camera.farClip;
                    _node["Canis::Camera3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Camera3D"])
                {
                    auto &camera = *_entity.AddComponent<Camera3D>();
                    camera.primary = comp["primary"].as<bool>(true);
                    camera.fovDegrees = comp["fovDegrees"].as<float>(60.0f);
                    camera.nearClip = comp["nearClip"].as<float>(0.1f);
                    camera.farClip = comp["farClip"].as<float>(1000.0f);
                    if (_callCreate)
                        camera.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Camera3D* camera = nullptr;
                if (_entity.HasComponent<Camera3D>() && ((camera = &_entity.GetComponent<Camera3D>()), true))
                {
                    ImGui::Checkbox("primary", &camera->primary);
                    ImGui::InputFloat("fovDegrees", &camera->fovDegrees);
                    ImGui::InputFloat("nearClip", &camera->nearClip);
                    ImGui::InputFloat("farClip", &camera->farClip);
                }
            },
        };

        RegisterScript(camera3DConf);

        ScriptConf directionalLightConf = {
            .name = "Canis::DirectionalLight",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                _entity.AddComponent<DirectionalLight>();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<DirectionalLight>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<DirectionalLight>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<DirectionalLight>() ? (void*)(&_entity.GetComponent<DirectionalLight>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<DirectionalLight>())
                {
                    DirectionalLight& light = _entity.GetComponent<DirectionalLight>();
                    YAML::Node comp;
                    comp["enabled"] = light.enabled;
                    comp["color"] = light.color;
                    comp["intensity"] = light.intensity;
                    comp["direction"] = light.direction;
                    _node["Canis::DirectionalLight"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::DirectionalLight"])
                {
                    auto &light = *_entity.AddComponent<DirectionalLight>();
                    light.enabled = comp["enabled"].as<bool>(true);
                    light.color = comp["color"].as<Vector4>(Color(1.0f));
                    light.intensity = comp["intensity"].as<float>(1.0f);
                    light.direction = comp["direction"].as<Vector3>(Vector3(-0.4f, -1.0f, -0.25f));
                    if (_callCreate)
                        light.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                DirectionalLight* light = nullptr;
                if (_entity.HasComponent<DirectionalLight>() && ((light = &_entity.GetComponent<DirectionalLight>()), true))
                {
                    ImGui::Checkbox("enabled", &light->enabled);
                    ImGui::ColorEdit3("color", &light->color.r);
                    ImGui::InputFloat("intensity", &light->intensity);
                    ImGui::InputFloat3("direction", &light->direction.x, "%.3f");
                }
            },
        };

        RegisterScript(directionalLightConf);

        ScriptConf pointLightConf = {
            .name = "Canis::PointLight",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Transform3D>())
                    _entity.AddComponent<Transform3D>();

                _entity.AddComponent<PointLight>();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<PointLight>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<PointLight>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<PointLight>() ? (void*)(&_entity.GetComponent<PointLight>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<PointLight>())
                {
                    PointLight& light = _entity.GetComponent<PointLight>();
                    YAML::Node comp;
                    comp["enabled"] = light.enabled;
                    comp["color"] = light.color;
                    comp["intensity"] = light.intensity;
                    comp["range"] = light.range;
                    _node["Canis::PointLight"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::PointLight"])
                {
                    auto &light = *_entity.AddComponent<PointLight>();
                    light.enabled = comp["enabled"].as<bool>(true);
                    light.color = comp["color"].as<Vector4>(Color(1.0f));
                    light.intensity = comp["intensity"].as<float>(1.2f);
                    light.range = comp["range"].as<float>(12.0f);
                    if (_callCreate)
                        light.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                PointLight* light = nullptr;
                if (_entity.HasComponent<PointLight>() && ((light = &_entity.GetComponent<PointLight>()), true))
                {
                    ImGui::Checkbox("enabled", &light->enabled);
                    ImGui::ColorEdit3("color", &light->color.r);
                    ImGui::InputFloat("intensity", &light->intensity);
                    ImGui::InputFloat("range", &light->range);
                }
            },
        };

        RegisterScript(pointLightConf);

        ScriptConf materialConf = {
            .name = "Canis::Material",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Model3D>())
                {
                    if (!_entity.HasComponent<Transform3D>())
                        _entity.AddComponent<Transform3D>();

                    Model3D* model = _entity.AddComponent<Model3D>();
                    model->modelId = AssetManager::LoadModel("assets/models/dq.gltf");
                }

                Material* material = _entity.AddComponent<Material>();
                material->materialId = AssetManager::LoadMaterial("assets/defaults/materials/default.material");
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Material>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<Material>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Material>() ? (void*)(&_entity.GetComponent<Material>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Material>())
                {
                    Material& material = _entity.GetComponent<Material>();
                    YAML::Node comp;
                    comp["color"] = material.color;

                    if (material.materialId > -1)
                    {
                        const std::string materialPath = AssetManager::GetPath(material.materialId);
                        if (materialPath.rfind("Path was not found", 0) != 0)
                        {
                            YAML::Node materialAssetNode;
                            materialAssetNode["path"] = materialPath;

                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(materialPath))
                                materialAssetNode["uuid"] = (uint64_t)meta->uuid;

                            comp["MaterialAsset"] = materialAssetNode;
                        }
                    }

                    YAML::Node slotAssets = YAML::Node(YAML::NodeType::Sequence);
                    for (i32 slotMaterialId : material.materialIds)
                    {
                        if (slotMaterialId < 0)
                        {
                            slotAssets.push_back(YAML::Node());
                            continue;
                        }

                        const std::string slotPath = AssetManager::GetPath(slotMaterialId);
                        if (slotPath.rfind("Path was not found", 0) == 0)
                        {
                            slotAssets.push_back(YAML::Node());
                            continue;
                        }

                        YAML::Node slotAssetNode;
                        slotAssetNode["path"] = slotPath;
                        if (MetaFileAsset* meta = AssetManager::GetMetaFile(slotPath))
                            slotAssetNode["uuid"] = (uint64_t)meta->uuid;
                        slotAssets.push_back(slotAssetNode);
                    }

                    if (!slotAssets.IsNull() && slotAssets.size() > 0)
                        comp["MaterialSlots"] = slotAssets;

                    _node["Canis::Material"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Material"])
                {
                    auto &material = *_entity.AddComponent<Material>();
                    material.color = comp["color"].as<Vector4>(Color(1.0f));

                    std::string path = "";
                    if (auto materialAsset = comp["MaterialAsset"])
                    {
                        if (auto uuidNode = materialAsset["uuid"])
                        {
                            UUID uuid = uuidNode.as<uint64_t>(0);
                            path = AssetManager::GetPath(uuid);
                            if (path.rfind("Path was not found", 0) == 0)
                                path.clear();
                        }

                        if (path.empty())
                            path = materialAsset["path"].as<std::string>("");
                    }

                    if (!path.empty())
                        material.materialId = AssetManager::LoadMaterial(path);

                    material.materialIds.clear();
                    if (auto slotAssets = comp["MaterialSlots"]; slotAssets && slotAssets.IsSequence())
                    {
                        material.materialIds.resize(slotAssets.size(), -1);
                        for (size_t i = 0; i < slotAssets.size(); ++i)
                        {
                            const YAML::Node slotNode = slotAssets[i];
                            if (!slotNode || slotNode.IsNull())
                                continue;

                            std::string slotPath = "";
                            if (auto uuidNode = slotNode["uuid"])
                            {
                                UUID uuid = uuidNode.as<uint64_t>(0);
                                slotPath = AssetManager::GetPath(uuid);
                                if (slotPath.rfind("Path was not found", 0) == 0)
                                    slotPath.clear();
                            }

                            if (slotPath.empty())
                                slotPath = slotNode["path"].as<std::string>("");

                            if (!slotPath.empty())
                                material.materialIds[i] = AssetManager::LoadMaterial(slotPath);
                        }
                    }

                    if (_callCreate)
                        material.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Material* material = nullptr;
                if (_entity.HasComponent<Material>() && ((material = &_entity.GetComponent<Material>()), true))
                {
                    auto getMaterialLabel = [](i32 _materialId) -> std::string
                    {
                        if (_materialId < 0)
                            return "[ empty ]";

                        std::string path = AssetManager::GetPath(_materialId);
                        if (path.rfind("Path was not found", 0) == 0)
                            return "[ missing ]";

                        if (MetaFileAsset* meta = AssetManager::GetMetaFile(path))
                            return meta->name;

                        return path;
                    };

                    auto handleMaterialDrop = [](i32 &_materialId) -> void
                    {
                        if (!ImGui::BeginDragDropTarget())
                            return;

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
                        {
                            const AssetDragData dropped = *static_cast<const AssetDragData*>(payload->Data);
                            std::string path = std::string(dropped.path);
                            if (path.empty() || !FileExists(path.c_str()))
                                path = AssetManager::GetPath(dropped.uuid);

                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(path))
                            {
                                if (meta->type == MetaFileAsset::FileType::MATERIAL)
                                    _materialId = AssetManager::LoadMaterial(path);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    };

                    ImGui::ColorEdit4("material color", &material->color.r);

                    std::string materialLabel = getMaterialLabel(material->materialId);

                    ImGui::Text("material");
                    ImGui::SameLine();
                    ImGui::Button(materialLabel.c_str(), ImVec2(150, 0));
                    handleMaterialDrop(material->materialId);

                    Model3D* model3D = _entity.HasComponent<Model3D>() ? &_entity.GetComponent<Model3D>() : nullptr;
                    ModelAsset* modelAsset = nullptr;
                    if (model3D != nullptr && model3D->modelId >= 0)
                        modelAsset = AssetManager::GetModel(model3D->modelId);

                    const i32 slotCount = (modelAsset != nullptr) ? modelAsset->GetMaterialSlotCount() : 0;
                    ImGui::Text("material slots: %d", slotCount);
                    if (slotCount > 0)
                    {
                        material->materialIds.resize(static_cast<size_t>(slotCount), -1);

                        for (i32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
                        {
                            const std::string slotName = modelAsset->GetMaterialSlotName(slotIndex);
                            const std::string slotLabel = slotName.empty()
                                ? ("slot " + std::to_string(slotIndex))
                                : ("slot " + std::to_string(slotIndex) + " (" + slotName + ")");
                            ImGui::Text("%s", slotLabel.c_str());
                            ImGui::SameLine();

                            std::string buttonLabel = getMaterialLabel(material->materialIds[static_cast<size_t>(slotIndex)]);
                            buttonLabel += "##material_slot_" + std::to_string(slotIndex);
                            ImGui::Button(buttonLabel.c_str(), ImVec2(180, 0));
                            handleMaterialDrop(material->materialIds[static_cast<size_t>(slotIndex)]);
                        }
                    }
                }
            },
        };

        RegisterScript(materialConf);

        ScriptConf model3DConf = {
            .name = "Canis::Model3D",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Transform3D>())
                    _entity.AddComponent<Transform3D>();

                Model3D* model = _entity.AddComponent<Model3D>();
                model->modelId = AssetManager::LoadModel("assets/models/dq.gltf");

                if (!_entity.HasComponent<Material>())
                {
                    Material* material = _entity.AddComponent<Material>();
                    material->materialId = AssetManager::LoadMaterial("assets/defaults/materials/default.material");
                }
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<Model3D>(); },
            .Remove = [this](Entity& _entity) -> void {
                _entity.RemoveComponent<ModelAnimation3D>();
                _entity.RemoveComponent<Model3D>();
            },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<Model3D>() ? (void*)(&_entity.GetComponent<Model3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<Model3D>())
                {
                    Model3D& model = _entity.GetComponent<Model3D>();
                    YAML::Node comp;
                    comp["color"] = model.color;

                    if (model.modelId > -1)
                    {
                        if (ModelAsset* modelAsset = AssetManager::GetModel(model.modelId))
                        {
                            YAML::Node modelAssetNode;
                            modelAssetNode["path"] = modelAsset->GetPath();

                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(modelAsset->GetPath()))
                                modelAssetNode["uuid"] = (uint64_t)meta->uuid;

                            comp["ModelAsset"] = modelAssetNode;
                        }
                    }

                    _node["Canis::Model3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::Model3D"])
                {
                    auto &model = *_entity.AddComponent<Model3D>();
                    model.color = comp["color"].as<Vector4>(Color(1.0f));

                    std::string path = "";
                    if (auto modelAsset = comp["ModelAsset"])
                    {
                        if (auto uuidNode = modelAsset["uuid"])
                        {
                            UUID uuid = uuidNode.as<uint64_t>(0);
                            path = AssetManager::GetPath(uuid);
                            if (path.rfind("Path was not found", 0) == 0)
                                path.clear();
                        }

                        if (path.empty())
                            path = modelAsset["path"].as<std::string>("");
                    }

                    if (!path.empty())
                        model.modelId = AssetManager::LoadModel(path);

                    // Backward compatibility: migrate legacy animation fields on Canis::Model3D.
                    //if (!_node["Canis::ModelAnimation3D"])
                    //{
                    //    const bool hasLegacyAnimation =
                    //        comp["playAnimation"].IsDefined() ||
                    //        comp["loop"].IsDefined() ||
                    //        comp["animationSpeed"].IsDefined() ||
                    //        comp["animationTime"].IsDefined() ||
                    //        comp["animationIndex"].IsDefined();
//
                    //    if (hasLegacyAnimation && !_entity.HasComponent<ModelAnimation3D>())
                    //    {
                    //        auto &animation = *_entity.AddComponent<ModelAnimation3D>();
                    //        animation.playAnimation = comp["playAnimation"].as<bool>(true);
                    //        animation.loop = comp["loop"].as<bool>(true);
                    //        animation.animationSpeed = comp["animationSpeed"].as<float>(1.0f);
                    //        animation.animationTime = comp["animationTime"].as<float>(0.0f);
                    //        animation.animationIndex = comp["animationIndex"].as<i32>(0);
//
                    //        if (_callCreate)
                    //            animation.Create();
                    //    }
                    //}

                    if (_callCreate)
                        model.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Model3D* model = nullptr;
                if (_entity.HasComponent<Model3D>() && ((model = &_entity.GetComponent<Model3D>()), true))
                {
                    ImGui::ColorEdit4("color", &model->color.r);

                    std::string modelLabel = "[ empty ]";
                    ModelAsset* modelAsset = nullptr;
                    if (model->modelId > -1)
                    {
                        modelAsset = AssetManager::GetModel(model->modelId);
                        if (modelAsset != nullptr)
                        {
                            if (MetaFileAsset* meta = AssetManager::GetMetaFile(modelAsset->GetPath()))
                                modelLabel = meta->name;
                            else
                                modelLabel = modelAsset->GetPath();
                        }
                    }

                    ImGui::Text("model");
                    ImGui::SameLine();
                    ImGui::Button(modelLabel.c_str(), ImVec2(150, 0));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
                        {
                            const AssetDragData dropped = *static_cast<const AssetDragData*>(payload->Data);
                            std::string path = AssetManager::GetPath(dropped.uuid);
                            std::string extension = GetFileExtension(path);

                            if (extension == "gltf" || extension == "glb")
                            {
                                model->modelId = AssetManager::LoadModel(path);
                                if (ModelAnimation3D* animation = _entity.HasComponent<ModelAnimation3D>() ? &_entity.GetComponent<ModelAnimation3D>() : nullptr)
                                {
                                    animation->animationTime = 0.0f;
                                    animation->animationIndex = 0;
                                    animation->poseModelId = -1;
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                }
            },
        };

        RegisterScript(model3DConf);

        ScriptConf modelAnimation3DConf = {
            .name = "Canis::ModelAnimation3D",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Model3D>())
                {
                    if (!_entity.HasComponent<Transform3D>())
                        _entity.AddComponent<Transform3D>();

                    Model3D* model = _entity.AddComponent<Model3D>();
                    model->modelId = AssetManager::LoadModel("assets/models/dq.gltf");
                }

                ModelAnimation3D* animation = _entity.AddComponent<ModelAnimation3D>();
                animation->animationIndex = 0;
                animation->animationTime = 0.0f;
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<ModelAnimation3D>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<ModelAnimation3D>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<ModelAnimation3D>() ? (void*)(&_entity.GetComponent<ModelAnimation3D>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<ModelAnimation3D>())
                {
                    ModelAnimation3D& animation = _entity.GetComponent<ModelAnimation3D>();
                    YAML::Node comp;
                    comp["playAnimation"] = animation.playAnimation;
                    comp["loop"] = animation.loop;
                    comp["animationSpeed"] = animation.animationSpeed;
                    comp["animationTime"] = animation.animationTime;
                    comp["animationIndex"] = animation.animationIndex;
                    _node["Canis::ModelAnimation3D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::ModelAnimation3D"])
                {
                    auto &animation = *_entity.AddComponent<ModelAnimation3D>();
                    animation.playAnimation = comp["playAnimation"].as<bool>(true);
                    animation.loop = comp["loop"].as<bool>(true);
                    animation.animationSpeed = comp["animationSpeed"].as<float>(1.0f);
                    animation.animationTime = comp["animationTime"].as<float>(0.0f);
                    animation.animationIndex = comp["animationIndex"].as<i32>(0);

                    if (_callCreate)
                        animation.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                ModelAnimation3D* animation = nullptr;
                if (_entity.HasComponent<ModelAnimation3D>() && ((animation = &_entity.GetComponent<ModelAnimation3D>()), true))
                {
                    ImGui::Checkbox("playAnimation", &animation->playAnimation);
                    ImGui::Checkbox("loop", &animation->loop);
                    ImGui::InputFloat("animationSpeed", &animation->animationSpeed);
                    ImGui::InputFloat("animationTime", &animation->animationTime);

                    ModelAsset* modelAsset = nullptr;
                    if (Model3D* model = _entity.HasComponent<Model3D>() ? &_entity.GetComponent<Model3D>() : nullptr)
                    {
                        if (model->modelId > -1)
                            modelAsset = AssetManager::GetModel(model->modelId);
                    }

                    if (modelAsset != nullptr)
                    {
                        const i32 animationCount = modelAsset->GetAnimationCount();
                        ImGui::Text("animations: %d", animationCount);

                        if (animationCount > 0)
                        {
                            animation->animationIndex = std::clamp(animation->animationIndex, 0, animationCount - 1);
                            ImGui::InputInt("animationIndex", &animation->animationIndex);
                            animation->animationIndex = std::clamp(animation->animationIndex, 0, animationCount - 1);

                            ImGui::Text("clip: %s", modelAsset->GetAnimationName(animation->animationIndex).c_str());
                        }
                    }
                    else
                    {
                        ImGui::Text("Model3D is required.");
                    }
                }
            },
        };

        RegisterScript(modelAnimation3DConf);

        ScriptConf spriteAnimationConf = {
            .name = "Canis::SpriteAnimation",
            .Construct = nullptr,
            .Add = [this](Entity& _entity) -> void {
                if (!_entity.HasComponent<Sprite2D>())
                {
                    Sprite2D* sprite = _entity.AddComponent<Sprite2D>();
                    sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                }
                
                SpriteAnimation* anim = _entity.AddComponent<SpriteAnimation>();
            },
            .Has = [this](Entity& _entity) -> bool { return _entity.HasComponent<SpriteAnimation>(); },
            .Remove = [this](Entity& _entity) -> void { _entity.RemoveComponent<SpriteAnimation>(); },
            .Get = [this](Entity& _entity) -> void* { return _entity.HasComponent<SpriteAnimation>() ? (void*)(&_entity.GetComponent<SpriteAnimation>()) : nullptr; },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (_entity.HasComponent<SpriteAnimation>())
                {
                    SpriteAnimation& animation = _entity.GetComponent<SpriteAnimation>();

                    YAML::Node comp;
                    comp["id"] = (uint64_t)AssetManager::GetMetaFile(AssetManager::GetPath(animation.id))->uuid;
                    comp["speed"] = animation.speed;

                    _node["Canis::SpriteAnimation"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::SpriteAnimation"])
                {
                    auto &animation = *_entity.AddComponent<SpriteAnimation>();
                    
                    Canis::UUID uuid = comp["id"].as<u64>();
                    AssetManager::GetSpriteAnimation(AssetManager::GetPath(uuid));
                    animation.id = AssetManager::GetID(uuid);
                    animation.speed = comp["speed"].as<f32>(1.0f);
                    
                    if (_callCreate)
                        animation.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                SpriteAnimation* animation = nullptr;
                if (_entity.HasComponent<SpriteAnimation>() && ((animation = &_entity.GetComponent<SpriteAnimation>()), true))
                {
                    
                    _editor.InputAnimationClip("animation", animation->id);
                    ImGui::InputFloat("speed", &animation->speed);
                }
            },
        };

        RegisterScript(spriteAnimationConf);

        // register inspector items
        InspectorItemRightClick inspectorCreateSquare = {
            .name = "Create Square",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *entityOne = _app.scene.CreateEntity("Square");
                RectTransform * transform = entityOne->AddComponent<RectTransform>();
                Canis::Sprite2D *sprite = entityOne->AddComponent<Sprite2D>();

                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                transform->size = Vector2(64.0f);
            }
        };

        RegisterInspectorItem(inspectorCreateSquare);

        InspectorItemRightClick inspectorCreateCircle = {
            .name = "Create Circle",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *entityOne = _app.scene.CreateEntity("Circle");
                RectTransform * transform = entityOne->AddComponent<RectTransform>();
                Canis::Sprite2D *sprite = entityOne->AddComponent<Sprite2D>();

                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/circle.png");
                transform->size = Vector2(64.0f);
            }
        };

        RegisterInspectorItem(inspectorCreateCircle);

        InspectorItemRightClick inspectorCreateDirectionalLight = {
            .name = "Create Directional Light",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *lightEntity = _app.scene.CreateEntity("Directional Light");
                lightEntity->AddComponent<DirectionalLight>();
            }
        };

        RegisterInspectorItem(inspectorCreateDirectionalLight);

        InspectorItemRightClick inspectorCreatePointLight = {
            .name = "Create Point Light",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *lightEntity = _app.scene.CreateEntity("Point Light");
                Transform3D *transform = lightEntity->AddComponent<Transform3D>();
                transform->position = Vector3(2.0f, 2.5f, 2.0f);
                lightEntity->AddComponent<PointLight>();
            }
        };

        RegisterInspectorItem(inspectorCreatePointLight);
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

    ScriptConf* App::GetScriptConf(const std::string& _name)
    {
        for(ScriptConf& sc : m_scriptRegistry)
        {
            if (sc.name == _name)
            {
                return &sc;
            }
        }

        return nullptr;
    }

    bool App::AddRequiredScript(Entity& _entity, const std::string& _name)
    {
        if (ScriptConf* sc = GetScriptConf(_name))
        {
            if (sc->Has && sc->Has(_entity) == false)
            {
                if (sc->Add)
                    sc->Add(_entity);
            }

            return true;
        }
        else
        {
            return false;
        }
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
                ScriptConf& conf = m_scriptRegistry[i];

                for (Entity* entity : scene.GetEntities())
                {
                    if (entity == nullptr)
                        continue;

                    entity->RemoveScript(conf.name);
                }

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

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
#ifdef Win32
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

        // init window
        Window window("Canis Beta", 512, 512);
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
        editor.Init(&window);

        RegisterDefaults(editor);

        InputManager inputManager;

        if (Canis::GetProjectConfig().useFrameLimit)
            Time::Init(Canis::GetProjectConfig().frameLimit + 0.0f);
        else
            Time::Init(100000.0f);
        
        #if CANIS_EDITOR
        Time::SetTargetFPS(Canis::GetProjectConfig().frameLimitEditor + 0.0f);
        #endif

        scene.Init(this, &window, &inputManager, "assets/scenes/scene3d.scene");

        GameCodeObject gameCodeObject = GameCodeObjectInit(sharedObjectPath);
        GameCodeObjectInitFunction(&gameCodeObject, this);

        // call after all the systems are added
        // and after script from the game lib have been registered
        scene.Load(m_scriptRegistry);

        while (inputManager.Update((void *)&window))
        {
            f32 deltaTime = Time::StartFrame();

            bool runGameTick = true;
            #if CANIS_EDITOR
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
            
            editor.Draw(&scene, &window, this, &gameCodeObject, deltaTime);

            Uint64 renderStart = SDL_GetTicksNS();
            window.Clear();
            scene.Render(deltaTime);
            window.SwapBuffer();
            m_renderTimeMs = static_cast<float>(SDL_GetTicksNS() - renderStart) / 1000000.0f;

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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::RectTransform::ScriptName, new Canis::RectTransform(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                CANIS_ADD_SCRIPT(_entity, RectTransform);
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, RectTransform) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, RectTransform); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, RectTransform); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, RectTransform))
                {
                    RectTransform& transform = *CANIS_GET_SCRIPT(_entity, RectTransform);

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
                    auto &rt = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::RectTransform, false);
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
                if ((transform = CANIS_GET_SCRIPT(_entity, RectTransform)) != nullptr)
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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Sprite2D::ScriptName, new Canis::Sprite2D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                // TODO: require a RectTransform component
                Sprite2D* sprite = CANIS_ADD_SCRIPT(_entity, Sprite2D);
                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                //sprite->size.x = sprite->textureHandle.texture.width;
                //sprite->size.y = sprite->textureHandle.texture.height;
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Sprite2D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, Sprite2D); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Sprite2D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Sprite2D))
                {
                    Sprite2D& sprite = *CANIS_GET_SCRIPT(_entity, Sprite2D);

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
                    auto &sprite = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Sprite2D, false);
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
                if ((sprite = CANIS_GET_SCRIPT(_entity, Sprite2D)) != nullptr)
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
                        if (SpriteAnimation* animation = CANIS_GET_SCRIPT(_entity, SpriteAnimation))
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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Text::ScriptName, new Canis::Text(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, RectTransform) == nullptr)
                {
                    CANIS_ADD_SCRIPT(_entity, RectTransform);
                }

                Text* text = CANIS_ADD_SCRIPT(_entity, Text);
                text->assetId = AssetManager::LoadText("assets/fonts/Antonio-Bold.ttf", 32);
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Text) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, Text); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Text); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Text))
                {
                    Text& text = *CANIS_GET_SCRIPT(_entity, Text);
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
                    auto &text = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Text, false);
                    text.text = comp["text"].as<std::string>("");
                    text.color = comp["color"].as<Vector4>(Color(1.0f));
                    text.alignment = comp["alignment"].as<unsigned int>(Canis::TextAlignment::LEFT);
                    text.horizontalBoundary = comp["horizontalBoundary"].as<unsigned int>(Canis::TextBoundary::OVERFLOW);
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
                if ((text = CANIS_GET_SCRIPT(_entity, Text)) != nullptr)
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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Camera2D::ScriptName, new Canis::Camera2D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void { CANIS_ADD_SCRIPT(_entity, Camera2D); },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Camera2D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, Camera2D); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Camera2D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Camera2D))
                {
                    Camera2D& camera = *CANIS_GET_SCRIPT(_entity, Camera2D);

                    YAML::Node comp;
                    comp["position"] = camera.GetPosition();
                    comp["scale"] = camera.GetScale();

                    _node["Canis::Camera2D"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto camera2DComponent = _node["Canis::Camera2D"])
                {
                    auto &camera = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Camera2D, false);
                    camera.SetPosition(camera2DComponent["position"].as<Vector2>(camera.GetPosition()));
                    camera.SetScale(camera2DComponent["scale"].as<float>(camera.GetScale()));
                    if (_callCreate)
                        camera.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Camera2D* camera = nullptr;
                if ((camera = CANIS_GET_SCRIPT(_entity, Camera2D)) != nullptr)
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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Transform3D::ScriptName, new Canis::Transform3D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void { CANIS_ADD_SCRIPT(_entity, Transform3D); },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Transform3D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, Transform3D); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Transform3D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Transform3D))
                {
                    Transform3D& transform = *CANIS_GET_SCRIPT(_entity, Transform3D);
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
                    auto &transform = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Transform3D, false);
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
                if ((transform = CANIS_GET_SCRIPT(_entity, Transform3D)) != nullptr)
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

        ScriptConf camera3DConf = {
            .name = "Canis::Camera3D",
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Camera3D::ScriptName, new Canis::Camera3D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Transform3D) == nullptr)
                    CANIS_ADD_SCRIPT(_entity, Transform3D);

                CANIS_ADD_SCRIPT(_entity, Camera3D);
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Camera3D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, Camera3D); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Camera3D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Camera3D))
                {
                    Camera3D& camera = *CANIS_GET_SCRIPT(_entity, Camera3D);
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
                    auto &camera = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Camera3D, false);
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
                if ((camera = CANIS_GET_SCRIPT(_entity, Camera3D)) != nullptr)
                {
                    ImGui::Checkbox("primary", &camera->primary);
                    ImGui::InputFloat("fovDegrees", &camera->fovDegrees);
                    ImGui::InputFloat("nearClip", &camera->nearClip);
                    ImGui::InputFloat("farClip", &camera->farClip);
                }
            },
        };

        RegisterScript(camera3DConf);

        ScriptConf model3DConf = {
            .name = "Canis::Model3D",
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::Model3D::ScriptName, new Canis::Model3D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Transform3D) == nullptr)
                    CANIS_ADD_SCRIPT(_entity, Transform3D);

                Model3D* model = CANIS_ADD_SCRIPT(_entity, Model3D);
                model->modelId = AssetManager::LoadModel("assets/models/dq.gltf");

                if (CANIS_GET_SCRIPT(_entity, ModelAnimation3D) == nullptr)
                {
                    ModelAnimation3D* animation = CANIS_ADD_SCRIPT(_entity, ModelAnimation3D);
                    animation->animationIndex = 0;
                    animation->animationTime = 0.0f;
                }
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, Model3D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void {
                CANIS_REMOVE_SCRIPT(_entity, ModelAnimation3D);
                CANIS_REMOVE_SCRIPT(_entity, Model3D);
            },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, Model3D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::Model3D))
                {
                    Model3D& model = *CANIS_GET_SCRIPT(_entity, Model3D);
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
                    auto &model = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::Model3D, false);
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
                    if (!_node["Canis::ModelAnimation3D"])
                    {
                        const bool hasLegacyAnimation =
                            comp["playAnimation"].IsDefined() ||
                            comp["loop"].IsDefined() ||
                            comp["animationSpeed"].IsDefined() ||
                            comp["animationTime"].IsDefined() ||
                            comp["animationIndex"].IsDefined();

                        if (hasLegacyAnimation && CANIS_GET_SCRIPT(_entity, Canis::ModelAnimation3D) == nullptr)
                        {
                            auto &animation = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::ModelAnimation3D, false);
                            animation.playAnimation = comp["playAnimation"].as<bool>(true);
                            animation.loop = comp["loop"].as<bool>(true);
                            animation.animationSpeed = comp["animationSpeed"].as<float>(1.0f);
                            animation.animationTime = comp["animationTime"].as<float>(0.0f);
                            animation.animationIndex = comp["animationIndex"].as<i32>(0);

                            if (_callCreate)
                                animation.Create();
                        }
                    }

                    if (_callCreate)
                        model.Create();
                }
            },
            .DrawInspector = [this](Editor& _editor, Entity& _entity, const ScriptConf& _conf) -> void {
                Model3D* model = nullptr;
                if ((model = CANIS_GET_SCRIPT(_entity, Model3D)) != nullptr)
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
                                if (ModelAnimation3D* animation = CANIS_GET_SCRIPT(_entity, ModelAnimation3D))
                                {
                                    animation->animationTime = 0.0f;
                                    animation->animationIndex = 0;
                                    animation->poseModelId = -1;
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (modelAsset != nullptr)
                    {
                        const i32 animationCount = modelAsset->GetAnimationCount();
                        ImGui::Text("animations: %d", animationCount);
                    }
                }
            },
        };

        RegisterScript(model3DConf);

        ScriptConf modelAnimation3DConf = {
            .name = "Canis::ModelAnimation3D",
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::ModelAnimation3D::ScriptName, new Canis::ModelAnimation3D(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Model3D) == nullptr)
                {
                    if (CANIS_GET_SCRIPT(_entity, Transform3D) == nullptr)
                        CANIS_ADD_SCRIPT(_entity, Transform3D);

                    Model3D* model = CANIS_ADD_SCRIPT(_entity, Model3D);
                    model->modelId = AssetManager::LoadModel("assets/models/dq.gltf");
                }

                ModelAnimation3D* animation = CANIS_ADD_SCRIPT(_entity, ModelAnimation3D);
                animation->animationIndex = 0;
                animation->animationTime = 0.0f;
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, ModelAnimation3D) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, ModelAnimation3D); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, ModelAnimation3D); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::ModelAnimation3D))
                {
                    ModelAnimation3D& animation = *CANIS_GET_SCRIPT(_entity, ModelAnimation3D);
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
                    auto &animation = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::ModelAnimation3D, false);
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
                if ((animation = CANIS_GET_SCRIPT(_entity, ModelAnimation3D)) != nullptr)
                {
                    ImGui::Checkbox("playAnimation", &animation->playAnimation);
                    ImGui::Checkbox("loop", &animation->loop);
                    ImGui::InputFloat("animationSpeed", &animation->animationSpeed);
                    ImGui::InputFloat("animationTime", &animation->animationTime);

                    ModelAsset* modelAsset = nullptr;
                    if (Model3D* model = CANIS_GET_SCRIPT(_entity, Model3D))
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
            .Construct = [](Entity& _entity, bool _callCreate) -> ScriptableEntity* { return Canis::EntityAttachScriptByName(_entity, Canis::SpriteAnimation::ScriptName, new Canis::SpriteAnimation(_entity), _callCreate); },
            .Add = [this](Entity& _entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Sprite2D) == nullptr)
                {
                    Sprite2D* sprite = CANIS_ADD_SCRIPT(_entity, Sprite2D);
                    sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                }
                
                SpriteAnimation* anim = CANIS_ADD_SCRIPT(_entity, SpriteAnimation);
            },
            .Has = [this](Entity& _entity) -> bool { return (CANIS_GET_SCRIPT(_entity, SpriteAnimation) != nullptr); },
            .Remove = [this](Entity& _entity) -> void { CANIS_REMOVE_SCRIPT(_entity, SpriteAnimation); },
            .Get = [this](Entity& _entity) -> void* { return (void*)CANIS_GET_SCRIPT(_entity, SpriteAnimation); },
            .Encode = [](YAML::Node &_node, Entity &_entity) -> void {
                if (CANIS_GET_SCRIPT(_entity, Canis::SpriteAnimation))
                {
                    SpriteAnimation& animation = *CANIS_GET_SCRIPT(_entity, SpriteAnimation);

                    YAML::Node comp;
                    comp["id"] = (uint64_t)AssetManager::GetMetaFile(AssetManager::GetPath(animation.id))->uuid;
                    comp["speed"] = animation.speed;

                    _node["Canis::SpriteAnimation"] = comp;
                }
            },
            .Decode = [](YAML::Node &_node, Entity &_entity, bool _callCreate) -> void {
                if (auto comp = _node["Canis::SpriteAnimation"])
                {
                    auto &animation = *CANIS_ADD_SCRIPT_WITH_CREATE(_entity, Canis::SpriteAnimation, false);
                    
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
                if ((animation = CANIS_GET_SCRIPT(_entity, SpriteAnimation)) != nullptr)
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
                RectTransform * transform = CANIS_ADD_SCRIPT(entityOne, Canis::RectTransform);
                Canis::Sprite2D *sprite = CANIS_ADD_SCRIPT(entityOne, Canis::Sprite2D);

                sprite->textureHandle = Canis::AssetManager::GetTextureHandle("assets/defaults/textures/square.png");
                transform->size = Vector2(64.0f);
            }
        };

        RegisterInspectorItem(inspectorCreateSquare);

        InspectorItemRightClick inspectorCreateCircle = {
            .name = "Create Circle",
            .Func = [](App& _app, Editor& _editor, Entity& _entity, std::vector<ScriptConf>& _scriptConfs) -> void {
                Canis::Entity *entityOne = _app.scene.CreateEntity("Circle");
                RectTransform * transform = CANIS_ADD_SCRIPT(entityOne, Canis::RectTransform);
                Canis::Sprite2D *sprite = CANIS_ADD_SCRIPT(entityOne, Canis::Sprite2D);

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

        u32 componentIndex = u32_max;
        if (!m_freeComponentIndices.empty())
        {
            componentIndex = m_freeComponentIndices.back();
            m_freeComponentIndices.pop_back();
        }
        else
        {
            componentIndex = m_nextComponentIndex++;
        }

        if (componentIndex >= 63)
        {
            Debug::Error("Cannot register script '%s': max ECS component count reached (63).", _conf.name.c_str());
            return;
        }

        _conf.componentIndex = componentIndex;
        _conf.componentMask = (1ull << componentIndex);

        m_scriptRegistry.push_back(_conf);
        scene.RegisterComponent(componentIndex, _conf.componentMask);
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

                    if (entity->HasScript(conf.name))
                        entity->RemoveScript(conf.name);
                }

                scene.UnregisterComponent(conf.componentIndex);
                m_freeComponentIndices.push_back(conf.componentIndex);

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

#include <Canis/Editor.hpp>

#include <Canis/Canis.hpp>
#include <Canis/Debug.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Window.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>
#include <Canis/App.hpp>
#include <Canis/Time.hpp>
#include <Canis/Shader.hpp>
#include <Canis/IOManager.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/GameCodeObject.hpp>
#include <Canis/AssetManager.hpp>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

#include <filesystem>

namespace Canis
{
    std::vector<const char *> ConvertComponentToCStringVector(App &_app, Entity &_entity)
    {
        std::vector<const char *> cStringVector;
        for (ScriptConf &conf : _app.GetScriptRegistry())
        {
            if (conf.Has(_entity))
                continue;

            cStringVector.push_back(conf.name.c_str());
        }
        return cStringVector;
    }

    void Editor::Init(Window *_window)
    {
#if CANIS_EDITOR
        // if (GetProjectConfig().editor == false)
        //     return;
        //{

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

#ifdef __EMSCRIPTEN__

#else
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
#endif

        // io.ConfigViewportsNoAutoMerge = true;
        // io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle &style = ImGui::GetStyle();
        float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        style.ScaleAllSizes(main_scale);   // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;   // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)
        io.ConfigDpiScaleFonts = true;     // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
        io.ConfigDpiScaleViewports = true; // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForOpenGL((SDL_Window *)_window->GetSDLWindow(), (SDL_GLContext)_window->GetGLContext());
        ImGui_ImplOpenGL3_Init(OPENGLVERSION);

        m_assetPaths = FindFilesInFolder("assets", "");
#endif
    }

    void Editor::Draw(Scene *_scene, Window *_window, App *_app, GameCodeObject *_gameSharedLib)
    {
#if CANIS_EDITOR
        // if (GetProjectConfig().editor)
        //{
        if (m_scene != _scene)
        {
            Debug::Log("new scene");
        }
        m_app = _app;
        m_scene = _scene;
        m_window = _window;
        m_gameSharedLib = _gameSharedLib;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        bool refresh = DrawHierarchyPanel();
        DrawInspectorPanel(refresh);
        DrawEnvironment();
        // DrawSystemPanel();
        DrawAssetsPanel();
        DrawProjectSettings();
        DrawScenePanel(); // draw last

        SelectSprite2D();

        // find camera and verfy target entity
        m_debugDraw = DebugDraw::NONE;
        Camera2D *camera2D = nullptr;

        if (m_index > -1 && m_index < m_scene->GetEntities().size() && m_scene->GetEntities()[m_index] != nullptr)
        {
            Entity &entity = *m_scene->GetEntities()[m_index];

            std::vector<Entity *> &entities = m_scene->GetEntities();

            for (Entity *entity : entities)
            {
                if (entity == nullptr)
                    continue;

                Camera2D *camera = entity->GetScript<Camera2D>();

                if (camera == nullptr)
                    continue;

                camera2D = camera;
            }

            if (entity.GetScript<RectTransform>() && camera2D)
            {
                m_debugDraw = DebugDraw::RECT;
            }
        }

        // draw gizmo
        if (m_debugDraw == DebugDraw::RECT && m_scene->GetEntities()[m_index] != nullptr)
        {
            DrawGizmo(camera2D);
        }

        // rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        // draw debug bounding box
        if (m_debugDraw == DebugDraw::RECT && m_scene->GetEntities()[m_index] != nullptr)
        {
            DrawBoundingBox(camera2D);
        }
#endif
    }

    void Editor::FocusEntity(Canis::Entity* _entity)
    {
        for (int i = 0; i < m_scene->GetEntities().size(); i++)
        {
            if (m_scene->GetEntities()[i] == _entity)
            {
                m_index = i;
                return;
            }
        }
    }

    void Editor::InputEntity(const std::string& _name, Canis::Entity* &_variable)
    {
        ImGui::Text(_name.c_str());
        ImGui::SameLine();

        std::string label;
        Canis::Entity* entity = *&_variable;
        if (entity)
            label = "[entity] " + entity->name;
        else
            label = "[ missing entity ]";

        ImGui::Button(label.c_str(), ImVec2(150, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
            {
                const Canis::UUID dropped = *static_cast<const Canis::UUID*>(payload->Data);
                Canis::Entity* e = m_scene->GetEntityWithUUID(dropped);

                if (e)
                    *&_variable = e;
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (entity)
                FocusEntity(entity);
        }

        std::string popUpLabel = _name + "_ctx;";
        if (entity && ImGui::BeginPopupContextItem(popUpLabel.c_str()))
        {
            if (ImGui::MenuItem("Clear"))
                *&_variable = nullptr;

            if (ImGui::MenuItem("Select in Hierarchy"))
                FocusEntity(entity);

            ImGui::EndPopup();
        }
    }

    bool Editor::DrawHierarchyPanel()
    {
        ImGui::Begin("Hierarchy");
        bool refresh = false;

        std::vector<Entity *> &entities = m_scene->GetEntities();

        for (int i = 0; i < entities.size(); i++)
        {
            Canis::Entity* entity = entities[i];
            if (entity == nullptr)
                continue;

            // ImGui::Text("%s", entities[i]->name.c_str());
            std::string inputID = entities[i]->name + "##input" + std::to_string(i);
            ImGui::Selectable(inputID.c_str(), m_index == i);

            if (ImGui::IsItemDeactivated() && ImGui::IsItemHovered())
            {
                m_index = i;
                refresh = true;
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                Canis::UUID uuid = entity->uuid;

                ImGui::SetDragDropPayload("ENTITY_DRAG", &uuid, sizeof(Canis::UUID));
                ImGui::Text("Entity: %s", entity->name.c_str());

                ImGui::EndDragDropSource();
            }

            // TODO: extend from game dll
            if (ImGui::BeginPopupContextItem(("Menu##" + std::to_string(i)).c_str()))
            {
                if (ImGui::MenuItem(std::string("Create##").c_str()))
                {
                    m_scene->CreateEntity();
                }

                if (ImGui::MenuItem(std::string("Duplicate##").c_str()))
                {
                    // 1. make this into a function
                    // 2. once yaml is add encode then decode
                    Entity *selected = entities[i];
                    // Entity *entity = m_scene->CreateEntity();
                    // entity->name = selected->name; // ++ a number at the end
                    // entity->tag = selected->tag;

                    // encode
                    YAML::Node node = m_scene->EncodeEntity(m_app->GetScriptRegistry(), *selected);

                    // decode
                    Entity &entity = m_scene->DecodeEntity(m_app->GetScriptRegistry(), node, false);
                }

                if (ImGui::MenuItem(std::string("Remove##").c_str()))
                {
                    m_scene->Destroy(i);
                    i--;
                    ImGui::EndPopup();
                    continue;
                }

                for (int index = 0; index < m_app->GetInspectorItemRegistry().size(); index++)
                {
                    if (ImGui::MenuItem(std::string(m_app->GetInspectorItemRegistry()[index].name + "##").c_str()))
                    {
                        m_app->GetInspectorItemRegistry()[index].Func(*m_app, *this, *entities[i], m_app->GetScriptRegistry());
                    }
                }

                ImGui::EndPopup();
            }
        }

        /*for (int i = 0; i < GetSceneManager().hierarchyElements.size(); i++)
        {
            Entity entity = GetSceneManager().hierarchyElements[i].entity;

            if (entity.HasComponent<Canis::RectTransform>())
                if (entity.GetComponent<Canis::RectTransform>().parent)
                    continue;

            bool skip = DrawHierarchyElement(i);

            if (skip)
                break;
        }

        if (ImGui::Button("New Entity"))
        {
            Entity e = m_scene->CreateEntity();
            e.AddComponent<IDComponent>();

            HierarchyElementInfo hei;
            hei.entity.entityHandle = e.entityHandle;
            hei.entity.scene = m_scene;

            GetSceneManager().hierarchyElements.push_back(hei);
            m_forceRefresh = true;
        }*/

        ImGui::End();
        return refresh;
    }

    void Editor::DrawInspectorPanel(bool _refresh)
    {
        ImGui::Begin("Inspector");

        std::vector<Entity *> &entities = m_scene->GetEntities();

        Clamp(m_index, 0, entities.size() - 1);

        if (entities.size() != 0 && entities[m_index] != nullptr)
        {
            Entity &entity = *entities[m_index];

            ImGui::Text("name: ");
            ImGui::SameLine();
            ImGui::InputText("##name", &entity.name);
            ImGui::Text("tag:  ");
            ImGui::SameLine();
            ImGui::InputText("##tag", &entity.tag);

            for (ScriptConf &conf : m_app->GetScriptRegistry())
            {
                if (conf.Has(entity))
                {
                    bool open = ImGui::CollapsingHeader(conf.name.c_str());

                    if (ImGui::BeginPopupContextItem(std::string("Menu##" + conf.name).c_str()))
                    {
                        if (ImGui::MenuItem(std::string("Remove##" + conf.name).c_str()))
                        {
                            conf.Remove(entity);
                            open = false;
                        }

                        ImGui::EndPopup();
                    }

                    if (open)
                    {
                        conf.DrawInspector(*this, entity, conf);
                    }
                }
            }

            DrawAddComponentDropDown(_refresh);
        }

        ImGui::End();
    }

    void Editor::DrawAddComponentDropDown(bool _refresh)
    {
        Entity &entity = *m_scene->GetEntities()[m_index];

        static int componentToAdd = 0;

        if (_refresh)
            componentToAdd = 0;

        std::vector<const char *> cStringItems = ConvertComponentToCStringVector(*m_app, entity);

        if (cStringItems.size() > 0)
        {
            Clamp(componentToAdd, 0, cStringItems.size() - 1);
            ImGui::Combo("##Components", &componentToAdd, cStringItems.data(), static_cast<int>(cStringItems.size()));

            ImGui::SameLine();

            if (ImGui::Button("+##AddComponent"))
            {
                for (int i = 0; i < m_app->GetScriptRegistry().size(); i++)
                {
                    if (cStringItems[componentToAdd] == m_app->GetScriptRegistry()[i].name)
                    {
                        m_app->GetScriptRegistry()[i].Add(entity);
                        componentToAdd = 0;
                        break;
                    }
                }
            }
        }
    }

    void Editor::DrawEnvironment()
    {
        ImGui::Begin("Environment");
        Color background = m_window->GetClearColor();
        ImGui::ColorEdit4("Background##", &background.r);

        if (background != m_window->GetClearColor())
            m_window->SetClearColor(background);

        ImGui::End();
    }

    void Editor::CommitAssetRename()
    {
        using namespace std;
        namespace fs = std::filesystem;

        if (!m_isRenamingAsset || m_renamingPath.empty())
            return;

        string newName = m_renameBuffer;

        // nothing entered, cancel
        if (newName.empty())
        {
            m_isRenamingAsset = false;
            return;
        }

        fs::path oldPath = m_renamingPath;
        fs::path newPath = oldPath;
        newPath.replace_filename(newName);

        // handle no change
        if (newPath == oldPath)
        {
            m_isRenamingAsset = false;
            return;
        }

        AssetManager::MoveAsset(oldPath.string(), newPath.string());

        m_isRenamingAsset = false;
        m_renamingPath.clear();
    }

    void Editor::DrawDirectoryRecursive(const std::string &_dirPath)
    {
        namespace fs = std::filesystem;
        fs::path path = _dirPath;

        for (const auto &entry : fs::directory_iterator(path))
        {
            const std::string name = entry.path().filename().string();
            if (name == ".DS_Store" || entry.path().extension() == ".meta")
                continue;

            if (entry.is_directory())
            {
                ImGuiTreeNodeFlags nodeFlags =
                    ImGuiTreeNodeFlags_OpenOnArrow |
                    ImGuiTreeNodeFlags_SpanAvailWidth;
                bool open = ImGui::TreeNodeEx(entry.path().string().c_str(), nodeFlags, "%s", name.c_str());

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
                    {
                        const AssetDragData *data = static_cast<const AssetDragData *>(payload->Data);

                        fs::path src = data->path;

                        AssetManager::MoveAsset(src.string(), entry.path().string() + "/" + src.filename().string());
                    }
                    ImGui::EndDragDropTarget();
                }

                // right click
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Create 2D Scene"))
                    {
                        // copy scene template
                        std::string from = "assets/defaults/templates/scenes/2d_scene.scene";
                        std::string to = entry.path().string() + "/new_2d_scene.scene";
                        std::error_code ec;
                        fs::copy_file(from, to, ec);

                        // add it to asset manager meta
                        if (!ec)
                            MetaFileAsset *meta = AssetManager::GetMetaFile(to);
                    }

                    ImGui::EndPopup();
                }

                if (open)
                {
                    DrawDirectoryRecursive(entry.path().string());
                    ImGui::TreePop();
                }
            }
            else if (entry.is_regular_file())
            {
                const std::string fullPath = entry.path().string();
                const bool isRenamingThis = m_isRenamingAsset && (m_renamingPath == fullPath);

                if (isRenamingThis)
                {
                    // rename input
                    ImGui::PushID(fullPath.c_str());
                    ImGui::SetNextItemWidth(-1.0f);

                    ImGuiInputTextFlags flags =
                        ImGuiInputTextFlags_EnterReturnsTrue |
                        ImGuiInputTextFlags_AutoSelectAll |
                        ImGuiInputTextFlags_CharsNoBlank;

                    if (ImGui::InputText("##rename", m_renameBuffer, sizeof(m_renameBuffer), flags))
                    {
                        CommitAssetRename();
                    }

                    // click elsewhere or escape will cancel
                    if (!ImGui::IsItemActive() &&
                        (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
                    {
                        m_isRenamingAsset = false;
                    }

                    ImGui::PopID();
                }
                else
                {
                    ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

                    // right click
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Rename"))
                        {
                            m_isRenamingAsset = true;
                            m_renamingPath = fullPath;

                            std::strncpy(m_renameBuffer, name.c_str(), sizeof(m_renameBuffer));
                            m_renameBuffer[sizeof(m_renameBuffer) - 1] = '\0';
                        }

                        // TODO: Delete, Reveal in Finder, etc.

                        ImGui::EndPopup();
                    }

                    // drag source (for moving + using UUID elsewhere)
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        MetaFileAsset *meta = AssetManager::GetMetaFile(entry.path().string());
                        if (meta)
                        {
                            AssetDragData data{};
                            data.uuid = meta->uuid;

                            std::string full = entry.path().string();
                            std::snprintf(data.path, sizeof(data.path), "%s", full.c_str());

                            ImGui::SetDragDropPayload("ASSET_DRAG", &data, sizeof(data));
                            ImGui::Text("Asset: %s", meta->name.c_str());
                        }
                        ImGui::EndDragDropSource();
                    }

                    // double-click handling (open scene / shader)
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        MetaFileAsset *meta = AssetManager::GetMetaFile(entry.path().string());
                        if (!meta)
                            continue;

                        if (meta->type == MetaFileAsset::FileType::SCENE && m_mode == EditorMode::EDIT)
                        {
                            m_scene->Unload();
                            m_scene->Init(m_app, m_window, &m_scene->GetInputManager(), meta->path);
                            m_scene->Load(m_app->GetScriptRegistry());
                        }
                        else if ((meta->type == MetaFileAsset::FileType::FRAGMENT ||
                                  meta->type == MetaFileAsset::FileType::VERTEX) &&
                                 m_mode == EditorMode::EDIT)
                        {
                            OpenInVSCode(std::string(SDL_GetBasePath()) + meta->path);
                        }
                    }
                }
            }
        }
    }

    void Editor::DrawAssetsPanel()
    {
        namespace fs = std::filesystem;

        ImGui::Begin("Assets");

        DrawDirectoryRecursive("assets");

        ImGui::End();
    }

    void Editor::DrawProjectSettings()
    {
        ImGui::Begin("ProjectSettings");

        if (ImGui::Button("Save Project", ImVec2(-1.0f, 0.0f)))
        {
            Canis::SaveProjectConfig();
        }

        // fps limit checkbox
        ImGui::Text("in-game fps limit"); ImGui::SameLine();
        if (ImGui::Checkbox("##useFPSLimit", &Canis::GetProjectConfig().useFrameLimit) && m_mode == EditorMode::PLAY)
        {
            if (Canis::GetProjectConfig().useFrameLimit)
                Time::SetTargetFPS(Canis::GetProjectConfig().frameLimit + 0.0f);
            else
                Time::SetTargetFPS(100000.0f);
        }

        // fps limit input
        if (Canis::GetProjectConfig().useFrameLimit)
        {
            ImGui::Text("    fps limit"); ImGui::SameLine();
            if (ImGui::InputInt("##frameLimit", &Canis::GetProjectConfig().frameLimit, 0) && m_mode == EditorMode::PLAY)
                Time::SetTargetFPS(Canis::GetProjectConfig().frameLimit + 0.0f);
        }

        // editor fps limit input
        ImGui::Text("editor fps"); ImGui::SameLine();
        if (ImGui::InputInt("##editorframeLimit", &Canis::GetProjectConfig().frameLimitEditor, 0) && m_mode == EditorMode::EDIT)
                Time::SetTargetFPS(Canis::GetProjectConfig().frameLimitEditor + 0.0f);

        // application icon
        ImGui::Text("icon");
        ImGui::SameLine();
        ImGui::Button(
            AssetManager::GetMetaFile(AssetManager::GetPath(Canis::GetProjectConfig().iconUUID))->name.c_str(),
            ImVec2(150, 0)
        );

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
            {
                const AssetDragData dropped = *static_cast<const AssetDragData*>(payload->Data);
                std::string path = AssetManager::GetPath(dropped.uuid);
                TextureAsset* asset = AssetManager::GetTexture(path);

                if (asset) // validate that this is a texture
                {
                    Canis::GetProjectConfig().iconUUID = dropped.uuid;
                    Canis::SaveProjectConfig();
                    m_window->SetWindowIcon(path);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::End();
    }

    void Editor::DrawScenePanel()
    {
        static YAML::Node lastSceneNode;
        static float hotKeyCoolDown = 0.0f;
        const float HOTKEYRESET = 0.1f;

        ImGui::Begin("Scene");

        if (m_mode == EditorMode::EDIT)
        {
            if (ImGui::Button("Save##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_S) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
            {
                hotKeyCoolDown = HOTKEYRESET;
                m_scene->Save(m_app->GetScriptRegistry());
            }
            ImGui::SameLine();
            if (ImGui::Button("Play##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_P) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
            {
                hotKeyCoolDown = HOTKEYRESET;
                if (Canis::GetProjectConfig().useFrameLimit)
                    Time::SetTargetFPS(Canis::GetProjectConfig().frameLimit + 0.0f);
                else
                    Time::SetTargetFPS(100000.0f);
                // save copy of scene
                lastSceneNode = m_scene->EncodeScene(m_app->GetScriptRegistry());

                m_mode = EditorMode::PLAY;
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_R) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
            {
                hotKeyCoolDown = HOTKEYRESET;

                m_assetPaths = FindFilesInFolder("assets", "");

                // save copy of scene
                lastSceneNode = m_scene->EncodeScene(m_app->GetScriptRegistry());

                // unload data
                m_scene->Unload();

                GameCodeObjectWatchFile(m_gameSharedLib, m_app);

                m_scene->LoadSceneNode(m_app->GetScriptRegistry(), lastSceneNode);
            }
        }
        else
        {
            if (m_mode == EditorMode::PLAY)
            {
                if (ImGui::Button("Pause##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_P) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
                {
                    hotKeyCoolDown = HOTKEYRESET;
                    m_mode = EditorMode::PAUSE;
                }
            }
            else if (m_mode == EditorMode::PAUSE)
            {
                if (ImGui::Button("Resume##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_P) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
                {
                    hotKeyCoolDown = HOTKEYRESET;
                    m_mode = EditorMode::PLAY;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Stop##ScenePanel") || (ImGui::IsKeyDown(ImGuiKey_Q) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && hotKeyCoolDown < 0.0f))
            {
                hotKeyCoolDown = HOTKEYRESET;
                Time::SetTargetFPS(Canis::GetProjectConfig().frameLimitEditor + 0.0f);
                m_mode = EditorMode::EDIT;
                // restore from copy
                m_scene->Unload();
                m_scene->LoadSceneNode(m_app->GetScriptRegistry(), lastSceneNode);
            }
        }

        hotKeyCoolDown -= Time::DeltaTime();

        ImGui::SameLine();
        ImGui::Text("FPS: %s", std::to_string(m_app->FPS()).c_str());

        ImGui::SameLine();

        if (m_guizmoMode == GuizmoMode::LOCAL)
        {
            if (ImGui::Button("Local##ScenePanel"))
            {
                m_guizmoMode = GuizmoMode::WORLD;
            }
        }
        else
        {
            if (ImGui::Button("World##ScenePanel"))
            {
                m_guizmoMode = GuizmoMode::LOCAL;
            }
        }

        ImGui::End();
    }

    void Editor::SelectSprite2D()
    {
        // TODO: this will only be true when the mouse is over the game window
        if (m_scene->GetInputManager().JustLeftClicked() == false)
            return;

        // TODO: this will need to be adjested when I add canvas
        Vector2 mouse = m_scene->GetInputManager().mouse - (Vector2(m_window->GetScreenWidth(), m_window->GetScreenHeight()) / 2.0f);

        bool mouseLock = false;

        for (int i = 0; i < m_scene->GetEntities().size(); i++)
        {
            if (m_scene->GetEntities()[i] == nullptr)
                continue;

            RectTransform *transform = m_scene->GetEntities()[i]->GetScript<RectTransform>();

            if (transform == nullptr)
                continue;

            Vector2 globalPos = transform->GetPosition();    // rect_transform.GetGlobalPosition(window->GetScreenWidth(), window->GetScreenHeight());
            float globalRotation = transform->rotation; // rect_transform.GetGlobalRotation();

            if (globalRotation != 0.0f)
            {
                RotatePointAroundPivot(
                    mouse,
                    globalPos /* + rect_transform.rotationOriginOffset */,
                    -globalRotation);
            }

            // TODO: should add depth sort
            if (mouse.x > globalPos.x - transform->size.x * 0.5f * transform->scale.x &&
                mouse.x < globalPos.x + transform->size.x * 0.5f * transform->scale.x &&
                mouse.y > globalPos.y - transform->size.x * 0.5f * transform->scale.y &&
                mouse.y < globalPos.y + transform->size.x * 0.5f * transform->scale.y &&
                !mouseLock)
            {
                m_index = i;
            }
        }
    }

    void Editor::DrawGizmo(Camera2D *_camera2D)
    {
        SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);

        ImGuizmo::BeginFrame();

        ImGuiViewport *mainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(mainViewport->WorkPos);
        ImGui::SetNextWindowSize(mainViewport->WorkSize);
        ImGui::SetNextWindowViewport(mainViewport->ID);

        ImGui::Begin("##GuizmoWindow", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBackground);

        // === Gizmo operation selector ===
        static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;

        if (ImGui::GetIO().WantTextInput)
        {
            // user is focused on an InputText or InputTextMultiline
            // Debug::Log("User is typing in an input field.");
        }
        else
        {
            // safe to use keyboard shortcuts
            if (ImGui::IsKeyPressed(ImGuiKey_W))
                operation = ImGuizmo::TRANSLATE;

            if (ImGui::IsKeyPressed(ImGuiKey_E))
                operation = ImGuizmo::ROTATE;

            if (ImGui::IsKeyPressed(ImGuiKey_R))
                operation = ImGuizmo::SCALE;
        }

        Matrix4 projection;
        projection.Identity();

        projection = _camera2D->GetProjectionMatrix();

        if (_camera2D == nullptr)
            Debug::Log("NULL");

        Entity &debugRectTransformEntity = *m_scene->GetEntities()[m_index];

        RectTransform &rtc = *debugRectTransformEntity.GetScript<RectTransform>();
        Vector2 pos = rtc.GetPosition();

        // Align to bottom-left
        // ADD BACK pos += rtc.rotationOriginOffset;

        Matrix4 model;
        model.Identity();
        model.Translate(Vector3(pos.x, pos.y, 0.0f));
        model.Rotate(rtc.rotation, Vector3(0.0f, 0.0f, 1.0f));
        model.Scale(Vector3(rtc.size.x * rtc.scale.x, rtc.size.y * rtc.scale.y, 1.0f));

        ImGuizmo::SetOrthographic(true);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(mainViewport->WorkPos.x, mainViewport->WorkPos.y, mainViewport->WorkSize.x, mainViewport->WorkSize.y);
        ImGuizmo::Enable(true);

        Matrix4 view; // = camera2D->GetViewMatrix();
        view.Identity();
        view.Translate(
            Vector3(
                _camera2D->GetPosition().x + m_window->GetScreenWidth() / 2.0f,
                _camera2D->GetPosition().y + m_window->GetScreenHeight() / 2.0f,
                0.0f));
        view.Scale(Vector3(_camera2D->GetScale()));

        ImGuizmo::Manipulate(
            &view[0],
            &projection[0],
            operation,
            (ImGuizmo::MODE)m_guizmoMode,
            &model[0]);

        Vector2 cameraPos = _camera2D->GetPosition();
        float cameraRot = _camera2D->GetScale();

        if (ImGuizmo::IsUsing())
        {
            float t[3], r[3], s[3];
            ImGuizmo::DecomposeMatrixToComponents(&model[0], t, r, s);

            Vector3 translation(t[0], t[1], t[2]), rotation(r[0], r[1], r[2]), scale(s[0], s[1], s[2]);

            // update position
            Vector2 newPos(translation.x, translation.y);
            Vector2 oldPos = rtc.GetPosition();
            // ADD BACK Vector2 oldPos = rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight()) + rtc.originOffset;
            // ADD BACK oldPos += rtc.rotationOriginOffset;
            rtc.Move(newPos - oldPos);

            // update rotation
            rtc.rotation = DEG2RAD * rotation.z;

            // update size (scale stays constant, we resize the actual size)
            rtc.scale = Vector2(scale.x / rtc.size.x, scale.y / rtc.size.y);
        }

        ImGui::End();
    }

    void Editor::DrawBoundingBox(Camera2D *_camera2D)
    {
        Matrix4 projection;
        projection.Identity();

        projection = _camera2D->GetCameraMatrix();

        static Canis::Shader debugLineShader("assets/shaders/debug_line.vs", "assets/shaders/debug_line.fs");
        Entity &debugRectTransformEntity = *m_scene->GetEntities()[m_index];
        RectTransform &rtc = *debugRectTransformEntity.GetScript<RectTransform>();
        Vector2 pos = rtc.position; // rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight());
        pos += rtc.originOffset;
        // Vector2 vertices[] = {
        //     {pos.x, pos.y},
        //     {pos.x + (rtc.size.x * rtc.scale), pos.y},
        //     {pos.x + (rtc.size.x * rtc.scale), pos.y + (rtc.size.y * rtc.scale)},
        //     {pos.x, pos.y + (rtc.size.y * rtc.scale)}};
        Vector2 vertices[] = {
            {pos.x - (rtc.size.x * rtc.scale.x * 0.5f), pos.y - (rtc.size.y * rtc.scale.y * 0.5f)},
            {pos.x + (rtc.size.x * rtc.scale.x * 0.5f), pos.y - (rtc.size.y * rtc.scale.y * 0.5f)},
            {pos.x + (rtc.size.x * rtc.scale.x * 0.5f), pos.y + (rtc.size.y * rtc.scale.y * 0.5f)},
            {pos.x - (rtc.size.x * rtc.scale.x * 0.5f), pos.y + (rtc.size.y * rtc.scale.y * 0.5f)}};

        for (Vector2 &v : vertices)
            RotatePointAroundPivot(
                v,
                Vector2(pos.x, pos.y) /*vertices[0] + rtc.originOffset + rtc.rotationOriginOffset*/,
                -rtc.rotation // debugRectTransform.GetGlobalRotation()
            );

        for (Vector2 &v : vertices)
            v = Vector2(projection * Vector4(v.x, v.y, 0.0f, 1.0f));

        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        debugLineShader.Use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        debugLineShader.UnUse();

        // clean up
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

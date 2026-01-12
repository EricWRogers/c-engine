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
#include <imgui_internal.h>

#include <ImGuizmo.h>

#include <filesystem>
#include <cstdint>

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
        //ImGui::LoadIniSettingsFromMemory("");
        static std::string imguiIniPath = std::string(SDL_GetBasePath()) + "project_settings/imgui.ini";
        io.IniFilename = imguiIniPath.c_str();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        io.ConfigWindowsMoveFromTitleBarOnly = true;

#ifdef __EMSCRIPTEN__

#else
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Keep all windows in the main SDL window
        io.ConfigViewportsNoAutoMerge = true;
        io.ConfigViewportsNoTaskBarIcon = true;
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

        m_gameViewportWidth = _window->GetScreenWidth();
        m_gameViewportHeight = _window->GetScreenHeight();
        EnsureGameRenderTarget(m_gameViewportWidth, m_gameViewportHeight);
#endif
    }

    Editor::~Editor()
    {
        DestroyGameRenderTarget();
    }

    void Editor::BeginGameRender(Window* _window)
    {
#if CANIS_EDITOR
        int targetWidth = (m_gameViewportWidth > 0) ? m_gameViewportWidth : _window->GetScreenWidth();
        int targetHeight = (m_gameViewportHeight > 0) ? m_gameViewportHeight : _window->GetScreenHeight();

        EnsureGameRenderTarget(targetWidth, targetHeight);
        if (m_gameFramebuffer == 0)
            return;

        glBindFramebuffer(GL_FRAMEBUFFER, m_gameFramebuffer);
        glViewport(0, 0, targetWidth, targetHeight);

        Color clear = _window->GetClearColor();
        glClearColor(clear.r, clear.g, clear.b, clear.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    }

    void Editor::EndGameRender(Window* _window)
    {
#if CANIS_EDITOR
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _window->GetScreenWidth(), _window->GetScreenHeight());
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
        DrawSceneView();
        DrawEditorPanel(); // draw last

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

#endif
    }

    void Editor::RenderGameDebug()
    {
#if CANIS_EDITOR
        if (!m_scene)
            return;

        if (m_index < 0 || m_index >= m_scene->GetEntities().size())
            return;

        if (m_scene->GetEntities()[m_index] == nullptr)
            return;

        Camera2D *camera2D = nullptr;
        std::vector<Entity *> &entities = m_scene->GetEntities();

        for (Entity *entity : entities)
        {
            if (entity == nullptr)
                continue;

            Camera2D *camera = entity->GetScript<Camera2D>();
            if (camera)
            {
                camera2D = camera;
                break;
            }
        }

        if (!camera2D)
            return;

        Entity &selected = *m_scene->GetEntities()[m_index];
        if (!selected.GetScript<RectTransform>())
            return;

        DrawBoundingBox(camera2D);
        DrawGizmo(camera2D);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
    }

    void Editor::DrawSceneView()
    {
        ImGui::Begin("Scene");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        int nextWidth = static_cast<int>(avail.x);
        int nextHeight = static_cast<int>(avail.y);
        bool hovered = false;

        if (nextWidth > 0 && nextHeight > 0)
        {
            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImGuiViewport *viewport = ImGui::GetWindowViewport();

            if (viewport)
                m_gameViewportId = viewport->ID;

            m_gameViewportPosX = cursor.x;
            m_gameViewportPosY = cursor.y;

            if (nextWidth != m_gameViewportWidth || nextHeight != m_gameViewportHeight)
            {
                m_gameViewportWidth = nextWidth;
                m_gameViewportHeight = nextHeight;
            }

            if (m_gameColorTexture != 0)
            {
                float targetW = static_cast<float>(m_window->GetScreenWidth());
                float targetH = static_cast<float>(m_window->GetScreenHeight());
                float targetAspect = targetW / targetH;
                float availAspect = (avail.y > 0.0f) ? (avail.x / avail.y) : targetAspect;

                ImVec2 drawSize = avail;
                if (availAspect > targetAspect) {
                    drawSize.x = avail.y * targetAspect; // letterbox left/right
                } else {
                    drawSize.y = avail.x / targetAspect; // letterbox top/bottom
                }

                ImVec2 cursor = ImGui::GetCursorPos();
                ImVec2 offset((avail.x - drawSize.x) * 0.5f, (avail.y - drawSize.y) * 0.5f);
                ImGui::SetCursorPos(ImVec2(cursor.x + offset.x, cursor.y + offset.y));

                ImGui::Image(
                    (ImTextureID)(intptr_t)m_gameColorTexture,
                    drawSize,
                    ImVec2(0.0f, 1.0f),
                    ImVec2(1.0f, 0.0f));
                hovered = ImGui::IsItemHovered();
                DrawSceneViewGizmo();
            }
            else
            {
                ImGui::Text("Game view unavailable.");
            }
        }
        else
        {
            m_gameViewportPosX = 0.0f;
            m_gameViewportPosY = 0.0f;
            m_gameViewportWidth = 0;
            m_gameViewportHeight = 0;
        }

        m_gameViewHovered = hovered;
        ImGui::End();
    }

    void Editor::DrawSceneViewGizmo()
    {
        if (m_gameViewportWidth <= 0 || m_gameViewportHeight <= 0)
            return;

        if (m_index < 0 || m_index >= m_scene->GetEntities().size())
            return;

        Entity *selected = m_scene->GetEntities()[m_index];
        if (!selected)
            return;

        RectTransform *rtc = selected->GetScript<RectTransform>();
        if (!rtc)
            return;

        Camera2D *camera2D = nullptr;
        std::vector<Entity *> &entities = m_scene->GetEntities();
        for (Entity *entity : entities)
        {
            if (!entity)
                continue;
            Camera2D *camera = entity->GetScript<Camera2D>();
            if (camera)
            {
                camera2D = camera;
                break;
            }
        }

        if (!camera2D)
            return;

        Matrix4 projection;
        projection.Identity();
        projection.Orthographic(0.0f,
                                static_cast<float>(m_gameViewportWidth),
                                0.0f,
                                static_cast<float>(m_gameViewportHeight),
                                0.0f,
                                100.0f);

        Vector2 pos = rtc->GetPosition();
        Vector2 globalScale = rtc->GetScale();

        Matrix4 model;
        model.Identity();
        model.Translate(Vector3(pos.x, pos.y, 0.0f));
        model.Rotate(rtc->rotation, Vector3(0.0f, 0.0f, 1.0f));
        model.Scale(Vector3(rtc->size.x * globalScale.x, rtc->size.y * globalScale.y, 1.0f));

        Matrix4 view;
        view.Identity();
        view.Translate(
            Vector3(
                -camera2D->GetPosition().x + m_gameViewportWidth / 2.0f,
                -camera2D->GetPosition().y + m_gameViewportHeight / 2.0f,
                0.0f));
        view.Scale(Vector3(camera2D->GetScale(), camera2D->GetScale(), 1.0f));

        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(m_gameViewportPosX, m_gameViewportPosY,
                          static_cast<float>(m_gameViewportWidth),
                          static_cast<float>(m_gameViewportHeight));
        ImGuizmo::SetOrthographic(true);
        ImGuizmo::Enable(true);

        static ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
        if (!ImGui::GetIO().WantTextInput)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W))
                operation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E))
                operation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R))
                operation = ImGuizmo::SCALE;
        }

        ImGuizmo::Manipulate(
            &view[0],
            &projection[0],
            operation,
            (ImGuizmo::MODE)m_guizmoMode,
            &model[0]);

        if (ImGuizmo::IsUsing())
        {
            float t[3], r[3], s[3];
            ImGuizmo::DecomposeMatrixToComponents(&model[0], t, r, s);

            Vector2 newPos(t[0], t[1]);
            Vector2 oldPos = rtc->GetPosition();
            rtc->Move(newPos - oldPos);

            rtc->rotation = DEG2RAD * r[2];

            rtc->SetScale(Vector2(s[0] / rtc->size.x, s[1] / rtc->size.y));
        }
    }

    void Editor::EnsureGameRenderTarget(int _width, int _height)
    {
        if (_width <= 0 || _height <= 0)
            return;

        if (m_gameFramebuffer != 0 &&
            _width == m_gameTextureWidth &&
            _height == m_gameTextureHeight)
        {
            return;
        }

        DestroyGameRenderTarget();

        glGenFramebuffers(1, &m_gameFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_gameFramebuffer);

        glGenTextures(1, &m_gameColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_gameColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gameColorTexture, 0);

        glGenRenderbuffers(1, &m_gameDepthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_gameDepthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_gameDepthRbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Debug::Log("Game framebuffer incomplete.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_gameTextureWidth = _width;
        m_gameTextureHeight = _height;
    }

    void Editor::DestroyGameRenderTarget()
    {
        if (m_gameDepthRbo != 0)
        {
            glDeleteRenderbuffers(1, &m_gameDepthRbo);
            m_gameDepthRbo = 0;
        }

        if (m_gameColorTexture != 0)
        {
            glDeleteTextures(1, &m_gameColorTexture);
            m_gameColorTexture = 0;
        }

        if (m_gameFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_gameFramebuffer);
            m_gameFramebuffer = 0;
        }

        m_gameTextureWidth = 0;
        m_gameTextureHeight = 0;
    }

    void Editor::FocusEntity(Canis::Entity *_entity)
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

    void Editor::InputEntity(const std::string &_name, Canis::Entity *&_variable)
    {
        ImGui::Text("%s", _name.c_str());
        ImGui::SameLine();

        std::string label;
        Canis::Entity *entity = *&_variable;
        if (entity)
            label = "[entity] " + entity->name;
        else
            label = "[ missing entity ]";

        ImGui::Button(label.c_str(), ImVec2(150, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
            {
                const Canis::UUID dropped = *static_cast<const Canis::UUID *>(payload->Data);
                Canis::Entity *e = m_scene->GetEntityWithUUID(dropped);

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

    bool Editor::IsDescendantOf(Canis::Entity *_parent, Canis::Entity *_potentialChild)
    {
        if (!_parent || !_potentialChild)
            return false;

        auto *parentRT = _parent->GetScript<RectTransform>();
        if (!parentRT)
            return false;

        for (auto *child : parentRT->children)
        {
            if (!child)
                continue;
            if (child == _potentialChild)
                return true;
            if (IsDescendantOf(child, _potentialChild))
                return true;
        }

        return false;
    }

    void GetRectTransformChildren(Canis::Entity* _entity, std::vector<Canis::Entity*> &_entities)
    {
        Canis::RectTransform* transform = _entity->GetScript<Canis::RectTransform>();

        for (int i = 0; i < transform->children.size(); i++)
        {
            Canis::Entity* e = transform->children[i];
            _entities.push_back(e);

            GetRectTransformChildren(e, _entities);
        }
    }

    void Editor::DrawHierarchyNode(Canis::Entity *_entity, std::vector<Canis::Entity *> &_entities, bool &_refresh)
    {
        if (!_entity)
            return;

        auto *rt = _entity->GetScript<RectTransform>();

        bool isSelected = (m_index >= 0 && m_index < (int)_entities.size() &&
                           _entities[m_index] == _entity);

        bool hasChildren = (rt && !rt->children.empty());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;
        if (isSelected)
            flags |= ImGuiTreeNodeFlags_Selected;

        std::string label = _entity->name + "##" + std::to_string(_entity->uuid);
        bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

        // select on click
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            for (int i = 0; i < (int)_entities.size(); ++i)
            {
                if (_entities[i] == _entity)
                {
                    m_index = i;
                    _refresh = true;
                    break;
                }
            }
        }

        // drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            Canis::UUID uuid = _entity->uuid;
            ImGui::SetDragDropPayload("ENTITY_DRAG", &uuid, sizeof(Canis::UUID));
            ImGui::Text("Entity: %s", _entity->name.c_str());
            ImGui::EndDragDropSource();
        }

        // drop ON node = parent and append at end
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
            {
                Canis::UUID droppedUUID = *static_cast<const Canis::UUID *>(payload->Data);

                Canis::Entity *droppedEntity = nullptr;
                for (auto *e : _entities)
                {
                    if (e && e->uuid == droppedUUID)
                    {
                        droppedEntity = e;
                        break;
                    }
                }

                if (droppedEntity && droppedEntity != _entity)
                {
                    auto *droppedRT = droppedEntity->GetScript<RectTransform>();
                    auto *targetRT = _entity->GetScript<RectTransform>();

                    if (droppedRT && targetRT && !IsDescendantOf(droppedEntity, _entity))
                    {
                        // append as last child
                        droppedRT->SetParent(_entity);
                        _refresh = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        // context menu
        if (ImGui::BeginPopupContextItem())
        {
            int idx = -1;
            for (int i = 0; i < (int)_entities.size(); ++i)
            {
                if (_entities[i] == _entity)
                {
                    idx = i;
                    break;
                }
            }

            if (ImGui::MenuItem("Create"))
                m_scene->CreateEntity();

            if (idx >= 0 && ImGui::MenuItem("Duplicate"))
            {
                Debug::Log("Duplicate");
                Canis::Entity *selected = _entities[idx];

                std::vector<Canis::Entity*> entities;
                entities.push_back(selected);

                // get all entities to duplicate
                if (selected->GetScript<Canis::RectTransform>())
                {
                    GetRectTransformChildren(selected, entities);
                }

                // encode entities into sequence of nodes
                YAML::Node nodes;
                for (Canis::Entity* e : entities)
                {
                    nodes.push_back(m_scene->EncodeEntity(m_app->GetScriptRegistry(), *e));
                }

                // option to tell it to generate new UUIDS
                m_scene->LoadEntityNodes(m_app->GetScriptRegistry(), nodes, false);
            }

            if (idx >= 0 && ImGui::MenuItem("Remove"))
            {
                m_scene->Destroy(idx);
                if (m_index == idx)
                    m_index = -1;
                _refresh = true;
                if (nodeOpen)
                    ImGui::TreePop();
                ImGui::EndPopup();
                return;
            }

            for (auto &item : m_app->GetInspectorItemRegistry())
            {
                if (ImGui::MenuItem((item.name + "##").c_str()))
                    item.Func(*m_app, *this, *_entity, m_app->GetScriptRegistry());
            }

            ImGui::EndPopup();
        }

        // children + single per-gap drop slots
        if (nodeOpen)
        {
            if (rt)
            {
                auto &children = rt->children;

                for (std::size_t ci = 0; ci < children.size(); ++ci)
                {
                    Canis::Entity *child = children[ci];
                    if (!child)
                        continue;

                    // drop BEFORE this child -> specific index
                    ImGui::PushID((void *)((uintptr_t)child ^ 0xBEEF));
                    {
                        ImVec2 slotSize(ImGui::GetContentRegionAvail().x, 1.0f);
                        ImGui::InvisibleButton("##drop_before", slotSize);

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
                            {
                                Canis::UUID droppedUUID = *static_cast<const Canis::UUID *>(payload->Data);

                                Canis::Entity *droppedEntity = nullptr;
                                for (auto *e2 : _entities)
                                {
                                    if (e2 && e2->uuid == droppedUUID)
                                    {
                                        droppedEntity = e2;
                                        break;
                                    }
                                }

                                if (droppedEntity && droppedEntity != _entity)
                                {
                                    auto *droppedRT = droppedEntity->GetScript<RectTransform>();
                                    auto *targetRT = _entity->GetScript<RectTransform>();

                                    if (droppedRT && targetRT && !IsDescendantOf(droppedEntity, _entity))
                                    {
                                        droppedRT->SetParentAtIndex(_entity, ci);
                                        _refresh = true;
                                    }
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }
                    ImGui::PopID();

                    // actual child row
                    DrawHierarchyNode(child, _entities, _refresh);
                }
            }

            ImGui::TreePop();
        }
    }

    bool Editor::DrawHierarchyPanel()
    {
        ImGui::Begin("Hierarchy");
        bool refresh = false;

        std::vector<Canis::Entity *> &entities = m_scene->GetEntities();

        // Build list of root entities + their indices in 'entities'
        std::vector<Canis::Entity *> rootEntities;
        std::vector<int> rootIndices;

        for (int i = 0; i < (int)entities.size(); ++i)
        {
            Canis::Entity *entity = entities[i];
            if (!entity)
                continue;

            auto *rt = entity->GetScript<RectTransform>();

            // If it has a RectTransform and a parent, it's not a root
            if (rt && rt->parent != nullptr)
                continue;

            rootEntities.push_back(entity);
            rootIndices.push_back(i);
        }

        auto moveRootToPos = [&](Canis::Entity *droppedEntity, int targetRootPos)
        {
            if (!droppedEntity)
                return;

            // If it was a child, unparent it first so it becomes a root
            if (auto *droppedRT = droppedEntity->GetScript<RectTransform>())
            {
                if (droppedRT->parent != nullptr)
                    droppedRT->SetParent(nullptr);
            }

            // Find its index in the flat entities array
            int from = -1;
            for (int i = 0; i < (int)entities.size(); ++i)
            {
                if (entities[i] == droppedEntity)
                {
                    from = i;
                    break;
                }
            }
            if (from == -1)
                return;

            // Clamp targetRootPos
            if (targetRootPos < 0)
                targetRootPos = 0;
            if (targetRootPos > (int)rootEntities.size())
                targetRootPos = (int)rootEntities.size();

            int to;
            if (targetRootPos == (int)rootEntities.size())
            {
                // After last root
                to = rootIndices.empty() ? (int)entities.size()
                                         : rootIndices.back() + 1;
                if (to > (int)entities.size())
                    to = (int)entities.size();
            }
            else
            {
                to = rootIndices[targetRootPos];
            }

            Canis::Entity *ptr = entities[from];
            entities.erase(entities.begin() + from);

            if (from < to)
                --to;
            if (to < 0)
                to = 0;
            if (to > (int)entities.size())
                to = (int)entities.size();

            entities.insert(entities.begin() + to, ptr);
            refresh = true;
        };

        // ---------- TOP ROOT DROP SLOT (move to front) ----------
        {
            ImVec2 slotSize(ImGui::GetContentRegionAvail().x, 1.0f);
            ImGui::InvisibleButton("##root_drop_before_first", slotSize);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
                {
                    Canis::UUID droppedUUID = *static_cast<const Canis::UUID *>(payload->Data);
                    Canis::Entity *droppedEntity = nullptr;
                    for (auto *e : entities)
                    {
                        if (e && e->uuid == droppedUUID)
                        {
                            droppedEntity = e;
                            break;
                        }
                    }
                    moveRootToPos(droppedEntity, 0); // move to first root
                }
                ImGui::EndDragDropTarget();
            }
        }

        // ---------- ROOT ENTITIES + BETWEEN-SLOTS ----------
        for (int ri = 0; ri < (int)rootEntities.size(); ++ri)
        {
            Canis::Entity *entity = rootEntities[ri];
            if (!entity)
                continue;

            DrawHierarchyNode(entity, entities, refresh);

            // drop slot AFTER this root -> position ri+1
            ImGui::PushID((void *)((uintptr_t)entity ^ 0xABCDEF));
            ImVec2 slotSize(ImGui::GetContentRegionAvail().x, 1.0f);
            ImGui::InvisibleButton("##root_drop_after", slotSize);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
                {
                    Canis::UUID droppedUUID = *static_cast<const Canis::UUID *>(payload->Data);
                    Canis::Entity *droppedEntity = nullptr;
                    for (auto *e : entities)
                    {
                        if (e && e->uuid == droppedUUID)
                        {
                            droppedEntity = e;
                            break;
                        }
                    }
                    moveRootToPos(droppedEntity, ri + 1);
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
        }

        // --------- ROOT DROP ZONE (unparent children) ----------
        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.y < 24.0f)
            avail.y = 24.0f;

        ImGui::InvisibleButton("##hierarchy_root_drop_zone", avail);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY_DRAG"))
            {
                Canis::UUID droppedUUID = *static_cast<const Canis::UUID *>(payload->Data);

                Canis::Entity *droppedEntity = nullptr;
                for (auto *e : entities)
                {
                    if (e && e->uuid == droppedUUID)
                    {
                        droppedEntity = e;
                        break;
                    }
                }

                if (droppedEntity)
                {
                    if (auto *droppedRT = droppedEntity->GetScript<RectTransform>())
                    {
                        // Only unparent if it *has* a parent
                        if (droppedRT->parent != nullptr)
                        {
                            droppedRT->SetParent(nullptr);
                            refresh = true;
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        // -----------------------------------------------------

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
        ImGui::Text("in-game fps limit");
        ImGui::SameLine();
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
            ImGui::Text("    fps limit");
            ImGui::SameLine();
            if (ImGui::InputInt("##frameLimit", &Canis::GetProjectConfig().frameLimit, 0) && m_mode == EditorMode::PLAY)
                Time::SetTargetFPS(Canis::GetProjectConfig().frameLimit + 0.0f);
        }

        // editor fps limit input
        ImGui::Text("editor fps");
        ImGui::SameLine();
        if (ImGui::InputInt("##editorframeLimit", &Canis::GetProjectConfig().frameLimitEditor, 0) && m_mode == EditorMode::EDIT)
            Time::SetTargetFPS(Canis::GetProjectConfig().frameLimitEditor + 0.0f);

        // application icon
        ImGui::Text("icon");
        ImGui::SameLine();
        ImGui::Button(
            AssetManager::GetMetaFile(AssetManager::GetPath(Canis::GetProjectConfig().iconUUID))->name.c_str(),
            ImVec2(150, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_DRAG"))
            {
                const AssetDragData dropped = *static_cast<const AssetDragData *>(payload->Data);
                std::string path = AssetManager::GetPath(dropped.uuid);
                TextureAsset *asset = AssetManager::GetTexture(path);

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

    void Editor::DrawEditorPanel()
    {
        static YAML::Node lastSceneNode;
        static float hotKeyCoolDown = 0.0f;
        const float HOTKEYRESET = 0.1f;

        ImGui::Begin("Canis Editor");

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

        ImGui::SameLine();
        ImGui::Text("Entity Count: %zu", m_scene->GetEntities().size());

        ImGui::End();
    }

    void Editor::SelectSprite2D()
    {
        if (ImGuizmo::IsOver() || ImGuizmo::IsUsing())
            return;

        if (!m_gameViewHovered || m_gameViewportWidth <= 0 || m_gameViewportHeight <= 0)
            return;

        if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            return;

        ImVec2 mousePos = ImGui::GetMousePos();
        float localX = mousePos.x - m_gameViewportPosX;
        float localY = mousePos.y - m_gameViewportPosY;
        Vector2 mouse(localX, m_gameViewportHeight - localY);

        mouse = mouse - (Vector2(m_gameViewportWidth, m_gameViewportHeight) / 2.0f);

        Vector2 camPos(0.0f);
        float camScale = 1.0f;
        std::vector<Entity *> &entities = m_scene->GetEntities();
        for (Entity *entity : entities)
        {
            if (!entity)
                continue;
            if (Camera2D *camera = entity->GetScript<Camera2D>())
            {
                camPos = camera->GetPosition();
                camScale = camera->GetScale();
                break;
            }
        }

        if (camScale != 0.0f)
            mouse = (mouse / camScale) + camPos;

        bool mouseLock = false;

        for (int i = 0; i < m_scene->GetEntities().size(); i++)
        {
            if (m_scene->GetEntities()[i] == nullptr)
                continue;

            RectTransform *transform = m_scene->GetEntities()[i]->GetScript<RectTransform>();

            if (transform == nullptr)
                continue;

            Vector2 globalPos = transform->GetPosition();    // rect_transform.GetGlobalPosition(window->GetScreenWidth(), window->GetScreenHeight());
            float globalRotation = transform->GetRotation(); // rect_transform.GetGlobalRotation();

            if (globalRotation != 0.0f)
            {
                RotatePointAroundPivot(
                    mouse,
                    globalPos /* + rect_transform.rotationOriginOffset */,
                    -globalRotation);
            }

            Vector2 globalScale = transform->GetScale();
            if (mouse.x > globalPos.x - transform->size.x * 0.5f * globalScale.x &&
                mouse.x < globalPos.x + transform->size.x * 0.5f * globalScale.x &&
                mouse.y > globalPos.y - transform->size.x * 0.5f * globalScale.y &&
                mouse.y < globalPos.y + transform->size.x * 0.5f * globalScale.y &&
                !mouseLock)
            {
                m_index = i;
            }
        }
    }

    void Editor::DrawGizmo(Camera2D *_camera2D)
    {
        ImGuiContext *ctx = ImGui::GetCurrentContext();
        if (!ctx || !ctx->WithinFrameScope)
            return;

        if (m_gameViewportWidth <= 0 || m_gameViewportHeight <= 0)
            return;

        ImGui::SetNextWindowPos(ImVec2(m_gameViewportPosX, m_gameViewportPosY));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_gameViewportWidth), static_cast<float>(m_gameViewportHeight)));
        if (m_gameViewportId != 0)
            ImGui::SetNextWindowViewport(m_gameViewportId);

        ImGuizmo::BeginFrame();

        // Use the Game viewport draw list so the gizmo appears in the correct platform window.
        ImDrawList *drawList = ImGui::GetForegroundDrawList();
        if (m_gameViewportId != 0)
        {
            if (ImGuiViewport *viewport = ImGui::FindViewportByID(m_gameViewportId))
                drawList = ImGui::GetForegroundDrawList(viewport);
        }
        ImGuizmo::SetDrawlist(drawList);

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
        projection.Orthographic(0.0f,
                                static_cast<float>(m_gameViewportWidth),
                                0.0f,
                                static_cast<float>(m_gameViewportHeight),
                                0.0f,
                                100.0f);

        Entity &debugRectTransformEntity = *m_scene->GetEntities()[m_index];

        RectTransform &rtc = *debugRectTransformEntity.GetScript<RectTransform>();
        Vector2 pos = rtc.GetPosition();

        // Align to bottom-left
        // ADD BACK pos += rtc.rotationOriginOffset;
        Vector2 globalScale = rtc.GetScale();

        Matrix4 model;
        model.Identity();
        model.Translate(Vector3(pos.x, pos.y, 0.0f));
        model.Rotate(rtc.rotation, Vector3(0.0f, 0.0f, 1.0f));
        model.Scale(Vector3(rtc.size.x * globalScale.x, rtc.size.y * globalScale.y, 1.0f));

        ImGuizmo::SetOrthographic(true);
        ImGuizmo::SetRect(m_gameViewportPosX, m_gameViewportPosY,
                          static_cast<float>(m_gameViewportWidth),
                          static_cast<float>(m_gameViewportHeight));
        ImGuizmo::Enable(true);

        Matrix4 view;
        view.Identity();
        view.Translate(
            Vector3(
                -_camera2D->GetPosition().x + m_gameViewportWidth / 2.0f,
                -_camera2D->GetPosition().y + m_gameViewportHeight / 2.0f,
                0.0f));
        view.Scale(Vector3(_camera2D->GetScale(), _camera2D->GetScale(), 0.0f));

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
            rtc.SetScale(Vector2(scale.x / rtc.size.x, scale.y / rtc.size.y));
        }

    }

    void Editor::DrawBoundingBox(Camera2D *_camera2D)
    {
        Matrix4 projection;
        projection.Identity();

        projection = _camera2D->GetCameraMatrix();

        static Canis::Shader debugLineShader("assets/shaders/debug_line.vs", "assets/shaders/debug_line.fs");
        Entity &debugRectTransformEntity = *m_scene->GetEntities()[m_index];
        RectTransform &rtc = *debugRectTransformEntity.GetScript<RectTransform>();
        Vector2 pos = rtc.GetPosition();
        Vector2 scale = rtc.GetScale();
        // Vector2 vertices[] = {
        //     {pos.x, pos.y},
        //     {pos.x + (rtc.size.x * rtc.scale), pos.y},
        //     {pos.x + (rtc.size.x * rtc.scale), pos.y + (rtc.size.y * rtc.scale)},
        //     {pos.x, pos.y + (rtc.size.y * rtc.scale)}};
        Vector2 vertices[] = {
            {pos.x - (rtc.size.x * scale.x * 0.5f), pos.y - (rtc.size.y * scale.y * 0.5f)},
            {pos.x + (rtc.size.x * scale.x * 0.5f), pos.y - (rtc.size.y * scale.y * 0.5f)},
            {pos.x + (rtc.size.x * scale.x * 0.5f), pos.y + (rtc.size.y * scale.y * 0.5f)},
            {pos.x - (rtc.size.x * scale.x * 0.5f), pos.y + (rtc.size.y * scale.y * 0.5f)}};

        for (Vector2 &v : vertices)
            RotatePointAroundPivot(
                v,
                Vector2(pos.x, pos.y) /*vertices[0] + rtc.originOffset + rtc.rotationOriginOffset*/,
                -rtc.GetRotation() // debugRectTransform.GetGlobalRotation()
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

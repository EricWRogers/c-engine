#include <Canis/Editor.hpp>

#include <Canis/Debug.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Window.hpp>
#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>
#include <Canis/App.hpp>
#include <Canis/Shader.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <ImGuizmo.h>

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
#endif
    }

    void Editor::Draw(Scene *_scene, Window *_window, App *_app /*, Time *_time*/)
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

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // ImGui::DockSpaceOverViewport();

        bool refresh = DrawHierarchyPanel();
        DrawInspectorPanel(refresh);
        DrawScenePanel();
        // DrawSystemPanel();

        ImGui::Begin("Environment");
        Color background = _window->GetClearColor();
        ImGui::ColorEdit4("Background##", &background.r);

        if (background != _window->GetClearColor())
            _window->SetClearColor(background);

        ImGui::End();

        // DrawScenePanel(_window, _time);

        /*// rendering
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
        }*/

        m_debugDraw = DebugDraw::NONE;
        Camera2D *camera2D = nullptr;
        // ADD SIZE BACK TO RECTTRANSFORM

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

        if (m_debugDraw == DebugDraw::RECT && m_scene->GetEntities()[m_index] != nullptr)
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

            projection = camera2D->GetProjectionMatrix();

            if (camera2D == nullptr)
                Debug::Log("NULL");

            Entity &debugRectTransformEntity = *m_scene->GetEntities()[m_index];

            RectTransform &rtc = *debugRectTransformEntity.GetScript<RectTransform>();
            Vector2 pos = rtc.position; // ADD BACK rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight());
            pos += rtc.originOffset;

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
                    camera2D->GetPosition().x + _window->GetScreenWidth() / 2.0f,
                    camera2D->GetPosition().y + _window->GetScreenHeight() / 2.0f,
                    0.0f));
            view.Scale(Vector3(camera2D->GetScale()));

            ImGuizmo::Manipulate(
                &view[0],
                &projection[0],
                operation,
                ImGuizmo::LOCAL,
                &model[0]);

            Vector2 cameraPos = camera2D->GetPosition();
            float cameraRot = camera2D->GetScale();

            if (ImGuizmo::IsUsing())
            {
                float t[3], r[3], s[3];
                ImGuizmo::DecomposeMatrixToComponents(&model[0], t, r, s);

                Vector3 translation(t[0], t[1], t[2]), rotation(r[0], r[1], r[2]), scale(s[0], s[1], s[2]);

                // update position
                Vector2 newPos(translation.x, translation.y);
                Vector2 oldPos = rtc.position + rtc.originOffset;
                // ADD BACK Vector2 oldPos = rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight()) + rtc.originOffset;
                // ADD BACK oldPos += rtc.rotationOriginOffset;
                rtc.position += newPos - oldPos;

                // update rotation
                rtc.rotation = DEG2RAD * rotation.z;

                // update size (scale stays constant, we resize the actual size)
                rtc.scale = Vector2(scale.x / rtc.size.x, scale.y / rtc.size.y);
            }

            ImGui::End();
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

        // debug draw
        if (m_debugDraw == DebugDraw::RECT && m_scene->GetEntities()[m_index] != nullptr)
        {
            Matrix4 projection;
            projection.Identity();

            projection = camera2D->GetCameraMatrix();

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

        // Save
        // if (m_mode == EditorMode::EDIT && GetSceneManager().inputManager->JustPressedKey(SDLK_F5))
        //{
        //    GetSceneManager().Save();
        //}
        //}
#endif
    }

    bool Editor::DrawHierarchyPanel()
    {
        ImGui::Begin("Hierarchy");
        bool refresh = false;

        std::vector<Entity *> &entities = m_scene->GetEntities();

        for (int i = 0; i < entities.size(); i++)
        {
            if (entities[i] == nullptr)
                continue;
            
            // ImGui::Text("%s", entities[i]->name.c_str());
            std::string inputID = entities[i]->name + "##input" + std::to_string(i);
            ImGui::Selectable(inputID.c_str(), m_index == i);

            if (ImGui::IsItemFocused())
            {
                m_index = i;
                refresh = true;
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
                    //Entity *entity = m_scene->CreateEntity();
                    //entity->name = selected->name; // ++ a number at the end
                    //entity->tag = selected->tag;

                    // encode
                    YAML::Node node = m_scene->EncodeEntity(m_app->GetScriptRegistry(), *selected);

                    // decode
                    Entity& entity = m_scene->DecodeEntity(m_app->GetScriptRegistry(), node, false);
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

        if (entities.size() != 0 && entities[m_index] != nullptr)
        {            
            Clamp(m_index, 0, entities.size() - 1);

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


    void Editor::DrawScenePanel()
    {
        ImGui::Begin("Scene");
        if (ImGui::Button("Save##ScenePanel")) {
            m_scene->Save(m_app->GetScriptRegistry());
        }
        ImGui::SameLine();
        ImGui::Text("FPS: %s", std::to_string(m_app->FPS()).c_str());
        ImGui::End();
    }

}

#include <Canis/Editor.hpp>

#include <Canis/Debug.hpp>
#include <Canis/OpenGL.hpp>
#include <Canis/Window.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

namespace Canis
{
    void Editor::Init(Window *_window)
    {
#if CANIS_EDITOR
        //if (GetProjectConfig().editor)
        //{
            // Setup Dear ImGui context
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

            // Setup Platform/Renderer backends
            ImGui_ImplSDL3_InitForOpenGL((SDL_Window *)_window->GetSDLWindow(), (SDL_GLContext)_window->GetGLContext());
            ImGui_ImplOpenGL3_Init(OPENGLVERSION);
        //}
#endif
    }

    void Editor::Draw(Scene *_scene, Window *_window/*, Time *_time*/)
    {
#if CANIS_EDITOR
        //if (GetProjectConfig().editor)
        //{
            Debug::Log("Editor::Draw");
            if (m_scene != _scene)
            {
                Debug::Log("new scene");
            }
            m_scene = _scene;

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            //DrawInspectorPanel();
            //DrawSystemPanel();
            DrawHierarchyPanel();
            //DrawScenePanel(_window, _time);

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

            //ImGui::EndFrame();

            /*if (m_debugDraw == DebugDraw::RECT)
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
                
                if (ImGui::IsKeyPressed(ImGuiKey_W))
                    operation = ImGuizmo::TRANSLATE;
                
                if (ImGui::IsKeyPressed(ImGuiKey_E))
                    operation = ImGuizmo::ROTATE;
                
                if (ImGui::IsKeyPressed(ImGuiKey_R))
                    operation = ImGuizmo::SCALE;

                Camera2D camera2D;
                camera2D.Init((int)_window->GetScreenWidth(), (int)_window->GetScreenHeight());

                bool camFound = false;
                auto cam = _scene->entityRegistry.view<const Camera2DComponent>();
                for (auto [entity, camera] : cam.each())
                {
                    camera2D.SetPosition(camera.position);
                    camera2D.SetScale(camera.scale);
                    camera2D.Update();
                    camFound = true;
                    break;
                }

                glm::mat4 projection = camFound
                                           ? camera2D.GetProjectionMatrix()
                                           : glm::ortho(0.0f, static_cast<float>(_window->GetScreenWidth()), 0.0f, static_cast<float>(_window->GetScreenHeight()));

                RectTransform &rtc = debugRectTransformEntity.GetComponent<RectTransform>();
                glm::vec2 pos = rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight());
                pos += rtc.originOffset;

                // Align to bottom-left
                pos += rtc.rotationOriginOffset;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(pos, 0.0f));
                model = glm::rotate(model, -rtc.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, glm::vec3(rtc.size * rtc.scale, 1.0f)); // Scale affects size

                ImGuizmo::SetOrthographic(true);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(mainViewport->WorkPos.x, mainViewport->WorkPos.y, mainViewport->WorkSize.x, mainViewport->WorkSize.y);
                ImGuizmo::Enable(true);

                glm::mat4 view = camera2D.GetViewMatrix();

                ImGuizmo::Manipulate(
                    glm::value_ptr(view),
                    glm::value_ptr(projection),
                    operation,
                    ImGuizmo::LOCAL,
                    glm::value_ptr(model));

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

                    // update position
                    glm::vec2 newPos = glm::vec2(translation.x, translation.y);
                    glm::vec2 oldPos = rtc.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight()) + rtc.originOffset;
                    oldPos += rtc.rotationOriginOffset;
                    rtc.position += newPos - oldPos;

                    // update rotation
                    rtc.rotation = -glm::radians(rotation.z);

                    // update size (scale stays constant, we resize the actual size)
                    rtc.size = glm::vec2(scale.x, scale.y) / rtc.scale;
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
            if (m_debugDraw == DebugDraw::RECT)
            {
                Camera2D camera2D;
                camera2D.Init((int)_window->GetScreenWidth(), (int)_window->GetScreenHeight());
                bool camFound = false;
                auto cam = _scene->entityRegistry.view<const Camera2DComponent>();
                for (auto [entity, camera] : cam.each())
                {
                    camera2D.SetPosition(camera.position);
                    camera2D.SetScale(camera.scale);
                    camera2D.Update();
                    camFound = true;
                    continue;
                }

                glm::mat4 projection = glm::mat4(1.0f);

                if (camFound)
                    projection = camera2D.GetCameraMatrix();
                else
                    projection = glm::ortho(0.0f, static_cast<float>(_window->GetScreenWidth()), 0.0f, static_cast<float>(_window->GetScreenHeight()));

                static Canis::Shader debugLineShader("assets/shaders/debug_line.vs", "assets/shaders/debug_line.fs");
                RectTransform &debugRectTransform = debugRectTransformEntity.GetComponent<RectTransform>();
                glm::vec2 pos = debugRectTransform.GetGlobalPosition(_window->GetScreenWidth(), _window->GetScreenHeight());
                pos += debugRectTransform.originOffset;
                glm::vec2 vertices[] = {
                    {pos.x, pos.y},
                    {pos.x + (debugRectTransform.size.x * debugRectTransform.scale), pos.y},
                    {pos.x + (debugRectTransform.size.x * debugRectTransform.scale), pos.y + (debugRectTransform.size.y * debugRectTransform.scale)},
                    {pos.x, pos.y + (debugRectTransform.size.y * debugRectTransform.scale)}};
                
                for (glm::vec2 &v : vertices)
                    RotatePointAroundPivot(
                        v,
                        vertices[0] + debugRectTransform.originOffset + debugRectTransform.rotationOriginOffset,
                        debugRectTransform.GetGlobalRotation()
                    );

                for (glm::vec2 &v : vertices)
                    v = glm::vec2(projection * glm::vec4(v.x, v.y, 0.0f, 1.0f));

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
            if (m_mode == EditorMode::EDIT && GetSceneManager().inputManager->JustPressedKey(SDLK_F5))
            {
                GetSceneManager().Save();
            }*/
        //}
#endif
    }

    void Editor::DrawHierarchyPanel()
    {
        ImGui::Begin("Hierarchy");

        ImGui::Text("Hello World!");

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
    }

}
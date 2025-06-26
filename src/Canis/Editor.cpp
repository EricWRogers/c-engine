#include "Editor.hpp"

#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

namespace Canis
{
    Editor::Editor(Window *_window, World *_world, InputManager *_inputManager)
    {
        m_window = _window;
        m_world = _world;
        m_camera = &(m_world->GetCamera());
        m_inputManager = _inputManager;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        const char *glsl_version = "#version 330";
        ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)m_window->GetSDLWindow(), (SDL_GLContext)m_window->GetGLContext());
        ImGui_ImplOpenGL3_Init(glsl_version);

        // input buffer
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // Create a texture to store the entity IDs
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_window->GetScreenWidth(), m_window->GetScreenHeight(), 0, GL_RED_INTEGER, GL_INT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

        // Create a renderbuffer object for depth and stencil attachment
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_window->GetScreenWidth(), m_window->GetScreenHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Set up id shader
        m_idShader.Compile("assets/shaders/id_shader.vs", "assets/shaders/id_shader.fs");
        m_idShader.AddAttribute("model");
        m_idShader.AddAttribute("view");
        m_idShader.AddAttribute("projection");
        m_idShader.Link();
    }

    void Editor::Draw()
    {
        if (m_inputManager->LeftClickReleased())
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mat4 project = mat4(1.0f);
            project = perspective(radians(45.0f),
                              (float)m_window->GetScreenWidth() / (float)m_window->GetScreenHeight(),
                              0.01f, 100.0f);

            // Set up your shaders and matrices
            m_idShader.Use();
            m_idShader.SetMat4("view", m_camera->GetViewMatrix());
            m_idShader.SetMat4("projection", project);

            // Render each entity with its unique ID
            auto& entities = m_world->GetEntities();
            int size = entities.size();
            for (int i = 0; i < size; i++) {
                m_idShader.SetMat4("model", entities[i].transform.Matrix());
                m_idShader.SetInt("entityID", i);
                Canis::Draw(*entities[i].model);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0);

            std::vector<int> idBuffer(m_window->GetScreenWidth() * m_window->GetScreenHeight());
            glReadPixels(0, 0, m_window->GetScreenWidth(), m_window->GetScreenHeight(), GL_RED_INTEGER, GL_INT, idBuffer.data());
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            int index = m_inputManager->mouse.y * m_window->GetScreenWidth() + m_inputManager->mouse.x;

            if (index >= 0 && index < idBuffer.size())
                if (idBuffer[index] >= 0 && idBuffer[index] < size)
                    m_index = idBuffer[index];
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //ImGui::ShowDemoWindow(&showExtra);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            int count = m_world->GetEntitiesSize();

            if (count == 0)
                return;

            Entity *entity = m_world->GetEntity(m_index);

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            if (ImGui::Button("Back"))
            {
                m_index--;

                if (m_index < 0)
                    m_index = count - 1;
            }
            ImGui::SameLine();
            ImGui::Text("Entity ID: %d", m_index);
            ImGui::SameLine();
            if (ImGui::Button("Next"))
            {
                m_index++;

                if (m_index >= count)
                    m_index = 0;
            }

            ImGui::Checkbox("active", &(entity->active));
            ImGui::InputText("tag", &(entity->tag));

            if (ImGui::CollapsingHeader("Transform"))
            {
                ImGui::InputFloat3("Position", glm::value_ptr(entity->transform.position), "%.3f");
                ImGui::InputFloat3("Rotation", glm::value_ptr(entity->transform.rotation));
                ImGui::InputFloat3("Scale", glm::value_ptr(entity->transform.scale));
            }

            if (ImGui::CollapsingHeader("Material"))
            {
                ImGui::InputFloat3("Color", glm::value_ptr(entity->color));
            }

            // ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        // glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // SDL_GL_SwapWindow((SDL_Window*)m_window->GetSDLWindow());
    }
} // end of Canis namespace
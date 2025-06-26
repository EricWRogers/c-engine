#pragma once

#include "World.hpp"
#include "Window.hpp"

namespace Canis
{
class Editor {
public:
    Editor(Window *_window, World *_world, InputManager *_inputManager);
    void Draw();
private:
    Window *m_window;
    World *m_world;
    Camera *m_camera;
    InputManager *m_inputManager;
    bool showExtra = true;
    int m_index = 0;
    unsigned int m_fbo, m_texture, m_rbo;
    Canis::Shader m_idShader;
};
} // end of Canis namespace
#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>

namespace Canis {

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    // Polls for SDL events and updates internal state
    void PollEvents();
    // Returns true if a quit event was received
    bool ShouldClose() const;

    // Clears the screen to the given color
    void Clear(float r, float g, float b, float a) const;
    // Renders the triangle
    void Render() const;
    // Swaps the buffers
    void Display() const;

private:
    SDL_Window*      m_window     = nullptr;
    SDL_GLContext    m_context    = nullptr;
    bool             m_shouldClose = false;

    GLuint           m_program    = 0;
    GLuint           m_vbo        = 0;

    void InitGL();
    GLuint CompileShader(GLenum type, const char* source);
    void SetupTriangle();
};

} // namespace Canis

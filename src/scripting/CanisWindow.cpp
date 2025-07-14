#include "CanisWindow.hpp"
#include "canis/Window.hpp"

void CanisWindow::SetTitle(const char *_title)
{
    Canis::Window::SetWindowName(std::string(_title));
}

void CanisWindow::SetWindowSize(int _width, int _height)
{
    Canis::Window::SetWindowSize(_width, _height);
}

void CanisWindow::SetBackgroundColor(float _red, float _green, float _blue, float _alpha)
{
    Canis::Window::SetClearColor(glm::vec4(_red, _green, _blue, _alpha));
    Canis::Window::ClearColor();
}

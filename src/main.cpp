

#include <iostream>
#include <string>

#include "canis/Canis.hpp"
#include "canis/Debug.hpp"
#include "canis/Script.hpp"
#include "canis/Window.hpp"

int main() {
    Canis::Init();
    
    Canis::Window::Create("Test", 300, 300, 0);
    Canis::Window::SetClearColor(glm::vec4(1.0f));
    Canis::Window::ClearColor();
    Canis::Window::SwapBuffer();
    Canis::Window::ClearColor();
    Canis::Window::SetWindowName("Yes");

    Script testScript;
    testScript.Init("assets/GameScripts.dll");
    testScript.Start();
    testScript.Update();
    testScript.Destroy();

    
    return 0;
}

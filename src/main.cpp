

#include <iostream>
#include <string>

#include "canis/Canis.hpp"
#include "canis/Debug.hpp"
#include "canis/Script.hpp"
#include "canis/Window.hpp"

int main() {
    Canis::Init();
    
    Canis::Window::Create("Test", 300, 300, 0);
    Canis::Window::SetWindowName("Yes");
    Canis::Window::SetClearColor(glm::vec4(1.0f));
    Canis::Window::ClearColor();
    Canis::Window::SwapBuffer();
    Canis::Window::ClearColor();
    

    Script testScript;
    testScript.Init("GameScripts.dll"); // might move this to the assets folder or a new share lib folder in root and dist
    testScript.Start();

    for (int i = 0; i < 10000; i++)
    {
        Canis::Window::ClearColor();
        testScript.Update();
        Canis::Window::SwapBuffer();
    }

    testScript.Destroy();

    
    return 0;
}

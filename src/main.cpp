

#include <iostream>
#include <string>

#include "canis/Canis.hpp"
#include "canis/Debug.hpp"
#include "canis/ScriptManager.hpp"
#include "canis/Window.hpp"
#include "canis/Graphics.hpp"

int main() {
    Canis::Init();
    
    Canis::Window::Create("Test", 300, 300, 0);
    Canis::Window::SetWindowName("Yes");

    Canis::ScriptManager::Init("GameScripts.dll");
    Script testScript = Canis::ScriptManager::LoadScript("MyScript");
    testScript.Start();

    for (int i = 0; i < 10000; i++)
    {
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
        testScript.Update();
        Canis::Window::SwapBuffer();
    }

    testScript.Destroy();

    
    return 0;
}



#include <iostream>
#include <string>

#include "canis/Canis.hpp"
#include "canis/Debug.hpp"
#include "canis/ScriptManager.hpp"
#include "canis/Window.hpp"
#include "canis/Graphics.hpp"
#include "canis/Scene.hpp"

int main() {
    Canis::Init();
    
    Canis::Window::Create("Test", 300, 300, 0);
    Canis::Window::SetWindowName("Yes");

    Canis::ScriptManager::Init("GameScripts.dll");

    Canis::Scene scene;

    Canis::Entity entity = scene.CreateEntity();
    entity.AddScript("MyScript");
    //Canis::Entity entity1 = scene.CreateEntity();
    //entity1.AddScript("Player");

    for (int i = 0; i < 10000; i++)
    {
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
        scene.Update(0.5f);
        Canis::Window::SwapBuffer();
    }

    scene.Destroy();

    Canis::ScriptManager::Destroy();
    return 0;
}

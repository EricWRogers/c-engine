

#include <iostream>
#include <string>
#include <thread>

#include "canis/Canis.hpp"
#include "canis/Debug.hpp"
#include "canis/ScriptManager.hpp"
#include "canis/Window.hpp"
#include "canis/Graphics.hpp"
#include "canis/Scene.hpp"
#include "canis/FrameRateManager.hpp"

int main() {
    Canis::Init();
    Canis::FrameRateManager frameRateManager;
    frameRateManager.Init(10000);

    g_mainThreadId = std::this_thread::get_id();
    
    Canis::Window::Create("Test", 300, 300, 0);
    Canis::Window::SetWindowName("Yes");

    Canis::ScriptManager::Init("GameScripts.dll");

    Canis::Scene scene;

    Canis::Entity entity = scene.CreateEntity();
    entity.AddScript("MyScript");
    Canis::Entity entity1 = scene.CreateEntity();
    entity1.AddScript("Player");

    for (int i = 0; i < 10000; i++)
    {
        frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
        scene.Update(0.5f);
        Canis::Window::SwapBuffer();
        frameRateManager.EndFrame();
    }

    scene.Destroy();

    Canis::ScriptManager::Destroy();
    return 0;
}

#pragma once
#include <Canis/Scene.hpp>

namespace Canis
{
class App
{
public:
    Scene scene;
    void Run();

    // Time
    float FPS();
    float DeltaTime();
    void SetTargetFPS(float _targetFPS);
};
}
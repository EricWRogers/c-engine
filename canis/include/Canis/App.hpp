#pragma once
#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>

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
    void RegisterScript(ScriptConf& _conf);
    void UnregisterScript(ScriptConf& _conf);
    std::vector<ScriptConf>& GetScriptRegistry() { return m_scriptRegistry; }
private:
    std::vector<ScriptConf> m_scriptRegistry = {};
};
}
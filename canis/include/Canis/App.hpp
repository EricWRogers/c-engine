#pragma once
#include <Canis/Scene.hpp>
#include <Canis/Entity.hpp>

namespace Canis
{
class Editor;

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

    void RegisterInspectorItem(InspectorItemRightClick& _item);
    void UnregisterInspectorItem(InspectorItemRightClick& _item);
    std::vector<InspectorItemRightClick>& GetInspectorItemRegistry() { return m_inspectorItemRegistry; }
private:
    std::vector<ScriptConf> m_scriptRegistry = {};
    std::vector<InspectorItemRightClick> m_inspectorItemRegistry = {};

    void RegisterDefaults(Editor& _editor);
};
}
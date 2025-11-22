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

    ScriptConf* GetScriptConf(const std::string& _name);
    bool AddRequiredScript(Entity& _entity, const std::string& _name);

    Editor& GetEditor() { return *m_editor; }

    void RegisterInspectorItem(InspectorItemRightClick& _item);
    void UnregisterInspectorItem(InspectorItemRightClick& _item);
    std::vector<InspectorItemRightClick>& GetInspectorItemRegistry() { return m_inspectorItemRegistry; }
private:
    std::vector<ScriptConf> m_scriptRegistry = {};
    std::vector<InspectorItemRightClick> m_inspectorItemRegistry = {};

    void RegisterDefaults(Editor& _editor);
    Editor* m_editor;
};
}
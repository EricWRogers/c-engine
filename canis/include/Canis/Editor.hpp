#pragma once

#include <string>
#include <vector>

#include <Canis/UUID.hpp>

namespace Canis
{
    class Scene;
    class App;
    class Window;
    class Camera2D;
    class Entity;
    struct GameCodeObject;

    enum EditorMode
    {
        EDIT,
        PLAY,
        PAUSE,
        HIDDEN
    };

    enum GuizmoMode
    {
        LOCAL = 0,
        WORLD = 1,
    };

    struct AssetDragData
    {
        UUID uuid;
        char path[1024]; // full path to file
    };

    class Editor
    {
    friend class Scene;
    friend class App;

    public:
        Editor() = default;
        ~Editor() = default;
        void Init(Window* _window);
        void Draw(Scene* _scene, Window* _window, App* _app, GameCodeObject* _gameSharedLib);

        EditorMode GetMode() { return m_mode; }
        void FocusEntity(Canis::Entity* _entity);

        // inspector variables
        void InputEntity(const std::string& _name, Canis::Entity* &_variable);
        //void InputScriptableEntity(const std::string& _name, const std::string& _script, );
    private:
        void DrawInspectorPanel(bool _refresh);
        void DrawAddComponentDropDown(bool _refresh);
        //void DrawSystemPanel();
        bool IsDescendantOf(Canis::Entity* _parent, Canis::Entity* _potentialChild);
        void DrawHierarchyNode(Canis::Entity* _entity, std::vector<Canis::Entity*>& _entities,bool& _refresh);
        bool DrawHierarchyPanel();
        bool DrawHierarchyElement(int _index);
        void DrawEnvironment();
        void DrawAssetsPanel();
        void DrawDirectoryRecursive(const std::string &_dirPath);
        void CommitAssetRename();
        void DrawProjectSettings();
        void DrawScenePanel();

        void SelectSprite2D();
        void DrawGizmo(Camera2D *_camera2D);
        void DrawBoundingBox(Camera2D *_camera2D);

        //bool IsDescendantOf(Entity _potentialAncestor, Entity _entity);

        //SceneManager& GetSceneManager();

        // this should be a seperate system that runs after editor draw
        // that can take elements to draw queue them then draw at the end of a frame
        enum DebugDraw
        {
            NONE,
            RECT,
        };

        Scene *m_scene;
        App *m_app;
        Window* m_window;
        GameCodeObject* m_gameSharedLib;
        int m_index = 0;
        bool m_forceRefresh = false;
        EditorMode m_mode = EditorMode::EDIT;
        DebugDraw m_debugDraw = DebugDraw::NONE;
        GuizmoMode m_guizmoMode = GuizmoMode::WORLD;
        std::vector<std::string> m_assetPaths = {};

        // asset panel
        bool m_isRenamingAsset = false;
        std::string m_renamingPath;
        char m_renameBuffer[256] = {};
    };
}

#pragma once

#include <string>
#include <vector>

namespace Canis
{
    class Scene;
    class App;
    class Window;
    class Camera2D;
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
    private:
        void DrawInspectorPanel(bool _refresh);
        void DrawAddComponentDropDown(bool _refresh);
        //void DrawSystemPanel();
        bool DrawHierarchyPanel();
        bool DrawHierarchyElement(int _index);
        void DrawEnvironment();
        void DrawAssetsPanel();
        void DrawDirectoryRecursive(const std::string &_dirPath);
        void DrawScenePanel();

        void SelectGameUI();

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
    };
}

#pragma once

namespace Canis
{
    class Scene;
    class App;
    //class SceneManager;
    //class Time;
    class Window;

    enum EditorMode
    {
        EDIT,
        PLAY,
        HIDDEN
    };

    class Editor
    {
    friend class Scene;
    friend class App;

    public:
        Editor() = default;
        ~Editor() = default;
        void Init(Window* _window);
        void Draw(Scene* _scene, Window* _window, App* _app/*, Time *_time*/);

        EditorMode GetMode() { return m_mode; }
    private:
        void DrawInspectorPanel();
        void DrawAddComponent();
        //void DrawSystemPanel();
        void DrawHierarchyPanel();
        bool DrawHierarchyElement(int _index);
        //void DrawScenePanel(Window* _window, Time *_time);

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
        int m_index = 0;
        bool m_forceRefresh = false;
        EditorMode m_mode = EditorMode::EDIT;
        DebugDraw m_debugDraw = DebugDraw::NONE;
        //Entity debugRectTransformEntity;
    };
}
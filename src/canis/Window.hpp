#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>

#include "Debug.hpp"
#include "Canis.hpp"
#include "ScriptMacros.hpp"

namespace Canis
{
    enum SCRIPTABLE WindowFlags
    {
        FULLSCREEN = 1,
        BORDERLESS = 2,
        RESIZEABLE = 4
    };

    class SCRIPTABLE_PARTIAL Window
    {
    public:
        static int Create(std::string _windowName, int _screenWidth, int _screenHeight, unsigned int _currentFlags);
        static void SetWindowName(std::string _windowName);

        BIND_SCRIPT static void SwapBuffer();

        BIND_SCRIPT static void CenterMouse();
        BIND_SCRIPT static void SetMousePosition(int _x, int _y);

        static void ClearColor();
        static void SetClearColor(glm::vec4 _color);
        static glm::vec4 GetScreenColor() {
            CanisData& data = GetCanisData();
            return data.clearColor;
        }

        static void MouseLock(bool _isLocked);
        static bool GetMouseLock() {
            CanisData& data = GetCanisData();
            return data.mouseLock;
        }

        static int GetScreenWidth() {
            CanisData& data = GetCanisData();
            return data.screenWidth;
        }
        static int GetScreenHeight() {
            CanisData& data = GetCanisData();
            return data.screenHeight;
        }

        static void ToggleFullScreen();
        static void SetWindowSize(int _width, int _height);
        static void SetResized(bool _resized);
        static bool IsResized();

        static bool GetVSync();
        static void SetVSync(bool _vsync);
    };
} // end of Canis namespace

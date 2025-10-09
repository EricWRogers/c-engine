#include <Canis/InputManager.hpp>
#include <SDL_events.h>
#include <Canis/Debug.hpp>
//#include <imgui_impl_sdl2.h>
#include <Canis/Window.hpp>

namespace Canis
{
    InputManager::InputManager()
    {
        
    }

    InputManager::~InputManager()
    {
        while(m_gameControllers.size())
        {
            SDL_GamepadClose((SDL_Gamepad*)(m_gameControllers.begin()->controller));
            m_gameControllers.erase( m_gameControllers.begin() );
        }
    }

    bool InputManager::Update(int _screenWidth, int _screenHeight, void* _window)
    {
        SwapMaps();
        mouseRel = Vector2(0.0f);
        m_scrollVertical = 0;

        Window* window = (Window*)_window;
        window->SetResized(false);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            #if CANIS_EDITOR
            if (GetProjectConfig().editor)
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
            }
            #endif

            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID((SDL_Window*)window->GetSDLWindow()))
                return false;

            switch (event.type)
            {
            case SDL_QUIT:
                return false;
                break;
            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED && event.window.windowID == SDL_GetWindowID((SDL_Window*)window->GetSDLWindow())) {
                    _screenWidth = event.window.data1;
                    _screenHeight = event.window.data2;
                    window->SetWindowSize(_screenWidth, _screenHeight);
                }
                break;
            case SDL_MOUSEMOTION:
                    mouse.x = event.motion.x;
                    mouse.y = _screenHeight - event.motion.y;
                    mouseRel.x = event.motion.xrel;
                    mouseRel.y = event.motion.yrel;
                    
                    m_lastInputDeviceType = (mouseRel != Vector2(0.0f)) ? InputDevice::MOUSE : m_lastInputDeviceType;
                break;
            case SDL_MOUSEWHEEL:
                m_scrollVertical = event.wheel.y;
                
                m_lastInputDeviceType = (m_scrollVertical != 0.0f) ? InputDevice::MOUSE : m_lastInputDeviceType;
                break;
            case SDL_KEYUP:
                ReleasedKey(event.key.keysym.sym);
                break;
            case SDL_KEYDOWN:
                PressKey(event.key.keysym.sym);
                m_lastInputDeviceType = InputDevice::KEYBOARD;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                    m_leftClick = true;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    m_rightClick = true;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    m_leftClick = false;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    m_rightClick = false;
                break;
            case SDL_CONTROLLERDEVICEADDED:
                m_lastInputDeviceType = InputDevice::GAMEPAD;
                OnGameControllerConnected(&event.cdevice);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                OnGameControllerDisconnect(&event.cdevice);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                m_lastInputDeviceType = InputDevice::GAMEPAD;
                break;
            }
        }

        std::vector<SDL_Gamepad*> currentControllers;

        // find active controllers
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            if (SDL_IsGamepad(i)) {
                SDL_Gamepad *controller = SDL_GamepadOpen(i);
                currentControllers.push_back(controller);
            }
        }

        // remove unactive controllers
        for (int i = 0; i < m_gameControllers.size(); i++)
        {
            bool found = false;

            for (int x = 0; x < currentControllers.size(); x++)
            {
                if (m_gameControllers[i].controller == currentControllers[x])
                {
                    found = true;
                }
            }

            if (!found)
            {
                m_gameControllers.erase( m_gameControllers.begin() + i );
            }
        }

        // reorder controller order
        for (int i = 0; i < m_gameControllers.size(); i++)
        {
            SDL_SetGamepadPlayerIndex(m_gameControllers[i].controller, i);
        }

        // update controllers
        int controllerID = 0;
        for(auto it = m_gameControllers.begin(); it != m_gameControllers.end(); it++)
        {
            if (it->controller)
            {
                it->oldData                     = it->currentData;
                it->currentData.buttons         = 0u;
                it->currentData.leftStick       = Vector2(0.0f);
                it->currentData.rightStick      = Vector2(0.0f);
                it->currentData.leftTrigger     = 0.0f;
                it->currentData.rightTrigger    = 0.0f;

                for (unsigned int i = 0; i < 15; i++) // 14 is the last button i care about for now
                {
                    // next line is cool
                    it->currentData.buttons |= ((1 << i) * SDL_GetGamepadButton((SDL_Gamepad*)it->controller, (SDL_GamepadButton)i));
                }

                if (it->currentData.buttons != 0)
                {
                    m_lastInputDeviceType = InputDevice::GAMEPAD;
                    it->lastButtonsPressed = it->currentData.buttons;
                    m_lastControllerID = controllerID;
                }

                it->currentData.leftStick.x     = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTX)/32767.0f;
                it->currentData.leftStick.x     = (abs(it->currentData.leftStick.x) < it->deadZone) ? 0.0f : it->currentData.leftStick.x;

                it->currentData.leftStick.y     = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFTY)/32767.0f;
                it->currentData.leftStick.y     = (abs(it->currentData.leftStick.y) < it->deadZone) ? 0.0f : -(it->currentData.leftStick.y);

                it->currentData.rightStick.x    = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTX)/32767.0f;
                it->currentData.rightStick.x    = (abs(it->currentData.rightStick.x) < it->deadZone) ? 0.0f : it->currentData.rightStick.x;

                it->currentData.rightStick.y    = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHTY)/32767.0f;
                it->currentData.rightStick.y    = (abs(it->currentData.rightStick.y) < it->deadZone) ? 0.0f : -(it->currentData.rightStick.y);

                it->currentData.rightTrigger    = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)/32767.0f;
                it->currentData.leftTrigger     = SDL_GetGamepadAxis((SDL_Gamepad*)it->controller, SDL_GamepadAxis::SDL_GAMEPAD_AXIS_LEFT_TRIGGER)/32767.0f;
            
                if (it->currentData.leftStick != Vector2(0.0f) || it->currentData.rightStick != Vector2(0.0f))
                {
                    //Debug::Log("left x: " + std::to_string(it->currentData.leftStick.x));
                    //Debug::Log("left y: " + std::to_string(it->currentData.leftStick.y));
                    //Debug::Log("right x: " + std::to_string(it->currentData.rightStick.x));
                    //Debug::Log("right y: " + std::to_string(it->currentData.rightStick.y));
                    m_lastInputDeviceType = InputDevice::GAMEPAD;
                    it->lastButtonsPressed = 0u;
                    m_lastControllerID = controllerID;
                }

                controllerID++;
            }
        }
        
        return true;
    }

    void InputManager::PressKey(unsigned int _keyID)
    {
        m_keyVec.push_back(InputData { _keyID , true});
    }

    void InputManager::ReleasedKey(unsigned int _keyID)
    {
        m_keyVec.push_back(InputData { _keyID , false});
    }

    void InputManager::SwapMaps()
    {
        m_wasLeftClick = m_leftClick;
        //m_leftClick = false;
        m_wasRightClick = m_rightClick;
        //m_rightClick = false;

        int index;
        for (int i = 0; i < m_keyVec.size(); i++)
        {
            index = IsInLastKnown(m_keyVec[i].key);
            if (index != -1)
            {
                m_lastKnown[index].value = m_keyVec[i].value;
            }
            else
            {
                m_lastKnown.push_back(m_keyVec[i]);
            }
        }

        m_keyVec.clear();
    }

    bool InputManager::GetKey(unsigned int _keyID)
    {
        int *keystate;
        SDL_GetKeyboardState(keystate);
        return keystate[_keyID] && active;
    }

    bool InputManager::GetButton(unsigned int _gameControllerId, unsigned int _buttonId)
    {
        if (m_gameControllers.size() > _gameControllerId)
        {
            return ((m_gameControllers[_gameControllerId].currentData.buttons & _buttonId) > 0) && active;
        }

        return false;
    }

    bool InputManager::JustPressedButton(unsigned int _gameControllerId, unsigned int _buttonId)
    {
        if (m_gameControllers.size() > _gameControllerId)
        {
            return ((m_gameControllers[_gameControllerId].currentData.buttons & _buttonId) > 0 &&
            (m_gameControllers[_gameControllerId].oldData.buttons & _buttonId) == 0) && active;
        }

        return false;
    }

    bool InputManager::JustReleasedButton(unsigned int _gameControllerId, unsigned int _buttonId)
    {
        if (m_gameControllers.size() > _gameControllerId)
        {
            return ((m_gameControllers[_gameControllerId].currentData.buttons & _buttonId) == 0 &&
            (m_gameControllers[_gameControllerId].oldData.buttons & _buttonId) > 0) && active;
        }

        return false;
    }

    bool InputManager::LastButtonsPressed(unsigned int _gameControllerId, unsigned int _buttonId)
    {
        if (m_gameControllers.size() > _gameControllerId)
        {
            return m_gameControllers[_gameControllerId].lastButtonsPressed & _buttonId && active;
        }

        return false;
    }

    Vector2 InputManager::GetLeftStick(unsigned int _gameControllerId)
    {
        if (m_gameControllers.size() > _gameControllerId && active)
        {
            return m_gameControllers[_gameControllerId].currentData.leftStick;
        }

        return Vector2(0.0f);
    }

    Vector2 InputManager::GetRightStick(unsigned int _gameControllerId)
    {
        if (m_gameControllers.size() > _gameControllerId && active)
        {
            return m_gameControllers[_gameControllerId].currentData.rightStick;
        }

        return Vector2(0.0f);
    }

    float InputManager::GetLeftTrigger(unsigned int _gameControllerId)
    {
        if (m_gameControllers.size() > _gameControllerId && active)
        {
            return m_gameControllers[_gameControllerId].currentData.leftTrigger;
        }

        return 0.0f;
    }

    float InputManager::GetRightTrigger(unsigned int _gameControllerId)
    {
        if (m_gameControllers.size() > _gameControllerId && active)
        {
            return m_gameControllers[_gameControllerId].currentData.rightTrigger;
        }

        return 0.0f;
    }

    bool InputManager::JustPressedKey(unsigned int _keyID)
    {
        bool currentValue = IsKeyDownInVec(&m_keyVec, _keyID);

        bool lastKnownValue = false;

        int index = IsInLastKnown(_keyID);
        if (index != -1)
        {
            lastKnownValue = m_lastKnown[index].value;
        }

        if (currentValue && !lastKnownValue && active)
            return true;
        
        return false;       
    }

    bool InputManager::JustReleasedKey(unsigned int _keyID)
    {
        return IsKeyUpInVec(&m_keyVec, _keyID) && active;
    }

    bool InputManager::IsKeyUpInVec(std::vector<InputData> *_arr, unsigned int _value)
    {
        for (int i = 0; i < _arr->size(); i++)
        {
            if ((*_arr)[i].key == _value)
                return !(*_arr)[i].value && active;
        }
        
        return false;
    }

    bool InputManager::IsKeyDownInVec(std::vector<InputData> *_arr, unsigned int _value)
    {
        for (int i = 0; i < _arr->size(); i++)
        {
            if ((*_arr)[i].key == _value)
                return (*_arr)[i].value && active;
        }
        
        return false;
    }

    int InputManager::IsInLastKnown(unsigned int _value)
    {
        for (int i = 0; i < m_lastKnown.size(); i++)
        {
            if (m_lastKnown[i].key == _value)
                return i;
        }

        return -1;
    }
    
    bool InputManager::IsKeyDownInLastKnowVec(unsigned int _value)
    {
        for (int i = 0; i < m_lastKnown.size(); i++)
        {
            if (m_lastKnown[i].key == _value)
                return m_lastKnown[i].value && active;
        }
        
        return false;
    }

    void InputManager::OnGameControllerConnected(void *_device)
    {
        SDL_GamepadDeviceEvent& device = (*(SDL_GamepadDeviceEvent*)(_device));

        if (SDL_IsGamepad(device.which))
        {
            GameController gameController = {};
            gameController.controller = SDL_OpenGamepad(device.which);
            if (gameController.controller)
            { 
                SDL_Joystick* j = SDL_GetGamepadJoystick((SDL_Gamepad*)gameController.controller);
                gameController.joyId = SDL_JoystickID(j);

                m_lastInputDeviceType = InputDevice::GAMEPAD;
                gameController.lastButtonsPressed = ControllerButton::DPAD_UP;
                m_lastControllerID = m_gameControllers.size();

                std::string controllerName = std::string(SDL_GetGamepadName(gameController.controller));

                if (controllerName[0] == 'P')
                {
                    gameController.gameControllerType = GameControllerType::PLAYSTATION;
                }

                m_gameControllers.push_back(gameController);

                Debug::Log("Game Controller Connected Joy ID: %s Name: %s", std::to_string(gameController.joyId), controllerName);
            }
        }
    }
    
    void InputManager::OnGameControllerDisconnect(void *_device)
    {
        SDL_GamepadDeviceEvent& device = (*(SDL_GamepadDeviceEvent*)(_device));
        SDL_Gamepad * controller;
        SDL_JoystickID joyID = 0;

        std::vector<SDL_Gamepad*> currentControllers;

        // find active controllers
        for (int i = 0; i < SDL_GetNumJoysticks(); i++) {
            if (SDL_IsGamepad(i)) {
                controller = SDL_OpenGamepad(i);
                SDL_Joystick * js = SDL_GetGamepadJoystick(controller);
                if (device.which == SDL_JoystickID(js)) {
                    SDL_CloseGamepad(controller);
                    joyID = device.which;
                }
                else
                {
                    currentControllers.push_back(controller);
                }
            }
        }

        // remove unactive controllers
        for (int i = 0; i < m_gameControllers.size(); i++)
        {
            bool found = false;

            for (int x = 0; x < currentControllers.size(); x++)
            {
                if (m_gameControllers[i].controller == currentControllers[x])
                {
                    found = true;
                }
            }

            if (!found)
            {
                m_gameControllers.erase( m_gameControllers.begin() + i );

                Debug::Log("Game Controller Disconnected");
            }
        }

        // reorder controller order
        for (int i = 0; i < m_gameControllers.size(); i++)
        {
            SDL_SetGamepadPlayerIndex(m_gameControllers[i].controller, i);
        }
    }


} // end of Canis namespace

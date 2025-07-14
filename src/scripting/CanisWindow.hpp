#pragma once
#include "ScriptMacros.hpp"

class XPLAT_EXPORT CanisWindow
{
public:
    static void SetTitle(const char *_title);
    static void SetWindowSize(int _width, int _height);
    static void SetBackgroundColor(float _red, float _green, float _blue, float _alpha);
};

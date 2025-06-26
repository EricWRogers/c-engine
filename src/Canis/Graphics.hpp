#pragma once

#include "World.hpp"
#include "Window.hpp"

namespace Canis
{
namespace Graphics
{
    #define COLOR_BUFFER_BIT 0x00004000
    #define DEPTH_BUFFER_BIT 0x00000100

    void EnableDepthTest();
    void EnableAlphaChannel();
    void ClearBuffer(unsigned int _bufferBit);
}
}
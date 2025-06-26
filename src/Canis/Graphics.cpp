#include "Graphics.hpp"
#include <GL/glew.h>

namespace Canis
{
namespace Graphics
{
    void EnableDepthTest() {
        glEnable(GL_DEPTH_TEST);
    }

    void EnableAlphaChannel() {
        glEnable(GL_ALPHA);
    }

    void ClearBuffer(unsigned int _bufferBit) {
        glClear(_bufferBit);
    }
}
}
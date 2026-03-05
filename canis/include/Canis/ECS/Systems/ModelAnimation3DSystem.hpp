#pragma once
#include <Canis/System.hpp>

namespace Canis
{
    class ModelAnimation3DSystem : public System
    {
    public:
        ModelAnimation3DSystem() : System() { m_name = type_name<ModelAnimation3DSystem>(); }

        void Create() override {}
        void Ready() override {}
        void Update() override;
    };
} // end of Canis namespace

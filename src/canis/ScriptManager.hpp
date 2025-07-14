#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#include <stdio.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Script.hpp"
#include "Debug.hpp"

namespace Canis {
    namespace ScriptManager {
        struct Data{
            MonoDomain* domain = nullptr;
            MonoAssembly* bindingsAssembly = nullptr;
            MonoAssembly* scriptAssembly = nullptr;
            MonoImage* image = nullptr;
            std::unordered_map<std::string, MonoClass*> scriptClasses = {};
        };

        extern Data& GetData();
        extern void Init(std::string _bindingsDllPath, std::string _scriptDllPath);
        extern void Destroy();
        extern Script LoadScript(std::string _className);
    }
}
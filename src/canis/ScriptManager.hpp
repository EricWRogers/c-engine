#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Script.hpp"
#include "Debug.hpp"

namespace Canis {
    namespace ScriptManager {
        struct Data{
            MonoDomain* domain = nullptr;
            MonoAssembly* assembly = nullptr;
            MonoImage* image = nullptr;
            std::unordered_map<std::string, MonoClass*> scriptClasses = {};
        };

        Data& GetData() {
            static Data data = {};
            return data;
        }

        void Init(std::string _dllPath)
        {
            Data& data = GetData();
            mono_set_dirs("mono/lib", "mono/etc");

            data.domain = mono_jit_init("ScriptDomain");
            if (!data.domain) {
                FatalError("Failed to init Mono JIT");
                return;
            }

            data.assembly = mono_domain_assembly_open(data.domain, _dllPath.c_str());
            if (!data.assembly) {
                FatalError(("Failed to load assembly: " + _dllPath).c_str());
                return;
            }

            data.image = mono_assembly_get_image(data.assembly);
            if (!data.image) {
                FatalError("Failed to get image from assembly.");
                return;
            }
        }

        void Destroy() {
            Data& data = GetData();
            if (data.domain) {
                mono_jit_cleanup(data.domain);
                data.domain = nullptr;
            }
        }

        Script LoadScript(std::string _className)
        {
            Data& data = GetData();

            if (data.scriptClasses.contains(_className) == false)
            {
                MonoClass* klass = mono_class_from_name(data.image, "", _className.c_str());
                data.scriptClasses[_className] = klass;
                if (!klass) {
                    FatalError("Failed to find class MyScript.");
                }
            }

            Script script = {};
            script.className = _className;
            script.klass = data.scriptClasses[_className];

            // allocate instance
            script.instance = mono_object_new(data.domain, script.klass);
            if (!script.instance) {
                FatalError(("Failed to create instance of " + _className + ".").c_str());
            }

            // calls constructor if available
            mono_runtime_object_init(script.instance);

            return script;
        }
    }
}
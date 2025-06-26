#include "ScriptManager.hpp"
#include <mono/metadata/threads.h>

namespace Canis {
    namespace ScriptManager {

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
            int count = 0;

            if (data.scriptClasses.contains(_className) == false)
            {
                const char* className = _className.c_str();
                printf("f%s%d%p\n", _className.c_str(), count++, data.image);
                MonoClass* klass = mono_class_from_name(data.image, "", className);
                data.scriptClasses[_className] = klass;
                if (!klass) {
                    FatalError("Failed to find class MyScript.");
                }
            }

            printf("s%s%d\n", _className.c_str(), count++);

            Script script = {};
            script.className = _className;
            script.klass = data.scriptClasses[_className];

            printf("%s%d\n", _className.c_str(), count++);

            // allocate instance
            script.instance = mono_object_new(data.domain, script.klass);
            if (!script.instance) {
                FatalError(("Failed to create instance of " + _className + ".").c_str());
            }

            printf("%s%d\n", _className.c_str(), count++);

            // calls constructor if available
            mono_runtime_object_init(script.instance);

            { // cache methods
                script.startMethod = mono_class_get_method_from_name(script.klass, "Start", 0);
                
                script.updateMethod = mono_class_get_method_from_name(script.klass, "Update", 1);
                if (!script.updateMethod)
                    std::cerr << "Failed to find Update(float) method.\n";
                
                script.onDestroyMethod = mono_class_get_method_from_name(script.klass, "OnDestroy", 0);
            }

            printf("%s%d\n", _className.c_str(), count++);

            return script;
        }
    }
}
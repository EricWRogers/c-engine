#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

struct Script {
    MonoDomain* domain = nullptr;
    MonoAssembly* assembly = nullptr;
    MonoImage* image = nullptr;

    void Init(const char* dllPath) {
        mono_set_dirs("/usr/lib", "/etc/mono"); // adjust if needed

        domain = mono_jit_init("ScriptDomain");
        if (!domain) {
            std::cerr << "Failed to init Mono JIT\n";
            return;
        }

        assembly = mono_domain_assembly_open(domain, dllPath);
        if (!assembly) {
            std::cerr << "Failed to load assembly: " << dllPath << "\n";
            return;
        }

        image = mono_assembly_get_image(assembly);
        if (!image) {
            std::cerr << "Failed to get image from assembly.\n";
        }
    }

    void CallMethod(const char* methodName) {
        std::string fullName = "MyScript:" + std::string(methodName);  // e.g., "MyScript:Start"
        MonoMethodDesc* desc = mono_method_desc_new(fullName.c_str(), /*include_namespace=*/false);
        MonoMethod* method = mono_method_desc_search_in_image(desc, image);
        mono_method_desc_free(desc);

        if (method) {
            mono_runtime_invoke(method, nullptr, nullptr, nullptr);
        } else {
            std::cerr << "Method not found: " << methodName << "\n";
        }
    }

    void Start() {
        CallMethod("Start");
    }

    void Update() {
        CallMethod("Update");
    }

    void Destroy() {
        if (domain) {
            mono_jit_cleanup(domain);
        }
    }
};
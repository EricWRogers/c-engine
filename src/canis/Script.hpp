#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>
#include <string>

struct Script {
    MonoDomain* domain = nullptr;
    MonoAssembly* assembly = nullptr;
    MonoImage* image = nullptr;

    MonoClass* klass = nullptr;
    MonoObject* instance = nullptr;

    void Init(const char* dllPath) {
        mono_set_dirs("/usr/lib", "/etc/mono");

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
            return;
        }

        // Find the class MyScript in the global namespace
        klass = mono_class_from_name(image, "", "MyScript"); // "" = no namespace
        if (!klass) {
            std::cerr << "Failed to find class MyScript.\n";
            return;
        }

        // Allocate and initialize the object
        instance = mono_object_new(domain, klass);
        if (!instance) {
            std::cerr << "Failed to create instance of MyScript.\n";
            return;
        }

        mono_runtime_object_init(instance); // calls constructor if available
    }

    void CallInstanceMethod(const char* methodName) {
        MonoMethod* method = mono_class_get_method_from_name(klass, methodName, 0);
        if (!method) {
            std::cerr << "Method not found: " << methodName << "\n";
            return;
        }

        MonoObject* exception = nullptr;
        mono_runtime_invoke(method, instance, nullptr, &exception);

        if (exception) {
            MonoString* msg = mono_object_to_string(exception, nullptr);
            char* cstr = mono_string_to_utf8(msg);
            std::cerr << "Exception: " << cstr << "\n";
            mono_free(cstr);
        }
    }

    void Start() { CallInstanceMethod("Start"); }
    void Update() { CallInstanceMethod("Update"); }

    void Destroy() {
        if (domain) {
            mono_jit_cleanup(domain);
            domain = nullptr;
        }
    }
};

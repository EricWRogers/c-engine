#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <iostream>
#include <string>

struct Script {
    std::string className = "";
    MonoClass* klass = nullptr;
    MonoObject* instance = nullptr;

    void CallInstanceMethod(const char* methodName) {
        // cashe this later
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
    void Destroy() { CallInstanceMethod("Destroy"); }
};

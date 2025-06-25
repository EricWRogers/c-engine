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
    MonoMethod* startMethod = nullptr;
    MonoMethod* updateMethod = nullptr;
    MonoMethod* onDestroyMethod = nullptr;

    void CallInstanceMethod(MonoMethod* _method) {
        if (_method == nullptr)
            return;

        MonoObject* exception = nullptr;
        mono_runtime_invoke(_method, instance, nullptr, &exception);

        if (exception) {
            MonoString* msg = mono_object_to_string(exception, nullptr);
            char* cstr = mono_string_to_utf8(msg);
            std::cerr << "Exception: " << cstr << "\n";
            mono_free(cstr);
        }
    }

    void Start() {
        CallInstanceMethod(startMethod);
    }

    void Update() {
        CallInstanceMethod(updateMethod);
    }

    void Destroy() {
        CallInstanceMethod(onDestroyMethod);
    }
};

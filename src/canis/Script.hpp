#pragma once
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include <iostream>
#include <string>

struct Script {
    std::string className = "";
    MonoClass* klass = nullptr;
    MonoObject* instance = nullptr;
    MonoMethod* startMethod = nullptr;
    MonoMethod* updateMethod = nullptr;
    MonoMethod* onDestroyMethod = nullptr;

    void CallInstanceMethod(MonoMethod* _method, void** _args = nullptr) {
        if (_method == nullptr)
            return;

        MonoThread* monoThread = mono_thread_attach(mono_domain_get());

        MonoObject* exception = nullptr;
        mono_runtime_invoke(_method, instance, _args, &exception);

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

    void Update(float _deltaTime) {
        printf("calling update\n");
        float deltaTime = _deltaTime;
        void* args[1] = { &deltaTime };
        CallInstanceMethod(updateMethod, args);
    }

    void Destroy() {
        CallInstanceMethod(onDestroyMethod);
    }
};

#include "ScriptInstance.hpp"
#include <iostream>
#include <scriptbuilder.h>

ScriptInstance::ScriptInstance(asIScriptEngine* _engine, const std::string& className, const std::string& _filePath)
    : engine(_engine) 
{
    if (!engine) return;

    std::cout << "HI\n";

    // compile the script
    CScriptBuilder builder;
    if (builder.StartNewModule(engine, "Main") < 0 ||
        builder.AddSectionFromFile(_filePath.c_str()) < 0 ||
        builder.BuildModule() < 0) {
        std::cerr << "Failed to compile script: " << _filePath << std::endl;
        return;
    }

    asIScriptModule* mod = engine->GetModule("Main");
    if (!mod) {
        std::cerr << "Module 'Main' not found\n";
        return;
    }

    asITypeInfo* type = mod->GetTypeInfoByDecl(className.c_str());
    if (!type) {
        std::cerr << "Type '" << className << "' not found in module\n";
        return;
    }

    object = reinterpret_cast<asIScriptObject*>(engine->CreateScriptObject(type));
    if (!object) {
        std::cerr << "Failed to create script object\n";
    }
}

ScriptInstance::~ScriptInstance() {
    if (object) object->Release();
}

bool ScriptInstance::IsValid() const {
    return object != nullptr;
}

void ScriptInstance::Call(const std::string& methodName) {
    if (!object) return;

    std::string decl = "void " + methodName + "()";
    asIScriptFunction* func = object->GetObjectType()->GetMethodByDecl(decl.c_str());
    if (!func) return;

    asIScriptContext* ctx = engine->CreateContext();
    ctx->Prepare(func);
    ctx->SetObject(object);
    ctx->Execute();
    ctx->Release();
}

void ScriptInstance::CallFloat(const std::string& methodName, float value) {
    if (!object) return;

    std::string decl = "void " + methodName + "(float)";
    asIScriptFunction* func = object->GetObjectType()->GetMethodByDecl(decl.c_str());
    if (!func) return;

    asIScriptContext* ctx = engine->CreateContext();
    ctx->Prepare(func);
    ctx->SetObject(object);
    ctx->SetArgFloat(0, value);
    ctx->Execute();
    ctx->Release();
}

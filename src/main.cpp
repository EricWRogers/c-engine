#include <angelscript.h>
#include <scriptstdstring.h>
#include <scriptbuilder.h>
#include <iostream>
#include <cassert>

void Print(const std::string& msg) {
    std::cout << "[Script] " << msg << std::endl;
}

void MessageCallback(const asSMessageInfo* msg, void*) {
    const char* type = "ERROR";
    if (msg->type == asMSGTYPE_WARNING) type = "WARNING";
    else if (msg->type == asMSGTYPE_INFORMATION) type = "INFO";

    std::cerr << msg->section << " (" << msg->row << ", " << msg->col << ") : "
              << type << " : " << msg->message << std::endl;
}

int main() {
    asIScriptEngine* engine = asCreateScriptEngine();
    if (!engine) {
        std::cerr << "Failed to create AngelScript engine" << std::endl;
        return -1;
    }

    engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);

    RegisterStdString(engine);

    engine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(Print), asCALL_CDECL);


    CScriptBuilder builder;
    if (builder.StartNewModule(engine, "Main") < 0 ||
        builder.AddSectionFromFile("assets/scripts/test.as") < 0 ||
        builder.BuildModule() < 0) {
        std::cerr << "Failed to compile script." << std::endl;
        engine->ShutDownAndRelease();
        return -1;
    }

    asIScriptModule* mod = engine->GetModule("Main");
    asITypeInfo* type = mod->GetTypeInfoByDecl("MyScript");
    if (!type) {
        std::cerr << "Class 'MyScript' not found in script." << std::endl;
        engine->ShutDownAndRelease();
        return -1;
    }

    asIScriptObject* obj = reinterpret_cast<asIScriptObject*>(engine->CreateScriptObject(type));

    auto CallMethod = [&](const char* decl, float arg = 0.0f) {
        asIScriptFunction* func = obj->GetObjectType()->GetMethodByDecl(decl);
        if (!func) return;
        asIScriptContext* ctx = engine->CreateContext();
        ctx->Prepare(func);
        ctx->SetObject(obj);
        if (std::string(decl).find("float") != std::string::npos)
            ctx->SetArgFloat(0, arg);
        ctx->Execute();
        ctx->Release();
    };

    CallMethod("void Create()");
    CallMethod("void Start()");
    CallMethod("void Update(float)", 0.016f);
    CallMethod("void Destroy()");

    obj->Release();
    engine->ShutDownAndRelease();
    return 0;
}

#pragma once

#include <angelscript.h>
#include <string>

namespace Canis
{
class ScriptInstance {
public:
    ScriptInstance(asIScriptEngine* engine, const std::string& className, const std::string& _filePath);
    ~ScriptInstance();

    bool IsValid() const;

    void Call(const std::string& methodName);
    void CallFloat(const std::string& methodName, float value);

    bool startCalled = false;
private:
    asIScriptEngine* engine = nullptr;
    asIScriptObject* object = nullptr;
};
}
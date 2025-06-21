#pragma once
#include <iostream>
#include <string>

namespace Canis
{
  struct LoggingData {
    std::string logTarget = "";
    int count = 0;
    void *logFile = nullptr;
    void *logFileError = nullptr;
  };

  extern void FatalError(const char* _message);

  extern void Error(const char* _message);

  extern void Warning(const char* _message);

  extern void Log(const char* _message);

  extern void TurnOffLog();

  extern void TurnOnLog();

  extern void SetLogTarget(std::string _path);

  extern LoggingData& GetLoggingData();
} // end of Canis namespace

namespace CSharpLayer
{
extern "C" {
    void CSharpLayer_FatalError(const char* _message);
    void CSharpLayer_Error(const char* _message);
    void CSharpLayer_Warning(const char* _message);
    void CSharpLayer_Log(const char* _message);
}
}
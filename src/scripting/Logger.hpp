#pragma once
#include "ScriptMacros.hpp"

class XPLAT_EXPORT Logger
{
public:
  static void FatalError(const char *_message);
  static void Error(const char *_message);
  static void Warning(const char *_message);
  static void Log(const char *_message);
};

#pragma once
#include <iostream>
#include <string>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace Canis
{
  extern void FatalError(std::string message);

  extern void Error(std::string message);

  extern void Warning(std::string message);

  extern void Log(std::string message);
} // end of Canis namespace
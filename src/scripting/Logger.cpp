#include "Logger.hpp"
#include <string>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include "canis/Debug.hpp"

void Logger::FatalError(const char *_message)
{
    Canis::FatalError(_message);
}

void Logger::Error(const char *_message)
{
    Canis::Error(_message);
}

void Logger::Warning(const char *_message)
{
    Canis::Warning(_message);
}

void Logger::Log(const char *_message)
{
    Canis::Log(_message);
}

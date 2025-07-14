#pragma once

// Macro for exporting to DLL
#ifdef _WIN32
#define XPLAT_EXPORT __declspec(dllexport)
#else
#define XPLAT_EXPORT
#endif

// This macro will only expand to the 'annotate' attribute when CppSharp is parsing.
#ifdef CPP_SHARP_PARSER
    #if defined(__clang__) || defined(__GNUC__)
        #define BIND_SCRIPT __attribute__((annotate("BIND_SCRIPT")))
    #elif defined(_MSC_VER)
        #define BIND_SCRIPT __declspec(annotate("BIND_SCRIPT"))
    #else
        #define BIND_SCRIPT
    #endif
#else
    // For all other cases, we define it as empty.
    #define BIND_SCRIPT
#endif

// Exposes all fields/methods of a class/struct to C#. Can also be used to expose Global variables/functions.
#define SCRIPTABLE XPLAT_EXPORT BIND_SCRIPT

// Marks a class/struct as bindable to C#, but doesn't actually expose any fields/methods. You can use BIND_SCRIPT on each field/method to expose them individually.
#define SCRIPTABLE_PARTIAL XPLAT_EXPORT

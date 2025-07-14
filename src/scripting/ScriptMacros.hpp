#pragma once

#define EXPOSE_TO_SCRIPT

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

#ifdef _WIN32
#define XPLAT_EXPORT __declspec(dllexport) BIND_SCRIPT
#else
#define XPLAT_EXPORT
#endif

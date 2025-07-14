namespace EngineBindingsGenerator
{
    class CppSharpModuleRegistry
    {
        public static readonly List<CppSharpModuleInfo> RegisteredModules = [
            new CppSharpModuleInfo() {
                Name = "CanisEngine",
                IncludeDirs = ["../src", "../src/scripting", "../external/glm", "../external/SDL/include"],
                Headers = [
                    //".generated/ScriptBindings.cpp",
                    
                    // Canis
                    "canis/Window.hpp",

                    // Scripting
                    "Logger.hpp",
                    "CanisWindow.hpp",
                ],
            },
            //new CppSharpModuleInfo() {
            //    Name = "CanisEngine2",
            //    IncludeDirs = ["../src", "../src/scripting", "../external/glm", "../external/SDL/include"],
            //    Headers = [
            //        //".generated/ScriptBindings.cpp",
            //        // Core
            //        //"core/vec.h",

            //        // Canis
            //        "canis/Window.hpp",
            //    ],
            //},
        ];
    }
}

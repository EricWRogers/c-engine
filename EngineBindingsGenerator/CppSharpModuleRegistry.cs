namespace EngineBindingsGenerator
{
    class CppSharpModuleRegistry
    {
        public static readonly List<CppSharpModuleInfo> RegisteredModules = [
            new CppSharpModuleInfo() {
                Name = "CanisEngine",
                IncludeDirs = ["../src", "../src/scripting", "../external/glm", "../external/SDL/include"],
                Headers = [
                    // Canis
                    "canis/Window.hpp",

                    // Scripting
                    "Logger.hpp",
                    "CanisWindow.hpp",
                ],
            },
        ];
    }
}

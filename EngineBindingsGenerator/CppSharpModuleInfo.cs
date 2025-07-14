using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EngineBindingsGenerator
{
    class CppSharpModuleInfo
    {
        public required string Name;
        public required List<string> IncludeDirs;
        public required List<string> Headers;
    }
}

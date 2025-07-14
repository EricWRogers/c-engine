using CppSharp;
using CppSharp.AST;
using CppSharp.Generators;
using CppSharp.Passes;

namespace EngineBindingsGenerator
{
    class EngineBindings : ILibrary
    {
        public const string BIND_SCRIPT_ATTRIBUTE = "BIND_SCRIPT";
        public const string SCRIPTABLE_ATTRIBUTE = "SCRIPTABLE";

        public static readonly List<string> AllowedHeaderFolders = [
            Path.GetFullPath("../src"),
        ];

        public void Setup(Driver driver)
        {
            var options = driver.Options;
            options.GeneratorKind = GeneratorKind.CSharp;
            options.OutputDir = "../GameBindings/Generated";
            options.GenerationOutputMode = GenerationOutputMode.FilePerUnit;
            options.MarshalCharAsManagedChar = false;
            options.MarshalConstCharArrayAsString = false;
            options.GenerateInternalImports = false;
            options.Encoding = System.Text.Encoding.ASCII;

            driver.ParserOptions.AddDefines("CPP_SHARP_PARSER");

            // Recursively register all modules
            foreach (var moduleInfo in CppSharpModuleRegistry.RegisteredModules)
            {
                var module = options.AddModule(moduleInfo.Name);
                foreach (var dir in moduleInfo.IncludeDirs)
                {
                    module.IncludeDirs.Add(dir);
                }
                foreach (var header in moduleInfo.Headers)
                {
                    module.Headers.Add(header);
                }
            }
        }

        public void SetupPasses(Driver driver) { }

        public void Preprocess(Driver driver, ASTContext ctx)
        {
            var allowedTUs = new List<TranslationUnit>();
            foreach (var tu in ctx.TranslationUnits.Where(u => u.IsValid))
            {
                var tuPath = Path.GetFullPath(tu.FilePath);
                if (AllowedHeaderFolders.Any(dir => tuPath.StartsWith(dir)))
                {
                    allowedTUs.Add(tu);
                }
                else
                {
                    allowedTUs.Add(tu);
                }
            }

            var ignoreVisitor = new IgnoreAllDeclarationsVisitor();
            foreach (var tu in ctx.TranslationUnits.Where(u => u.IsValid))
            {
                tu.Visit(ignoreVisitor);
            }

            var findVisitor = new FindMarkedDeclarationsVisitor();
            foreach (var tu in ctx.TranslationUnits.Where(u => u.IsValid))
            {
                tu.Visit(findVisitor);
            }
        }

        public void Postprocess(Driver driver, ASTContext ctx) { }
    }


    /// <summary>
    /// CppSharp AST visitor that marks every declaration to be ignored.
    /// </summary>
    public class IgnoreAllDeclarationsVisitor : CustomAstVisitor
    {
        public override bool VisitDeclaration(Declaration decl)
        {
            decl.Ignore = true;
            return base.VisitDeclaration(decl);
        }
    }

    /// <summary>
    /// CppSharp AST visitor that finds all declarations marked with our MACROS
    /// </summary>
    public class FindMarkedDeclarationsVisitor : CustomAstVisitor
    {
        public readonly HashSet<Declaration> MarkedDeclarations = [];

        public FindMarkedDeclarationsVisitor()
        {
            //
        }

        public override bool VisitDeclaration(Declaration decl)
        {
            var declPath = Path.GetFullPath(decl.TranslationUnit.FilePath);
            if (!IsDeclarationAllowed(decl))
            {
                return false;
            }

            var debug = false;
            //var debug = declPath.EndsWith("\\Window.hpp");
            if (debug)
            {
                Console.WriteLine("Visiting.. " + decl.ToString());
                Console.WriteLine("Type.. " + decl.GetType().FullName);
            }

            if (decl.Namespace != null && MarkedDeclarations.Contains(decl.Namespace))
            {
                if (debug)
                    Console.WriteLine("Parent is marked.");
                decl.GenerationKind = GenerationKind.Generate;
                MarkedDeclarations.Add(decl);   // To allow transitive generation
            }

            bool foundMacro = false;
            IEnumerable<MacroExpansion> macros = GetAppropriateMacros(decl);
            if (macros.Any((MacroExpansion e) => e.Text == EngineBindings.SCRIPTABLE_ATTRIBUTE || e.Text == EngineBindings.BIND_SCRIPT_ATTRIBUTE))
            {
                foundMacro = true;
            }

            if (debug)
            {
                Console.WriteLine("Macros: " + string.Join(',', macros.Select(m => m.Text)));
            }

            if (decl.Ignore && foundMacro)
            {
                if (debug)
                    Console.WriteLine("Macro Found.");
                decl.GenerationKind = GenerationKind.Generate;
                MarkedDeclarations.Add(decl);

                // Walk up the parent hierarchy (class, namespace, TU) and un-ignore them as well
                var parent = decl.Namespace;
                while (parent != null)
                {
                    parent.GenerationKind = GenerationKind.Generate;
                    if (debug)
                        Console.WriteLine("Unignoring parent.. " + parent.ToString());
                    parent = parent.Namespace;
                }
            }

            return base.VisitDeclaration(decl);
        }
    }

    public class CustomAstVisitor : AstVisitor
    {
        public bool IsDeclarationAllowed(Declaration decl)
        {
            var declPath = Path.GetFullPath(decl.TranslationUnit.FilePath);
            if (!EngineBindings.AllowedHeaderFolders.Any(dir => declPath.StartsWith(dir)))
            {
                return false;
            }
            return true;
        }

        public IEnumerable<MacroExpansion> GetAppropriateMacros(Declaration decl)
        {
            var macroExpansions = decl.PreprocessedEntities.OfType<MacroExpansion>();

            if (decl is Class)
            {
                return macroExpansions.Where(me => me.MacroLocation == MacroLocation.ClassHead);
            }
            else if (decl is Function)
            {
                return macroExpansions.Where(me => me.MacroLocation == MacroLocation.FunctionHead);
            }
            else if (decl is TranslationUnit || decl is Namespace)
            {
                return [];
            }

            return macroExpansions;
        }
    }
}

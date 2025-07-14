using CppSharp;
using CppSharp.AST;
using CppSharp.Generators;
using System.Linq;

namespace EngineBindingsGenerator
{
    class EngineBindings : ILibrary
    {
        private const string BIND_SCRIPT_ATTRIBUTE = "BIND_SCRIPT";
        public static readonly List<string> AllowedHeaderFolders = [
            Path.GetFullPath("../src"),
        ];

        public void Setup(Driver driver)
        {
            var options = driver.Options;
            options.GeneratorKind = GeneratorKind.CSharp;
            options.OutputDir = "../GameBindings/Generated";
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
                // Get the path of the main file parsed for this translation unit.
                var tuPath = Path.GetFullPath(tu.FilePath);

                // Check if this main file is inside one of our allowed folders.
                if (AllowedHeaderFolders.Any(dir => tuPath.StartsWith(dir)))
                {
                    allowedTUs.Add(tu);
                }
            }

            // 1. Mark all declarations to be ignored by default (opt-in).
            var ignoreVisitor = new IgnoreAllDeclarationsVisitor();
            foreach (var tu in ctx.TranslationUnits.Where(u => u.IsValid))
            {
                tu.Visit(ignoreVisitor);
            }

            // 2. Collect all declarations that have our BIND_API attribute.
            var findVisitor = new FindMarkedDeclarationsVisitor(BIND_SCRIPT_ATTRIBUTE);
            foreach (var tu in allowedTUs)
            {
                tu.Visit(findVisitor);
            }

            Console.WriteLine("Marked Declarations: " + findVisitor.MarkedDeclarations.Count);

            //3. Un-ignore the collected declarations and their parent containers.
            foreach (var decl in findVisitor.MarkedDeclarations)
            {
                // Un-ignore the declaration itself
                decl.Ignore = false;

                // Walk up the parent hierarchy (class, namespace) and un-ignore them too.
                var parent = decl.Namespace;
                while (parent != null)
                {
                    parent.Ignore = false;
                    parent = parent.Namespace;
                }
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
            //if (!IsDeclarationAllowed(decl))
            //{
            //    decl.Ignore = true;
            //    return false; // Returning false stops traversal down this path.
            //}

            decl.Ignore = true;
            return base.VisitDeclaration(decl);
        }
    }

    /// <summary>
    /// CppSharp AST visitor that finds all declarations marked with a specific attribute.
    /// </summary>
    public class FindMarkedDeclarationsVisitor : CustomAstVisitor
    {
        public readonly List<Declaration> MarkedDeclarations = new List<Declaration>();
        private readonly string _attributeText;

        public FindMarkedDeclarationsVisitor(string attributeText)
        {
            _attributeText = attributeText;
        }

        public override bool VisitDeclaration(Declaration decl)
        {
            if (!IsDeclarationAllowed(decl))
                return false;

            Console.WriteLine("Decl: " + decl.ToString());
            Console.WriteLine("Attrs: " + string.Join(',', decl.Attributes.Select(a => a.Value)));
            if (decl.Attributes.Any(attr => attr.Value == _attributeText))
            {
                MarkedDeclarations.Add(decl);
            }

            return base.VisitDeclaration(decl);
        }
    }

    public class CustomAstVisitor : AstVisitor
    {
        public bool IsDeclarationAllowed(Declaration decl)
        {
            var declPath = Path.GetFullPath(decl.TranslationUnit.FilePath);

            //// If the declaration is not from a source file in our allowed folders, skip it.
            if (!EngineBindings.AllowedHeaderFolders.Any(dir => declPath.StartsWith(dir)))
            {
                return false; // Stop traversal down this AST branch.
            }

            Console.WriteLine("Checking.. " + declPath);
            //Console.WriteLine(declPath);
            return true;
        }
    }
}

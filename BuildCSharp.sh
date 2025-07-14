#!/bin/bash

# Generating C# Bindings
echo ""
echo "Generating C# bindings..."
rm -rf GameBindings/Generated
cd EngineBindingsGenerator && dotnet run

# check if it succeeded
if [ $? -ne 0 ]; then
    echo ""
    echo "C# Binding generation failed."
    exit 1
fi

cd ..

# compile the scripts into a DLL
echo ""
echo "Compiling C# scripts..."
cd assets/scripts && dotnet clean && dotnet build -c Release

# check if it succeeded
if [ $? -ne 0 ]; then
    echo ""
    echo "Compilation failed."
    exit 1
else
    echo ""
    echo "GameScripts.dll compiled successfully."
fi

cd ../..

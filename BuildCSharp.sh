#!/bin/bash

# go to output directory
cd dist/Linux || { echo "dist/Linux directory not found."; exit 1; }

# find all .cs files under assets/scripts and store in a variable
CS_FILES=$(find ../../assets/scripts -type f -name "*.cs")

# compile the scripts into a DLL
echo "Compiling C# scripts..."
time mcs -debug- -target:library -out:GameScripts.dll $CS_FILES

# check if it succeeded
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
else
    echo "GameScripts.dll compiled successfully."
fi

cd ../..

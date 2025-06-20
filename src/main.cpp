

#include <iostream>
#include <string>

#include "canis/Debug.hpp"
#include "canis/Script.hpp"

int main() {
    Script testScript;
    testScript.Init("assets/GameScripts.dll");
    testScript.Start();
    testScript.Update();
    testScript.Destroy();
    return 0;
}

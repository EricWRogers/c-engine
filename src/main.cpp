#include <vector>
#include <stdio.h>

namespace String {
    
}

namespace Window {

}

#include <iostream>

#include <sol/sol.hpp>

void HelloCPP()
{
    printf("HelloCPP\n");
}

int main() {
    std::cout << "Hello, from C++\n";
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua["hello_cpp"] = HelloCPP;
    
    sol::protected_function_result result = lua.do_file("assets/scripts/test.lua");
    if (!result.valid()) {
        sol::error err = result;
        std::cout << err.what();
    }
}

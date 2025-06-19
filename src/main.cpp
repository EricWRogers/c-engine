#include <vector>
#include <stdio.h>

namespace String {
    
}

namespace Window {

}

#include <iostream>

#include <sol/sol.hpp>

int main() {
    std::cout << "Hello, from C++\n";
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    
    sol::protected_function_result result = lua.do_file("assets/scripts/test.lua");
    if (!result.valid()) {
        sol::error err = result;
        std::cout << err.what();
    }
}

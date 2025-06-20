#include <iostream>

extern "C" {
    void Log(const char* msg) {
        std::cout << "[C#] " << msg << std::endl;
    }
}
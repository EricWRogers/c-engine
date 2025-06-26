#include "Canis.hpp"
#include "Debug.hpp"
#include <SDL.h>

#include <fstream>

namespace Canis
{
    ProjectConfig& GetConfig()
    {
        static ProjectConfig projectConfig = {};
        return projectConfig;
    }

    int Init()
    {
        SDL_Init(SDL_INIT_EVERYTHING);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // load project.canis
        std::ifstream file;
        file.open("assets/project.canis");

        if (!file.is_open()) return 1;

        std::string word;
        int wholeNumber = 0;
        unsigned int unsignedWholeNumber = 0;
        float fNumber = 0.0f;
        while(file >> word) {
            if (word == "fullscreen"){
                if (file >> word) {
                    GetConfig().fullscreen = (word == "true");
                    continue;
                }
            }
            if (word == "width") {
                if (file >> wholeNumber) {
                    GetConfig().width = wholeNumber;
                    continue;
                }
            }
            if (word == "heigth") {
                if (file >> wholeNumber) {
                    GetConfig().heigth = wholeNumber;
                    continue;
                }
            }
            if (word == "volume") {
                if (file >> fNumber) {
                    GetConfig().volume = fNumber;
                    if (GetConfig().volume > 1.5f)
                        GetConfig().volume = 1.5f;
                    if (GetConfig().volume < 0.0f)
                        GetConfig().volume = 0.0f;
                    continue;
                }
            }
            if(word == "use_frame_limit")
            {
                if(file >> word)
                {
                    GetConfig().useFrameLimit = (word == "true");
                    continue;
                }
            }
            if(word == "frame_limit")
            {
                if(file >> wholeNumber)
                {
                    GetConfig().frameLimit = wholeNumber;
                    continue;
                }
            }
            if(word == "override_seed")
            {
                if(file >> word)
                {
                    GetConfig().overrideSeed = (word == "true");
                    continue;
                }
            }
            if(word == "seed")
            {
                if(file >> unsignedWholeNumber)
                {
                    GetConfig().seed = unsignedWholeNumber;
                    continue;
                }
            }
            if (word == "log")
            {
                if (file >> word)
                {
                    GetConfig().log = (word == "true");
                    continue;
                }
            }
        }

        file.close();
        
        return 0;
    }
} // end of Canis namespace
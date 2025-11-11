#include <Canis/Canis.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Yaml.hpp>
#include <Canis/IOManager.hpp>

#include <fstream>

namespace Canis
{
    ProjectConfig& GetProjectConfig()
    {
        static ProjectConfig projectConfig = {};
        return projectConfig;
    }

    bool SaveProjectConfig()
    {
        ProjectConfig projectConfig = GetProjectConfig();

        YAML::Node node;

        node["useFrameLimit"] = projectConfig.useFrameLimit;
        node["frameLimit"] = projectConfig.frameLimit;
        node["frameLimitEditor"] = projectConfig.frameLimitEditor;
        node["overrideSeed"] = projectConfig.overrideSeed;
        node["seed"] = projectConfig.seed;
        node["editor"] = projectConfig.editor;
        node["iconUUID"] = std::to_string(projectConfig.iconUUID);

        std::ofstream fout("project.canis");
        fout << node;

        return true;
    }

    int Init()
    {
        //SDL_Init(SDL_INIT_EVERYTHING);

        //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // load project.canis
        ProjectConfig projectConfig = GetProjectConfig();

        YAML::Node node;
        if (FileExists("project.canis"))
            node = YAML::LoadFile("project.canis");

        projectConfig.useFrameLimit = node["useFrameLimit"].as<bool>(projectConfig.useFrameLimit);
        projectConfig.frameLimit = node["frameLimit"].as<float>(projectConfig.frameLimit);
        projectConfig.frameLimitEditor = node["frameLimitEditor"].as<float>(projectConfig.frameLimitEditor);
        projectConfig.overrideSeed = node["overrideSeed"].as<bool>(projectConfig.overrideSeed);
        projectConfig.seed = node["useFrameLimit"].as<unsigned int>(projectConfig.seed);
        projectConfig.editor = node["editor"].as<bool>(projectConfig.editor);
        projectConfig.iconUUID = node["iconUUID"].as<uint64_t>(projectConfig.iconUUID);
        
        
        return 0;
    }
} // end of Canis namespace
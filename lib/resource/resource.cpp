#include <iostream>
#include <string>
#include <SDL.h>

#include "resource.hpp"

using namespace std;

string getResourcePath(const string &subDir)
{
    static std::string baseRes;
    if (baseRes.empty()) {
        char *basePath = SDL_GetBasePath();
        if (basePath == nullptr) {
            cerr << "[SDL ] Error getting resource path: ";
            cerr << SDL_GetError() << endl;
            return "";
        }
        baseRes = basePath;
        SDL_free(basePath);

        // We replace the last bin/ with res/ to get the the resource path
        size_t pos = baseRes.rfind("bin");
        baseRes = baseRes.substr(0, pos) + "resources/";
    }
    // If we want a specific subdirectory path in the resource directory
    // append it to the base path. This would be something like Lessons/res/Lesson0
    return (subDir.empty()) ? (baseRes) : (baseRes + subDir + "/");
}

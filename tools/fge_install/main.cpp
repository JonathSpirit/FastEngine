/*
 * Copyright 2022 Guillaume Guillet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <optional>
#include <list>
#include <utility>
#include <unordered_set>
#include "re2.h"

enum InstallFileType : uint8_t
{
    FTYPE_HEADER,
    FTYPE_DLL,
    FTYPE_LIB,
    FTYPE_FILE,

    FTYPE_REQUIRE_HEADER,
    FTYPE_REQUIRE_DLL,
    FTYPE_REQUIRE_LIB,
    FTYPE_REQUIRE_FILE
};
enum InstallFileArch : uint8_t
{
    FARCH_32,
    FARCH_64,
    FARCH_ALL
};
enum InstallFileBuild : uint8_t
{
    FBUILD_DEBUG,
    FBUILD_RELEASE,
    FBUILD_ALL
};

std::optional<std::string> EnumToString(InstallFileArch e)
{
    switch (e)
    {
    case FARCH_32:
        return "32";
    case FARCH_64:
        return "64";
    case FARCH_ALL:
        return "all";
    }
    return std::nullopt;
}
std::optional<std::string> EnumToString(InstallFileBuild e)
{
    switch (e)
    {
    case FBUILD_DEBUG:
        return "debug";
    case FBUILD_RELEASE:
        return "release";
    case FBUILD_ALL:
        return "all";
    }
    return std::nullopt;
}

struct BuildDirectory
{
    std::filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
};

struct InstallFile
{
    InstallFile(bool ignored, InstallFileType type, std::filesystem::path path, InstallFileArch arch, InstallFileBuild build) :
            _ignored(ignored),
            _type(type),
            _path(std::move(path)),
            _arch(arch),
            _build(build)
    {
        this->applyExtension();
    }

    InstallFile(bool ignored, InstallFileType type, std::filesystem::path path, InstallFileArch arch, InstallFileBuild build, std::string baseName) :
            _ignored(ignored),
            _type(type),
            _path(std::move(path)),
            _arch(arch),
            _build(build),
            _baseName(std::move(baseName))
    {
        this->applyExtension();
    }

    void applyExtension()
    {
        switch (this->_type)
        {
        case FTYPE_LIB:
#ifdef __linux__
            this->_path += ".so";
#else
#ifdef __APPLE__
            this->_path += ".dylib";
#else
            this->_path += ".dll.a";
#endif //__APPLE__
#endif //__linux__
            break;
        case FTYPE_REQUIRE_LIB:
#ifdef __linux__
            this->_path += ".so";
#else
    #ifdef __APPLE__
            this->_path += ".dylib";
    #else
            this->_path += ".a";
    #endif //__APPLE__
#endif //__linux__
            break;
        default:
            break;
        }
    }

    bool _ignored;
    InstallFileType _type;
    std::filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
    std::string _baseName;
};

std::optional<InstallFileBuild> FindBuildDirectoryBuildType(const std::filesystem::path& buildPath)
{
    RE2 re("libFastEngine(?:32|64)(_d)?\\.(?:dll|so|dylib)");
    if (!re.ok())
    {
        throw std::runtime_error(re.error());
    }

    if ( std::filesystem::is_directory(buildPath) )
    {
        for (const auto& entry : std::filesystem::directory_iterator{buildPath})
        {
            if (entry.is_regular_file())
            {
                std::string buildType;
                if ( RE2::FullMatch(entry.path().filename().string(), re, &buildType) )
                {
                    if (buildType == "_d")
                    {
                        return InstallFileBuild::FBUILD_DEBUG;
                    }
                    return InstallFileBuild::FBUILD_RELEASE;
                }
            }
        }
    }
    return std::nullopt;
}
std::optional<InstallFileArch> FindBuildDirectoryArchitecture(const std::filesystem::path& buildPath)
{
    RE2 re("libFastEngine(32|64)(?:_d)?\\.(?:dll|so|dylib)");
    if (!re.ok())
    {
        throw std::runtime_error(re.error());
    }

    if ( std::filesystem::is_directory(buildPath) )
    {
        for (const auto& entry : std::filesystem::directory_iterator{buildPath})
        {
            if (entry.is_regular_file())
            {
                std::string arch;
                if ( RE2::FullMatch(entry.path().filename().string(), re, &arch) )
                {
                    if (arch == "32")
                    {
                        return InstallFileArch::FARCH_32;
                    }
                    return InstallFileArch::FARCH_64;
                }
            }
        }
    }
    return std::nullopt;
}

std::vector<BuildDirectory> GetPossibleBuildDirectory()
{
    RE2 reBuildDir(".*[bB]uild.*");
    if (!reBuildDir.ok())
    {
        throw std::runtime_error(reBuildDir.error());
    }

    std::vector<BuildDirectory> results;
    std::unordered_set<uint32_t> founded;

    for (const auto& entry : std::filesystem::directory_iterator{"./"})
    {
        if (entry.is_directory() && RE2::FullMatch(entry.path().string(), reBuildDir))
        {
            BuildDirectory buildDirectory;
            buildDirectory._path = entry.path();

            std::cout << "["<<buildDirectory._path<<"]";

            auto buildType = FindBuildDirectoryBuildType(buildDirectory._path);
            auto arch = FindBuildDirectoryArchitecture(buildDirectory._path);

            if (!buildType.has_value())
            {
                std::cout << " <- Can't find the build type !\n";
                continue;
            }
            buildDirectory._build = buildType.value();
            std::cout << "["<<EnumToString(buildDirectory._build).value_or("-")<<"]";
            if (!arch.has_value())
            {
                std::cout << " <- Can't find the build architecture !\n";
                continue;
            }
            buildDirectory._arch = arch.value();
            std::cout << "["<<EnumToString(buildDirectory._arch).value_or("-")<<"]";

            uint32_t key = static_cast<uint32_t>(buildDirectory._build) |
                    (static_cast<uint32_t>(buildDirectory._arch)<<8);

            if (!founded.insert(key).second)
            {
                std::cout << " <- Duplicate ! (ignoring)\n";
                continue;
            }
            std::cout << '\n';
            results.push_back(std::move(buildDirectory));
        }
    }
    return results;
}

bool GetFastEngineVersionName(std::string& name)
{
    std::ifstream versionFile{"includes/FastEngine/fastengine_version.hpp"};

    if (versionFile)
    {
        RE2 re("FGE_VERSION_FULL_WITHTAG_STRING +\"([0-9.]+[-0-9.a-zA-Z]+)\"");
        if (!re.ok())
        {
            throw std::runtime_error(re.error());
        }

        std::string line;
        while ( getline(versionFile, line) )
        {
            if ( RE2::PartialMatch(line, re, &name) )
            {
#ifdef __linux__
                name.insert(0, "FastEngineLinux_");
#else
    #ifdef __APPLE__
                name.insert(0, "FastEngineMac_");
    #else
                name.insert(0, "FastEngineWin_");
    #endif //__APPLE__
#endif //__linux__
                return true;
            }
        }
    }
    return false;
}

int main()
{
    std::cout << "Computing FGE directory name ..." << std::endl;
    std::string fgeName;
    if ( !GetFastEngineVersionName(fgeName) )
    {
        std::cout << "Can't get the FastEngine version name in \"includes/FastEngine/fastengine_version.hpp\"" << std::endl;
        return -1;
    }
    std::cout << "Name : \""<< fgeName <<"\"" << std::endl;

    std::cout << "Computing possible build directories ..." << std::endl;
    auto fgeBuildDir = GetPossibleBuildDirectory();
    if ( fgeBuildDir.empty() )
    {
        std::cout << "No build directories founded, nothing to install !" << std::endl;
        return -1;
    }

    std::cout << "Installing FastEngine project ..." << std::endl;

    std::cout << "Where do you want to install it ?" << std::endl << ">";

    std::string installPathStr;
    getline(std::cin, installPathStr);
    std::filesystem::path installPath{installPathStr};

    if ( installPath.empty() || installPath.has_filename() )
    {
        std::cout << "Invalid path !" << std::endl;
        return -1;
    }

    installPath /= fgeName+"/";

    std::cout << "Check if directory "<< installPath <<" exist ..." << std::endl;
    if ( std::filesystem::is_directory(installPath) )
    {
        std::cout << "A directory is already present ... do you want to remove this directory before proceeding ?" << std::endl;
        std::cout << "[y/n] (default to n)>";
        std::string response;
        getline(std::cin, response);
        if (response == "y")
        {
            std::cout << "Removing ..." << std::endl;
            std::cout << "Removed " << std::filesystem::remove_all(installPath) << " files" << std::endl;
            if ( !std::filesystem::create_directory(installPath) )
            {
                std::cout << "Can't recreate directory : " << installPath << std::endl;
                return -1;
            }
        }
    }
    else
    {
        if (!std::filesystem::create_directories(installPath))
        {
            std::cout << "Can't create directory : " << installPath << std::endl;
            return -1;
        }
    }

    std::cout << "Proceeding with installation ?" << std::endl;
    std::cout << "[y/n] (default to n)>";
    std::string response;
    getline(std::cin, response);
    if (response != "y")
    {
        std::cout << "Aborting ..." << std::endl;
        return 0;
    }

    std::list<InstallFile> installFiles;

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngine32_d.dll", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngine64_d.dll", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngineServer32_d.dll", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngineServer64_d.dll", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngine32.dll", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngine64.dll", FARCH_64, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngineServer32.dll", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, "libFastEngineServer64.dll", FARCH_64, FBUILD_RELEASE);
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngine32_d", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngine64_d", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngineServer32_d", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngineServer64_d", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngine32", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngine64", FARCH_64, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngineServer32", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, "libFastEngineServer64", FARCH_64, FBUILD_RELEASE);

    installFiles.emplace_back(false, FTYPE_HEADER, "includes/FastEngine", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_HEADER, "includes/json.hpp", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_HEADER, "includes/tinyutf8.h", FARCH_ALL, FBUILD_ALL);

    installFiles.emplace_back(false, FTYPE_FILE, "logo.png", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "fge_changelog.txt", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "LICENSE", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "IMAGE_LOGO_LICENSE", FARCH_ALL, FBUILD_ALL);

    //sfml
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-audio-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-system-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-window-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-audio-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-system-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-window-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-audio-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-graphics-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-system-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-window-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-audio-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-graphics-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-system-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/lib/sfml-window-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-audio-d", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-graphics-d", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-system-d", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-window-d", FARCH_32, FBUILD_DEBUG, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-main-d", FARCH_32, FBUILD_DEBUG, "libsfml");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-audio-d", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-graphics-d", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-system-d", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-window-d", FARCH_64, FBUILD_DEBUG, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-main-d", FARCH_64, FBUILD_DEBUG, "libsfml");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-audio", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-graphics", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-system", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-window", FARCH_32, FBUILD_RELEASE, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-main", FARCH_32, FBUILD_RELEASE, "libsfml");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-audio", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-graphics", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-system", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-window", FARCH_64, FBUILD_RELEASE, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "libs/SFML/lib/libsfml-main", FARCH_64, FBUILD_RELEASE, "libsfml");
#endif //_WIN32

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "OpenAL_extern/src/OpenAL_extern-build/OpenAL32.dll", FARCH_32, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "OpenAL_extern/src/OpenAL_extern-build/OpenAL32.dll", FARCH_64, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "OpenAL_extern/src/OpenAL_extern-build/libOpenAL32.dll", FARCH_32, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "OpenAL_extern/src/OpenAL_extern-build/libOpenAL32.dll", FARCH_64, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/openal-soft/include/AL", FARCH_ALL, FBUILD_ALL, "libopenal");

    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/openal-soft/COPYING", FARCH_ALL, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/openal-soft/README.md", FARCH_ALL, FBUILD_ALL, "libopenal");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/SFML/include/SFML", FARCH_ALL, FBUILD_ALL, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/SFML/license.md", FARCH_ALL, FBUILD_ALL, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/SFML/readme.md", FARCH_ALL, FBUILD_ALL, "libsfml");

    //Ignoring un-built files and prefix path to the built ones
    for (auto itFile=installFiles.begin(); itFile!=installFiles.end(); ++itFile)
    {
        auto& file = *itFile;

        bool ok = false;
        for (const auto& buildDir : fgeBuildDir)
        {
            if ( (buildDir._arch == file._arch || file._arch == InstallFileArch::FARCH_ALL) &&
                (buildDir._build == file._build || file._build == InstallFileBuild::FBUILD_ALL) )
            {
                ok = true;
                if (file._type != InstallFileType::FTYPE_HEADER &&
                    file._type != InstallFileType::FTYPE_REQUIRE_HEADER &&
                    file._type != InstallFileType::FTYPE_FILE &&
                    file._type != InstallFileType::FTYPE_REQUIRE_FILE)
                {
                    std::filesystem::path newPath = buildDir._path;
                    newPath /= file._path;
                    file._path = std::move(newPath);
                }
                break;
            }
        }
        if (!ok)
        {
            itFile = --installFiles.erase(itFile);
        }
    }

    std::cout << "Checking for required files ..." << std::endl;

    for (auto itFile=installFiles.begin(); itFile!=installFiles.end(); ++itFile)
    {
        auto& file = *itFile;

        std::cout << "\tChecking " << file._path << "... ";
        if ( !std::filesystem::is_regular_file(file._path) && !std::filesystem::is_directory(file._path) )
        {//Is a file or directory
            if (file._ignored)
            {
                std::cout << "not ok !, but can be ignored !" << std::endl;
                itFile = --installFiles.erase(itFile);
                continue;
            }
            std::cout << "not ok !, not found ! (not a file or directory)" << std::endl;
            return -1;
        }
        std::cout << "ok !" << std::endl;
    }

    for (const auto& file : installFiles)
    {
        std::cout << "\tInstalling " << file._path << "... ";

        std::string baseName = file._baseName + ((file._build==FBUILD_DEBUG) ? "_d" : "");

        std::filesystem::path fileFolderPath = installPath;
        switch (file._type)
        {
        case FTYPE_HEADER:
            fileFolderPath /= "include/";
            break;
        case FTYPE_DLL:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "bin32/";
                break;
            case FARCH_64:
                fileFolderPath /= "bin64/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "bin/";
                break;
            }
            break;
        case FTYPE_LIB:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "lib32/";
                break;
            case FARCH_64:
                fileFolderPath /= "lib64/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "lib/";
                break;
            }
            break;
        case FTYPE_FILE:
            break;

        case FTYPE_REQUIRE_HEADER:
            fileFolderPath /= "require/lib/" + baseName + "/" + "include/";
            break;
        case FTYPE_REQUIRE_DLL:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + baseName + "/" + "bin/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + baseName + "/" + "bin/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + baseName + "/" + "bin/";
                break;
            }
            break;
        case FTYPE_REQUIRE_LIB:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + baseName + "/" + "lib/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + baseName + "/" + "lib/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + baseName + "/" + "lib/";
                break;
            }
            break;
        case FTYPE_REQUIRE_FILE:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + baseName + "/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + baseName + "/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + baseName + "/";
                break;
            }
            break;
        }

        fileFolderPath += file._path.filename();

        std::error_code err;
        const auto options = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

        std::filesystem::create_directories(fileFolderPath.parent_path(), err);
        if (err)
        {
            std::cout << "not ok !, " << err.message() << std::endl;
            std::cout << "target: " << fileFolderPath << std::endl;
            return -1;
        }
        std::filesystem::copy(file._path, fileFolderPath, options, err);
        if (err)
        {
            std::cout << "not ok !, " << err.message() << std::endl;
            std::cout << "target: " << fileFolderPath << std::endl;
            return -2;
        }
        std::cout << "ok !" << std::endl;
    }

    std::cout << "everything is good !" << std::endl;

    return 0;
}

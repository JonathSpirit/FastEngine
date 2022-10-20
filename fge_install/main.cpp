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
#include <list>
#include <utility>

enum InstallFileType
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
enum InstallFileArch
{
    FARCH_32,
    FARCH_64,
    FARCH_ALL
};
enum InstallFileBuild
{
    FBUILD_DEBUG,
    FBUILD_RELEASE,
    FBUILD_ALL
};

struct InstallFile
{
    InstallFile(bool ignored, InstallFileType type, std::filesystem::path path, InstallFileArch arch, InstallFileBuild build) :
            _ignored(ignored),
            _type(type),
            _path(std::move(path)),
            _arch(arch),
            _build(build)
    {}

    InstallFile(bool ignored, InstallFileType type, std::filesystem::path path, InstallFileArch arch, InstallFileBuild build, std::string&& name) :
            _ignored(ignored),
            _type(type),
            _path(std::move(path)),
            _arch(arch),
            _build(build),
            _name(std::move(name))
    {}

    InstallFile() :
        _ignored(true),
        _type(),
        _path(),
        _arch(),
        _build(),
        _name()
    {}

    bool _ignored;
    InstallFileType _type;
    std::filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
    std::string _name;
};

bool GetFGEversionName(std::string& name)
{
    std::ifstream versionFile{"includes/FastEngine/fastengine_version.hpp"};

    if (versionFile)
    {
        std::string line;
        while ( getline(versionFile, line) )
        {
            auto pos = line.find("FGE_VERSION_FULL_WITHTAG_STRING");
            if ( pos != std::string::npos )
            {
                auto posQuote = line.find('\"');
                auto posEndQuote = line.rfind('\"');
                if ( posQuote != std::string::npos && posEndQuote != std::string::npos )
                {
                    if (posEndQuote-posQuote-1 >= 5)
                    {
                        name = "FastEngine_" + line.substr(posQuote+1, posEndQuote-posQuote-1);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int main()
{
    std::cout << "Installing fastengine project ..." << std::endl;

    std::cout << "Where do you want to install it ?" << std::endl << ">";

    std::string installPathStr;
    getline(std::cin, installPathStr);
    std::filesystem::path installPath{installPathStr};

    if ( installPath.empty() || installPath.has_filename() )
    {
        std::cout << "Invalid path !" << std::endl;
        return -1;
    }

    std::cout << "Computing FGE directory name ..." << std::endl;
    std::string fgeName;
    if ( !GetFGEversionName(fgeName) )
    {
        std::cout << "Can't get the fastengine version name in \"includes/FastEngine/fastengine_version.hpp\"" << std::endl;
        return -1;
    }
    std::cout << "Name : \""<< fgeName <<"\"" << std::endl;

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

#ifdef __linux__
    std::filesystem::path pathBuild32DebugDir = "cmake-build-debug32linux/";
    std::filesystem::path pathBuild32ReleaseDir = "cmake-build-release32linux/";
    std::filesystem::path pathBuild64DebugDir = "cmake-build-debug64linux/";
    std::filesystem::path pathBuild64ReleaseDir = "cmake-build-release64linux/";

    std::filesystem::path pathLibExtension = ".so";
    std::filesystem::path pathSfmlLibExtension = ".so";
#else
    std::filesystem::path pathBuild32DebugDir = "cmake-build-debug32/";
    std::filesystem::path pathBuild32ReleaseDir = "cmake-build-release32/";
    std::filesystem::path pathBuild64DebugDir = "cmake-build-debug64/";
    std::filesystem::path pathBuild64ReleaseDir = "cmake-build-release64/";

    std::filesystem::path pathLibExtension = ".dll.a";
    std::filesystem::path pathSfmlLibExtension = ".a";
#endif //__linux__

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild32DebugDir/"libFastEngine32_d.dll", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild64DebugDir/"libFastEngine64_d.dll", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild32DebugDir/"libFastEngineServer32_d.dll", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild64DebugDir/"libFastEngineServer64_d.dll", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild32ReleaseDir/"libFastEngine32.dll", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild64ReleaseDir/"libFastEngine64.dll", FARCH_64, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild32ReleaseDir/"libFastEngineServer32.dll", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, pathBuild64ReleaseDir/"libFastEngineServer64.dll", FARCH_64, FBUILD_RELEASE);
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_LIB, pathBuild32DebugDir/"libFastEngine32_d"+=pathLibExtension, FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild64DebugDir/"libFastEngine64_d"+=pathLibExtension, FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild32DebugDir/"libFastEngineServer32_d"+=pathLibExtension, FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild64DebugDir/"libFastEngineServer64_d"+=pathLibExtension, FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild32ReleaseDir/"libFastEngine32"+=pathLibExtension, FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild64ReleaseDir/"libFastEngine64"+=pathLibExtension, FARCH_64, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild32ReleaseDir/"libFastEngineServer32"+=pathLibExtension, FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, pathBuild64ReleaseDir/"libFastEngineServer64"+=pathLibExtension, FARCH_64, FBUILD_RELEASE);

    installFiles.emplace_back(false, FTYPE_HEADER, "includes/FastEngine", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_HEADER, "includes/json.hpp", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_HEADER, "includes/tinyutf8.h", FARCH_ALL, FBUILD_ALL);

    installFiles.emplace_back(false, FTYPE_FILE, "logo.png", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "fge_changelog.txt", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "LICENSE", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "IMAGE_LOGO_LICENSE", FARCH_ALL, FBUILD_ALL);

    //sfml
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32DebugDir/"libs/SFML/lib/sfml-audio-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32DebugDir/"libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32DebugDir/"libs/SFML/lib/sfml-system-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32DebugDir/"libs/SFML/lib/sfml-window-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
#endif //_WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32DebugDir/"libs/SFML/lib/libsfml-audio-d"+=pathSfmlLibExtension, FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32DebugDir/"libs/SFML/lib/libsfml-graphics-d"+=pathSfmlLibExtension, FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32DebugDir/"libs/SFML/lib/libsfml-system-d"+=pathSfmlLibExtension, FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32DebugDir/"libs/SFML/lib/libsfml-window-d"+=pathSfmlLibExtension, FARCH_32, FBUILD_DEBUG, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32DebugDir/"libs/SFML/lib/libsfml-main-d"+=pathSfmlLibExtension, FARCH_32, FBUILD_DEBUG, "libsfml");
#endif //_WIN32

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64DebugDir/"libs/SFML/lib/sfml-audio-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64DebugDir/"libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64DebugDir/"libs/SFML/lib/sfml-system-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64DebugDir/"libs/SFML/lib/sfml-window-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
#endif //_WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64DebugDir/"libs/SFML/lib/libsfml-audio-d"+=pathSfmlLibExtension, FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64DebugDir/"libs/SFML/lib/libsfml-graphics-d"+=pathSfmlLibExtension, FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64DebugDir/"libs/SFML/lib/libsfml-system-d"+=pathSfmlLibExtension, FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64DebugDir/"libs/SFML/lib/libsfml-window-d"+=pathSfmlLibExtension, FARCH_64, FBUILD_DEBUG, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64DebugDir/"libs/SFML/lib/libsfml-main-d"+=pathSfmlLibExtension, FARCH_64, FBUILD_DEBUG, "libsfml");
#endif //_WIN32

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32ReleaseDir/"libs/SFML/lib/sfml-audio-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32ReleaseDir/"libs/SFML/lib/sfml-graphics-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32ReleaseDir/"libs/SFML/lib/sfml-system-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32ReleaseDir/"libs/SFML/lib/sfml-window-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
#endif //_WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"libs/SFML/lib/libsfml-audio"+=pathSfmlLibExtension, FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"libs/SFML/lib/libsfml-graphics"+=pathSfmlLibExtension, FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"libs/SFML/lib/libsfml-system"+=pathSfmlLibExtension, FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"libs/SFML/lib/libsfml-window"+=pathSfmlLibExtension, FARCH_32, FBUILD_RELEASE, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"libs/SFML/lib/libsfml-main"+=pathSfmlLibExtension, FARCH_32, FBUILD_RELEASE, "libsfml");
#endif //_WIN32

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64ReleaseDir/"libs/SFML/lib/sfml-audio-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64ReleaseDir/"libs/SFML/lib/sfml-graphics-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64ReleaseDir/"libs/SFML/lib/sfml-system-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64ReleaseDir/"libs/SFML/lib/sfml-window-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
#endif //_WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"libs/SFML/lib/libsfml-audio"+=pathSfmlLibExtension, FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"libs/SFML/lib/libsfml-graphics"+=pathSfmlLibExtension, FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"libs/SFML/lib/libsfml-system"+=pathSfmlLibExtension, FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"libs/SFML/lib/libsfml-window"+=pathSfmlLibExtension, FARCH_64, FBUILD_RELEASE, "libsfml");
#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"libs/SFML/lib/libsfml-main"+=pathSfmlLibExtension, FARCH_64, FBUILD_RELEASE, "libsfml");
#endif //_WIN32

#ifdef _WIN32
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild32ReleaseDir/"OpenAL_extern/src/OpenAL_extern-build/OpenAL32.dll", FARCH_32, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, pathBuild64ReleaseDir/"OpenAL_extern/src/OpenAL_extern-build/OpenAL32.dll", FARCH_64, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild32ReleaseDir/"OpenAL_extern/src/OpenAL_extern-build/libOpenAL32.dll.a", FARCH_32, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, pathBuild64ReleaseDir/"OpenAL_extern/src/OpenAL_extern-build/libOpenAL32.dll.a", FARCH_64, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/openal-soft/include/AL", FARCH_ALL, FBUILD_ALL, "libopenal");

    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/openal-soft/COPYING", FARCH_ALL, FBUILD_ALL, "libopenal");
    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/openal-soft/README.md", FARCH_ALL, FBUILD_ALL, "libopenal");
#endif //_WIN32

    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/SFML/include/SFML", FARCH_ALL, FBUILD_ALL, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/SFML/license.md", FARCH_ALL, FBUILD_ALL, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/SFML/readme.md", FARCH_ALL, FBUILD_ALL, "libsfml");

    std::cout << "Checking for required files ..." << std::endl;

    for (auto itFile=installFiles.begin(); itFile!=installFiles.end(); ++itFile)
    {
        auto& file = *itFile;

        std::cout << "\tChecking " << file._path << " ";
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
        std::cout << "\tInstalling " << file._path << " ";

        std::string fileName = file._name + ((file._build==FBUILD_DEBUG) ? "_d" : "");

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
            fileFolderPath /= "require/lib/" + fileName + "/" + "include/";
            break;
        case FTYPE_REQUIRE_DLL:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + fileName + "/" + "bin/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + fileName + "/" + "bin/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + fileName + "/" + "bin/";
                break;
            }
            break;
        case FTYPE_REQUIRE_LIB:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + fileName + "/" + "lib/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + fileName + "/" + "lib/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + fileName + "/" + "lib/";
                break;
            }
            break;
        case FTYPE_REQUIRE_FILE:
            switch (file._arch)
            {
            case FARCH_32:
                fileFolderPath /= "require/lib32/" + fileName + "/";
                break;
            case FARCH_64:
                fileFolderPath /= "require/lib64/" + fileName + "/";
                break;
            case FARCH_ALL:
                fileFolderPath /= "require/lib/" + fileName + "/";
                break;
            }
            break;
        }

        fileFolderPath += file._path.filename();

        std::error_code err;
        const auto options = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

        std::cout << "in " << fileFolderPath << " ";
        std::filesystem::create_directories(fileFolderPath.parent_path(), err);
        if (err)
        {
            std::cout << "not ok !, " << err.message() << std::endl;
            return -1;
        }
        std::filesystem::copy(file._path, fileFolderPath, options, err);
        if (err)
        {
            std::cout << "not ok !, " << err.message() << std::endl;
            return -2;
        }
        std::cout << "ok !" << std::endl;
    }

    std::cout << "everything is good !" << std::endl;

    return 0;
}

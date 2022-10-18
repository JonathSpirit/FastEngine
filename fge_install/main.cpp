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

using namespace std;

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
    InstallFile(bool ignored, InstallFileType type, filesystem::path path, InstallFileArch arch, InstallFileBuild build) :
            _ignored(ignored),
            _type(type),
            _path(std::move(path)),
            _arch(arch),
            _build(build)
    {}

    InstallFile(bool ignored, InstallFileType type, filesystem::path path, InstallFileArch arch, InstallFileBuild build, std::string&& name) :
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
    filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
    std::string _name;
};

bool GetFGEversionName(string& name)
{
    ifstream versionFile{"includes/FastEngine/fastengine_version.hpp"};

    if (versionFile)
    {
        string line;
        while ( getline(versionFile, line) )
        {
            auto pos = line.find("FGE_VERSION_FULL_WITHTAG_STRING");
            if ( pos != string::npos )
            {
                auto posQuote = line.find('\"');
                auto posEndQuote = line.rfind('\"');
                if ( posQuote != string::npos && posEndQuote != string::npos )
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
    cout << "Installing fastengine project ..." << endl;

    cout << "Where do you want to install it ?" << endl << ">";

    string installPathStr;
    getline(cin, installPathStr);
    filesystem::path installPath{installPathStr};

    if ( installPath.empty() || installPath.has_filename() )
    {
        cout << "Invalid path !" << endl;
        return -1;
    }

    cout << "Computing FGE directory name ..." << endl;
    string fgeName;
    if ( !GetFGEversionName(fgeName) )
    {
        cout << "Can't get the fastengine version name in \"includes/FastEngine/fastengine_version.hpp\"" << endl;
        return -1;
    }
    cout << "Name : \""<< fgeName <<"\"" << endl;

    installPath /= fgeName+"/";

    cout << "Check if directory "<< installPath <<" exist ..." << endl;
    if ( filesystem::is_directory(installPath) )
    {
        cout << "A directory is already present ... do you want to remove this directory before proceeding ?" << endl;
        cout << "[y/n] (default to n)>";
        string response;
        getline(cin, response);
        if (response == "y")
        {
            cout << "Removing ..." << endl;
            cout << "Removed " << filesystem::remove_all(installPath) << " files" << endl;
            if ( !filesystem::create_directory(installPath) )
            {
                cout << "Can't recreate directory : " << installPath << endl;
                return -1;
            }
        }
    }
    else
    {
        if (!filesystem::create_directories(installPath))
        {
            cout << "Can't create directory : " << installPath << endl;
            return -1;
        }
    }

    cout << "Proceeding with installation ?" << endl;
    cout << "[y/n] (default to n)>";
    string response;
    getline(cin, response);
    if (response != "y")
    {
        cout << "Aborting ..." << endl;
        return 0;
    }

    list<InstallFile> installFiles;

#ifdef __linux__
    filesystem::path pathBuild32DebugDir = "cmake-build-debug32linux/";
    filesystem::path pathBuild32ReleaseDir = "cmake-build-release32linux/";
    filesystem::path pathBuild64DebugDir = "cmake-build-debug64linux/";
    filesystem::path pathBuild64ReleaseDir = "cmake-build-release64linux/";

    filesystem::path pathLibExtension = ".so";
    filesystem::path pathSfmlLibExtension = ".so";
#else
    filesystem::path pathBuild32DebugDir = "cmake-build-debug32/";
    filesystem::path pathBuild32ReleaseDir = "cmake-build-release32/";
    filesystem::path pathBuild64DebugDir = "cmake-build-debug64/";
    filesystem::path pathBuild64ReleaseDir = "cmake-build-release64/";

    filesystem::path pathLibExtension = ".dll.a";
    filesystem::path pathSfmlLibExtension = ".a";
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

    cout << "Checking for required files ..." << endl;

    for (auto itFile=installFiles.begin(); itFile!=installFiles.end(); ++itFile)
    {
        auto& file = *itFile;

        cout << "\tChecking " << file._path << " ";
        if ( !filesystem::is_regular_file(file._path) && !filesystem::is_directory(file._path) )
        {//Is a file or directory
            if (file._ignored)
            {
                cout << "not ok !, but can be ignored !" << endl;
                itFile = --installFiles.erase(itFile);
                continue;
            }
            cout << "not ok !, not found ! (not a file or directory)" << endl;
            return -1;
        }
        cout << "ok !" << endl;
    }

    for (const auto& file : installFiles)
    {
        cout << "\tInstalling " << file._path << " ";

        std::string fileName = file._name + ((file._build==FBUILD_DEBUG) ? "_d" : "");

        filesystem::path fileFolderPath = installPath;
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

        error_code err;
        const auto options = filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive;

        cout << "in " << fileFolderPath << " ";
        filesystem::create_directories(fileFolderPath.parent_path(), err);
        if (err)
        {
            cout << "not ok !, " << err.message() << endl;
            return -1;
        }
        filesystem::copy(file._path, fileFolderPath, options, err);
        if (err)
        {
            cout << "not ok !, " << err.message() << endl;
            return -2;
        }
        cout << "ok !" << endl;
    }

    cout << "everything is good !" << endl;

    return 0;
}
/*
    bool _required;
    InstallFileType _type;
    filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
    std::string _name;
*/
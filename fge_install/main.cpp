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
            auto pos = line.find("VERSION_FULLVERSION_STRING");
            if ( pos != string::npos )
            {
                auto posQuote = line.find('\"');
                auto posEndQuote = line.rfind('\"');
                if ( posQuote != string::npos && posEndQuote != string::npos )
                {
                    if (posEndQuote-posQuote-1 >= 7)
                    {
                        auto dotCount = count(line.begin()+posQuote, line.begin()+posEndQuote, '.');
                        if (dotCount == 3)
                        {
                            name.clear();
                            auto posDotEnd = line.rfind('.');
                            if (posDotEnd != string::npos && posDotEnd > posQuote)
                            {
                                name = "FastEngine_" + line.substr(posQuote+1, posDotEnd-posQuote-1);
                                return true;
                            }
                        }
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

    list<InstallFile> installFiles;

    installFiles.emplace_back(false, FTYPE_DLL, "cmake-build-debug32/libFastEngine32_d.dll", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "cmake-build-debug64/libFastEngine64_d.dll", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_DLL, "cmake-build-release32/libFastEngine32.dll", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_DLL, "cmake-build-release64/libFastEngine64.dll", FARCH_64, FBUILD_RELEASE);

    installFiles.emplace_back(false, FTYPE_LIB, "cmake-build-debug32/libFastEngine32_d.dll.a", FARCH_32, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "cmake-build-debug64/libFastEngine64_d.dll.a", FARCH_64, FBUILD_DEBUG);
    installFiles.emplace_back(false, FTYPE_LIB, "cmake-build-release32/libFastEngine32.dll.a", FARCH_32, FBUILD_RELEASE);
    installFiles.emplace_back(false, FTYPE_LIB, "cmake-build-release64/libFastEngine64.dll.a", FARCH_64, FBUILD_RELEASE);

    installFiles.emplace_back(false, FTYPE_HEADER, "includes/FastEngine", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_HEADER, "includes/json.hpp", FARCH_ALL, FBUILD_ALL);

    installFiles.emplace_back(false, FTYPE_FILE, "logo.png", FARCH_ALL, FBUILD_ALL);
    installFiles.emplace_back(false, FTYPE_FILE, "fengine_changelog.txt", FARCH_ALL, FBUILD_ALL);

    //pcg-cpp
    installFiles.emplace_back(false, FTYPE_REQUIRE_FILE, "libs/pcg-cpp/LICENSE.txt", FARCH_ALL, FBUILD_ALL, "libpcg-cpp");
    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/pcg-cpp/include/pcg_extras.hpp", FARCH_ALL, FBUILD_ALL, "libpcg-cpp");
    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/pcg-cpp/include/pcg_random.hpp", FARCH_ALL, FBUILD_ALL, "libpcg-cpp");
    installFiles.emplace_back(false, FTYPE_REQUIRE_HEADER, "libs/pcg-cpp/include/pcg_uint128.hpp", FARCH_ALL, FBUILD_ALL, "libpcg-cpp");

    //sfml
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug32/libs/SFML/lib/sfml-audio-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug32/libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug32/libs/SFML/lib/sfml-system-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug32/libs/SFML/lib/sfml-window-d-2.dll", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug32/libs/SFML/lib/libsfml-audio-d.a", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug32/libs/SFML/lib/libsfml-graphics-d.a", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug32/libs/SFML/lib/libsfml-system-d.a", FARCH_32, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug32/libs/SFML/lib/libsfml-window-d.a", FARCH_32, FBUILD_DEBUG, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug64/libs/SFML/lib/sfml-audio-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug64/libs/SFML/lib/sfml-graphics-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug64/libs/SFML/lib/sfml-system-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-debug64/libs/SFML/lib/sfml-window-d-2.dll", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug64/libs/SFML/lib/libsfml-audio-d.a", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug64/libs/SFML/lib/libsfml-graphics-d.a", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug64/libs/SFML/lib/libsfml-system-d.a", FARCH_64, FBUILD_DEBUG, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-debug64/libs/SFML/lib/libsfml-window-d.a", FARCH_64, FBUILD_DEBUG, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release32/libs/SFML/lib/sfml-audio-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release32/libs/SFML/lib/sfml-graphics-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release32/libs/SFML/lib/sfml-system-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release32/libs/SFML/lib/sfml-window-2.dll", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release32/libs/SFML/lib/libsfml-audio.a", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release32/libs/SFML/lib/libsfml-graphics.a", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release32/libs/SFML/lib/libsfml-system.a", FARCH_32, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release32/libs/SFML/lib/libsfml-window.a", FARCH_32, FBUILD_RELEASE, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release64/libs/SFML/lib/sfml-audio-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release64/libs/SFML/lib/sfml-graphics-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release64/libs/SFML/lib/sfml-system-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "cmake-build-release64/libs/SFML/lib/sfml-window-2.dll", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release64/libs/SFML/lib/libsfml-audio.a", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release64/libs/SFML/lib/libsfml-graphics.a", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release64/libs/SFML/lib/libsfml-system.a", FARCH_64, FBUILD_RELEASE, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_LIB, "cmake-build-release64/libs/SFML/lib/libsfml-window.a", FARCH_64, FBUILD_RELEASE, "libsfml");

    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/extlibs/bin/x64/openal32.dll", FARCH_64, FBUILD_ALL, "libsfml");
    installFiles.emplace_back(false, FTYPE_REQUIRE_DLL, "libs/SFML/extlibs/bin/x86/openal32.dll", FARCH_32, FBUILD_ALL, "libsfml");

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
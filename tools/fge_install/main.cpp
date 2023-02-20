/*
 * Copyright 2023 Guillaume Guillet
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
#include <initializer_list>
#include "re2.h"

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
    FARCH_UNSPECIFIED
};
enum InstallFileBuild
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
    case FARCH_UNSPECIFIED:
        return "unspecified";
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

void ReplaceStringWords(std::string& string, const std::string& oldWord, const std::string& newWord)
{
    std::size_t pos = 0;

    while ((pos = string.find(oldWord, pos)) != std::string::npos)
    {
        string.replace(pos, oldWord.length(), newWord);
        pos += newWord.length();
    }
}

struct BuildDirectory
{
    std::filesystem::path _path;
    InstallFileArch _arch;
    InstallFileBuild _build;
};

class InstallFile
{
public:
    InstallFile(InstallFileType type, std::filesystem::path path,
                std::initializer_list<InstallFileArch> arch,
                std::initializer_list<InstallFileBuild> build) :
            _ignored(false),
            _type(type),
            _arch(arch),
            _build(build),
            g_path(std::move(path))
    {
        this->applyExtension();
    }

    InstallFile(InstallFileType type, std::filesystem::path path,
                std::initializer_list<InstallFileArch> arch,
                std::initializer_list<InstallFileBuild> build, std::string baseName) :
            _ignored(false),
            _type(type),
            _arch(arch),
            _build(build),
            _baseName(std::move(baseName)),
            g_path(std::move(path))
    {
        this->applyExtension();
    }

    [[nodiscard]] std::filesystem::path getPath(InstallFileArch targetArch, InstallFileBuild targetBuild) const
    {
        std::string pathString = this->g_path.string();

        std::string archString;
        switch (targetArch)
        {
        case FARCH_32:
            archString = "32";
            break;
        case FARCH_64:
            archString = "64";
            break;
        default:
            archString = "";
            break;
        }

        std::string buildString;
        switch (targetBuild)
        {
        case FBUILD_DEBUG:
            buildString = "_d";
            break;
        default:
            buildString = "";
            break;
        }

        ReplaceStringWords(pathString, "%ARCH%", archString);
        ReplaceStringWords(pathString, "%BUILD%", buildString);

        return pathString;
    }

    [[nodiscard]] bool checkBuildType(InstallFileArch arch, InstallFileBuild build) const
    {
        return (std::find(this->_arch.begin(), this->_arch.end(), arch) != this->_arch.end() || this->_arch.empty()) &&
               (std::find(this->_build.begin(), this->_build.end(), build) != this->_build.end() || this->_build.empty());
    }

    [[nodiscard]] bool applyBuildDirPath(std::filesystem::path& path, std::filesystem::path buildDirPath) const
    {
        if (this->_type != InstallFileType::FTYPE_HEADER &&
            this->_type != InstallFileType::FTYPE_REQUIRE_HEADER &&
            this->_type != InstallFileType::FTYPE_FILE &&
            this->_type != InstallFileType::FTYPE_REQUIRE_FILE)
        {
            buildDirPath /= path;
            path = std::move(buildDirPath);
            return true;
        }
        return false;
    }

    void applyExtension()
    {
        switch (this->_type)
        {
        case FTYPE_LIB:
#ifdef __linux__
            this->g_path += ".so";
#else
#ifdef __APPLE__
            this->g_path += ".dylib";
#else
            this->g_path += ".dll.a";
#endif //__APPLE__
#endif //__linux__
            break;
        case FTYPE_REQUIRE_LIB:
#ifdef __linux__
            this->g_path += ".so";
#else
    #ifdef __APPLE__
            this->g_path += ".dylib";
    #else
            this->g_path += ".a";
    #endif //__APPLE__
#endif //__linux__
            break;
        default:
            break;
        }
    }

    bool _ignored;
    InstallFileType _type;
    std::vector<InstallFileArch> _arch;
    std::vector<InstallFileBuild> _build;
    std::string _baseName;

private:
    std::filesystem::path g_path;
};

std::optional<InstallFileBuild> FindBuildDirectoryBuildType(const std::filesystem::path& buildPath)
{
    const RE2 re("libFastEngine(?:32|64)(_d)?\\.(?:dll|so|dylib)");
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
    const RE2 re("libFastEngine(32|64)(?:_d)?\\.(?:dll|so|dylib)");
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

            const uint32_t key = static_cast<uint32_t>(buildDirectory._build) |
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
    installFiles.push_back({FTYPE_DLL, "libFastEngine%ARCH%%BUILD%.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG, FBUILD_RELEASE}});
    installFiles.push_back({FTYPE_DLL, "libFastEngineServer%ARCH%%BUILD%.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG, FBUILD_RELEASE}});
#endif //_WIN32

    installFiles.push_back({FTYPE_LIB, "libFastEngine%ARCH%%BUILD%", {FARCH_32, FARCH_64}, {FBUILD_DEBUG, FBUILD_RELEASE}});
    installFiles.push_back({FTYPE_LIB, "libFastEngineServer%ARCH%%BUILD%", {FARCH_32, FARCH_64}, {FBUILD_DEBUG, FBUILD_RELEASE}});

    installFiles.push_back({FTYPE_HEADER, "includes/FastEngine", {}, {}});
    installFiles.push_back({FTYPE_HEADER, "includes/json.hpp", {}, {}});
    installFiles.push_back({FTYPE_HEADER, "includes/tinyutf8.h", {}, {}});

    installFiles.push_back({FTYPE_FILE, "logo.png", {}, {}});
    installFiles.push_back({FTYPE_FILE, "fge_changelog.txt", {}, {}});
    installFiles.push_back({FTYPE_FILE, "LICENSE", {}, {}});
    installFiles.push_back({FTYPE_FILE, "IMAGE_AUDIO_LOGO_LICENSE", {}, {}});
    installFiles.push_back({FTYPE_FILE, "tools/CMakeInterfaceLibs.txt", {}, {}});

    //sdl
#ifdef _WIN32
    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL/SDL2d.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL_image/SDL2_imaged.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL_mixer/SDL2_mixerd.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_mixer"});

    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL/SDL2.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL_image/SDL2_image.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_DLL, "libs/SDL_mixer/SDL2_mixer.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_mixer"});
#endif //_WIN32

#ifdef _WIN32
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2maind", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2d.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_image/libSDL2_imaged.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_mixer/libSDL2_mixerd.dll", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_mixer"});
#else
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2d", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_image/libSDL2_imaged", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_mixer/libSDL2_mixerd", {FARCH_32, FARCH_64}, {FBUILD_DEBUG}, "libsdl_mixer"});
#endif //_WIN32

#ifdef _WIN32
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2main", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_image/libSDL2_image.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_mixer/libSDL2_mixer.dll", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_mixer"});
#else
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL/libSDL2", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_image/libSDL2_image", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_LIB, "libs/SDL_mixer/libSDL2_mixer", {FARCH_32, FARCH_64}, {FBUILD_RELEASE}, "libsdl_mixer"});
#endif //_WIN32

    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/SDL/include", {}, {}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/SDL_image/SDL_image.h", {}, {}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/SDL_mixer/include/SDL_mixer.h", {}, {}, "libsdl_mixer"});

    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/SDL/LICENSE.txt", {}, {}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/SDL/CREDITS.txt", {}, {}, "libsdl"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/SDL_image/LICENSE.txt", {}, {}, "libsdl_image"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/SDL_mixer/LICENSE.txt", {}, {}, "libsdl_mixer"});

    //volk
    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/volk/volk.h", {}, {}, "volk"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/volk/LICENSE.md", {}, {}, "volk"});

    //VulkanMemoryAllocator
    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/VulkanMemoryAllocator/include/vk_mem_alloc.h", {}, {}, "vma"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/VulkanMemoryAllocator/LICENSE.txt", {}, {}, "vma"});

    //glm
    installFiles.push_back({FTYPE_REQUIRE_HEADER, "libs/glm/glm", {}, {}, "glm"});
    installFiles.push_back({FTYPE_REQUIRE_FILE, "libs/glm/copying.txt", {}, {}, "glm"});

    //Starting installation

    std::cout << "Checking for required files ..." << std::endl;

    for (auto& file : installFiles)
    {
        for (const auto& buildDir : fgeBuildDir)
        {
            if (!file.checkBuildType(buildDir._arch, buildDir._build))
            {
                continue;
            }

            auto filePath = file.getPath(buildDir._arch, buildDir._build);

            bool isUnique = false;
            if (!file.applyBuildDirPath(filePath, buildDir._path))
            {//unique file
                isUnique = true;
            }

            std::cout << "\tChecking " << filePath << "... ";
            if ( !std::filesystem::is_regular_file(filePath) && !std::filesystem::is_directory(filePath) )
            {//Is a file or directory
                if (file._ignored)
                {
                    std::cout << "not ok !, but can be ignored !" << std::endl;
                    continue;
                }
                std::cout << "not ok !, not found ! (not a file or directory)" << std::endl;
                return -1;
            }
            std::cout << "ok !" << std::endl;

            if (isUnique)
            {
                break;
            }
        }
    }

    for (const auto& file : installFiles)
    {
        for (const auto& buildDir : fgeBuildDir)
        {
            if (!file.checkBuildType(buildDir._arch, buildDir._build))
            {
                continue;
            }

            auto filePath = file.getPath(buildDir._arch, buildDir._build);

            bool isUnique = false;
            if (!file.applyBuildDirPath(filePath, buildDir._path))
            {//unique file
                isUnique = true;
            }

            std::cout << "\tInstalling " << filePath << "... ";

            std::string baseName = file._baseName;
            if (std::find(file._build.begin(), file._build.end(), FBUILD_DEBUG) != file._build.end())
            {
                baseName += "_d";
            }

            std::string archString;
            if (file._arch.empty())
            { //Unspecified arch
                archString = "/";
            }
            else
            {
                switch (buildDir._arch)
                {
                case FARCH_32:
                    archString = "32/";
                    break;
                case FARCH_64:
                    archString = "64/";
                    break;
                default:
                    archString = "/";
                    break;
                }
            }

            std::filesystem::path fileFolderPath = installPath;
            switch (file._type)
            {
            case FTYPE_HEADER:
                fileFolderPath /= "include/";
                break;
            case FTYPE_DLL:
                fileFolderPath /= "bin"+archString;
                break;
            case FTYPE_LIB:
                fileFolderPath /= "lib"+archString;
                break;
            case FTYPE_FILE:
                break;

            case FTYPE_REQUIRE_HEADER:
                fileFolderPath /= "require/lib/" + baseName + "/" + "include/";
                break;
            case FTYPE_REQUIRE_DLL:
                fileFolderPath /= "require/lib" + archString + baseName + "/" + "bin/";
                break;
            case FTYPE_REQUIRE_LIB:
                fileFolderPath /= "require/lib" + archString + baseName + "/" + "lib/";
                break;
            case FTYPE_REQUIRE_FILE:
                fileFolderPath /= "require/lib" + archString + baseName + "/";
                break;
            }

            fileFolderPath += filePath.filename();

            std::error_code err;
            const auto options = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

            std::filesystem::create_directories(fileFolderPath.parent_path(), err);
            if (err)
            {
                std::cout << "not ok !, " << err.message() << std::endl;
                std::cout << "target: " << fileFolderPath << std::endl;
                return -1;
            }
            std::filesystem::copy(filePath, fileFolderPath, options, err);
            if (err)
            {
                std::cout << "not ok !, " << err.message() << std::endl;
                std::cout << "target: " << fileFolderPath << std::endl;
                return -2;
            }
            std::cout << "ok !" << std::endl;

            if (isUnique)
            {
                break;
            }
        }
    }

    std::cout << "everything is good !" << std::endl;

    return 0;
}

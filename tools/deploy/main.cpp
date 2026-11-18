/*
 * Copyright 2026 Guillaume Guillet
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

/*
 * fgeDeploy : build and package an application.
 *
 *     fgeDeploy build      configure, compile and install into install/<name>
 *     fgeDeploy package    zip install/<name> into dist/
 *     fgeDeploy all        both (default)
 *
 * The two steps are separate so that the binaries can be signed in between.
 *
 * Expected from the application:
 *   - a CMakeLists.txt with "project(<name> VERSION <x.y.z>)"
 *   - install rules that put everything inside a "<name>" folder
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

#ifdef _WIN32
    #define DEPLOY_PLATFORM "windows"
#else
    #define DEPLOY_PLATFORM "linux"
#endif

//GRUpdater picks the release asset from the platform and architecture in its name
#if defined(__x86_64__) || defined(_M_X64)
    #define DEPLOY_ARCH "x64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define DEPLOY_ARCH "arm64"
#else
    #define DEPLOY_ARCH "x86"
#endif

//A tool built with MinGW is most likely used for MinGW applications
#ifdef __MINGW32__
    #define DEPLOY_DEFAULT_GENERATOR "MinGW Makefiles"
#else
    #define DEPLOY_DEFAULT_GENERATOR ""
#endif

struct Options
{
    std::string _step = "all";
    fs::path _source = ".";
    std::string _name;
    std::string _buildType = "Release";
    std::string _generator = DEPLOY_DEFAULT_GENERATOR;
    std::string _jobs;
    std::string _expectTag;
    std::string _tag;
    std::vector<std::string> _cmakeArgs;
};

struct Project
{
    std::string _name;
    std::string _version;

    fs::path _buildDir;
    fs::path _installDir;
    fs::path _appDir;
    fs::path _distDir;
};

[[noreturn]] void Fail(std::string const& message)
{
    std::cerr << "error: " << message << '\n';
    std::exit(1);
}

//Run a command and return its exit code
int Execute(std::vector<std::string> const& command)
{
    std::string line;
    for (auto const& part: command)
    {
        if (!line.empty())
        {
            line += ' ';
        }
#ifdef _WIN32
        line += '"' + part + '"';
#else
        line += '\'' + part + '\'';
#endif
    }

    std::cout << "\n> " << line << std::endl;

#ifdef _WIN32
    //cmd.exe removes the first and the last quote of the line
    line = '"' + line + '"';
#endif
    return std::system(line.c_str());
}

//Run a command and stop everything if it fails
void Run(std::vector<std::string> const& command)
{
    if (Execute(command) != 0)
    {
        Fail("command failed: " + command.front());
    }
}

//The name and the version are defined once, in the project() call of CMakeLists.txt
Project ReadProject(Options const& options)
{
    fs::path const root = fs::absolute(options._source).lexically_normal();

    std::ifstream const file{root / "CMakeLists.txt"};
    if (!file)
    {
        Fail("can't open " + (root / "CMakeLists.txt").generic_string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string const text = buffer.str();

    std::string name = options._name;
    std::string tag = options._tag;
    if (tag.empty())
    {
        std::regex const pattern{R"(project\s*\(\s*(\w+)[^)]*VERSION\s+(\d+\.\d+\.\d+))"};
        std::smatch match;
        if (!std::regex_search(text, match, pattern))
        {
            Fail("can't find project(<name> VERSION <x.y.z>) in CMakeLists.txt");
        }
        tag = match[2].str();
        name = match[1].str();
    }
    else if (name.empty())
    {
        std::regex const pattern{R"(project\s*\(\s*(\w+)[^)]*(?:VERSION\s+\d+\.\d+\.\d+)?)"};
        std::smatch match;
        if (!std::regex_search(text, match, pattern))
        {
            Fail("can't find project(<name> (VERSION <x.y.z>)?) in CMakeLists.txt");
        }
        name = match[1].str();
    }

    Project project;
    project._name = std::move(name);
    project._version = std::move(tag);
    project._buildDir = root / "build-deploy";
    project._installDir = root / "install-deploy";
    project._appDir = project._installDir / project._name;
    project._distDir = root / "dist";
    return project;
}

void CheckTag(Options const& options, Project const& project)
{
    std::string tag = options._expectTag;

    //Automatic on GitHub Actions tag builds
    char const* refType = std::getenv("GITHUB_REF_TYPE");
    char const* refName = std::getenv("GITHUB_REF_NAME");
    if (tag.empty() && refType != nullptr && refName != nullptr && std::string{refType} == "tag")
    {
        tag = refName;
    }

    if (tag.empty())
    {
        return;
    }

    std::string const expected = "v" + project._version;
    if (tag != expected)
    {
        Fail("tag is " + tag + " but CMakeLists.txt says " + expected + ", update project(... VERSION ...)");
    }
    std::cout << "Tag " << tag << " matches the project version\n";
}

#ifdef _WIN32
    #define DEPLOY_RETRIEVER_URL "https://github.com/JonathSpirit/depsRetriever.git"
    #define DEPLOY_RETRIEVER_COMMIT "43de4827a7b20b82d2ee68b80012e19128a95936"

//Clone and build depsRetriever inside the build folder, only the first time
fs::path BuildRetriever(Options const& options, Project const& project)
{
    fs::path const source = project._buildDir / "depsRetriever";
    fs::path const build = source / "build";
    fs::path const executable = build / "depsRetriever.exe";

    if (fs::exists(executable))
    {
        return executable;
    }

    if (!fs::exists(source / "CMakeLists.txt"))
    {
        Run({"git", "clone", DEPLOY_RETRIEVER_URL, source.generic_string()});
        Run({"git", "-C", source.generic_string(), "checkout", DEPLOY_RETRIEVER_COMMIT});
    }

    std::vector<std::string> configure = {
            "cmake", "-S", source.generic_string(), "-B", build.generic_string(), "-DCMAKE_BUILD_TYPE=Release"};
    if (!options._generator.empty())
    {
        configure.insert(configure.end(), {"-G", options._generator});
    }

    Run(configure);
    Run({"cmake", "--build", build.generic_string()});

    if (!fs::exists(executable))
    {
        Fail("depsRetriever was built but " + executable.generic_string() + " doesn't exist");
    }
    return executable;
}

//Copy the DLLs that the application imports next to it, fail if one can't be found
void RetrieveDlls(Options const& options, Project const& project)
{
    fs::path const retriever = BuildRetriever(options, project);

    Run({retriever.string(), "--in", project._appDir.string(), "--recursive", "--ignoreWindows", "--ignoreInvalid",
         "--copy", project._appDir.string()});
}
#else
//Bundled libraries must also find their own dependencies next to them
void SetOriginRpath(Project const& project)
{
    for (auto const& entry: fs::directory_iterator{project._appDir})
    {
        if (entry.is_symlink() || entry.path().filename().string().find(".so") == std::string::npos)
        {
            continue;
        }

        if (Execute({"patchelf", "--set-rpath", "$ORIGIN", entry.path().string()}) != 0)
        {
            std::cout << "warning: patchelf failed or is missing, bundled libraries keep their original RUNPATH\n";
            return;
        }
    }
}
#endif

void Build(Options const& options, Project const& project)
{
    CheckTag(options, project);

    //generic_string(): CMake wants forward slashes, even on Windows
    std::vector<std::string> configure = {"cmake",
                                          "-S",
                                          project._buildDir.parent_path().generic_string(),
                                          "-B",
                                          project._buildDir.generic_string(),
                                          "-DCMAKE_BUILD_TYPE=" + options._buildType,
                                          "-DCMAKE_INSTALL_PREFIX=" + project._installDir.generic_string()};
    if (!options._generator.empty())
    {
        configure.insert(configure.end(), {"-G", options._generator});
    }
    configure.insert(configure.end(), options._cmakeArgs.begin(), options._cmakeArgs.end());

    Run(configure);
    Run({"cmake", "--build", project._buildDir.generic_string(), "--config", options._buildType, "--parallel",
         options._jobs});

    fs::remove_all(project._installDir);
    Run({"cmake", "--install", project._buildDir.generic_string(), "--config", options._buildType});

    if (!fs::is_directory(project._appDir))
    {
        Fail("nothing was installed in " + project._appDir.generic_string() + ", the install rules must use a \"" +
             project._name + "\" folder");
    }

#ifdef _WIN32
    RetrieveDlls(options, project);
#else
    SetOriginRpath(project);
#endif

    std::cout << "\nInstalled in " << project._appDir.generic_string() << '\n';
}

void Package(Project const& project)
{
    if (!fs::is_directory(project._appDir))
    {
        Fail(project._appDir.generic_string() + " doesn't exist, run the build step first");
    }

    std::string const tag = "v" + project._version;
    fs::path const archive =
            project._distDir / (project._name + "-" + tag + "-" DEPLOY_PLATFORM "-" DEPLOY_ARCH ".zip");

    fs::create_directories(project._distDir);
    fs::remove(archive);

    //cmake is already required, so it's also used to create the zip
    Run({"cmake", "-E", "chdir", project._installDir.generic_string(), "cmake", "-E", "tar", "cf",
         archive.generic_string(), "--format=zip", project._name});

    //On GitHub Actions, give the release tag to the following steps
    if (char const* githubEnv = std::getenv("GITHUB_ENV"))
    {
        std::ofstream env{githubEnv, std::ios::app};
        env << "RELEASE_TAG=" << tag << '\n';
    }

    std::cout << "\nPackaged " << project._appDir.generic_string() << " into " << archive.generic_string() << " ("
              << fs::file_size(archive) / 1024 << " KiB)\n";
}

void PrintUsage()
{
    std::cout << "Build and package an application\n"
                 "\n"
                 "usage: fgeDeploy [build|package|all] [options] [-- <cmake configure arguments>]\n"
                 "\n"
                 "  --source <dir>        application root, where CMakeLists.txt is (default: .)\n"
                 "  --name <name>         application name (default: from project() in CMakeLists.txt)\n"
                 "  --build-type <type>   CMake build type (default: Release)\n"
                 "  --generator <name>    CMake generator\n"
                 "  --jobs <count>        parallel build jobs (default: all cores)\n"
                 "  --tag <tag>           git tag to use (default: v<project version>)\n"
                 "  --expect-tag <tag>    fail if this git tag isn't v<project version>\n"
                 "                        (automatic on GitHub Actions tag builds)\n"
                 "\n"
                 "example: fgeDeploy build -- -DFastEngine_DIR=C:/FastEngine/share/cmake/FastEngine\n";
}

int main(int argc, char* argv[])
{
    Options options;
    options._jobs = std::to_string(std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency());

    for (int i = 1; i < argc; ++i)
    {
        std::string const arg = argv[i];

        auto value = [&]() -> std::string {
            if (i + 1 >= argc)
            {
                Fail("missing value after " + arg);
            }
            return argv[++i];
        };

        if (arg == "build" || arg == "package" || arg == "all")
        {
            options._step = arg;
        }
        else if (arg == "--source")
        {
            options._source = value();
        }
        else if (arg == "--name")
        {
            options._name = value();
        }
        else if (arg == "--build-type")
        {
            options._buildType = value();
        }
        else if (arg == "--generator")
        {
            options._generator = value();
        }
        else if (arg == "--jobs")
        {
            options._jobs = value();
        }
        else if (arg == "--expect-tag")
        {
            options._expectTag = value();
        }
        else if (arg == "--tag")
        {
            options._tag = value();
        }
        else if (arg == "--")
        {
            options._cmakeArgs.assign(argv + i + 1, argv + argc);
            break;
        }
        else if (arg == "-h" || arg == "--help")
        {
            PrintUsage();
            return 0;
        }
        else
        {
            PrintUsage();
            Fail("unknown argument " + arg);
        }
    }

    Project const project = ReadProject(options);
    std::cout << project._name << " v" << project._version << " (" DEPLOY_PLATFORM "-" DEPLOY_ARCH ")\n";

    if (options._step == "build" || options._step == "all")
    {
        Build(options, project);
    }
    if (options._step == "package" || options._step == "all")
    {
        Package(project);
    }
    return 0;
}

# FastEngine

Copyright (C) 2023 Guillaume Guillet

<table border="0px">
<tr>
<td>
Licensed under the Apache License, Version 2.0 (the "License");
</td>
</tr>
<tr>
<td>
You may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
</td>
</tr>
</table>

## Description
![logo](https://github.com/JonathSpirit/FastEngine/blob/master/logo.png?raw=true) FastEngine is a multiplayer oriented framework-ish on top of [Vulkan](https://www.vulkan.org/) library. It brings a lot of tools and classes that can help mostly game development.

### Tools support
FastEngine must be built using a C++20 compatible compiler :
- For GNU/Linux : GCC-11 minimum
- For Windows : MINGW-W64, GCC-11 minimum
- For MAC : AppleClang 13 minimum

Unsupported compiler/tools :
- Microsoft compiler, (it should work but is not tested on)
- Ninja build system

Working/supported architecture :
- 32bits
- 64bits
- ARM

FastEngine is built under C++20 but game/software that link to this library can be C++17 minimum

### Documentation
You can find the latest Doxygen docs here : https://jonathspirit.github.io/FastEngine/

Vulkan specification here : https://registry.khronos.org/vulkan/specs/1.3-extensions/html/index.html

Note that this is a young project that need a ton of works for examples, documentations, tutorials ...

##  How to build / Build details

First you should (if not already) clone the git repository :

    git clone https://github.com/JonathSpirit/FastEngine.git
and updating submodules :

    git submodule init
    git submodule update --recursive
In order to correctly build the project, please create a **build/** folder at the *root* of the project.

### CMake arguments
You can force a 64/32 architecture type by using :

    -DFGE_ARCH=64 or 32
For ARM and others do not set this flag.
You can precise the build type of the project by using :

    -DCMAKE_BUILD_TYPE=Release or Debug
By default the project is in release mode.
You can build the Doxygen documentation by setting :

    -DFGE_BUILD_DOC=ON or OFF
By default the documentation is built if the build type is **Release**. (You will need [Doxygen](https://doxygen.nl/) in order to build the doc.)

You can build all examples by setting :

    -DFGE_BUILD_EXAMPLES=ON or OFF
By default the all examples are built inside the **all** target.

The library type flag (**BUILD_SHARED_LIBS**) must not be manually set and static build is not supported.

You can enable/disable tests by setting :

    -DFGE_BUILD_TESTS=ON or OFF
By default the tests is always built and in order to run the tests, you should use :

    ctest

### CMake targets
- **all**
This is the default target, it will build **FastEngine_test** **FastEngine** and **FastEngineServer** with all the dependencies.
- **FastEngine##&&**
This will build the client (audio, graphics, window, ...) shared library.
- **FastEngineServer##&&**
This will build the server (no audio, graphics or window dependencies) shared library.
- **install**
The install target is not supported **for now** and should not be used. In order to install the library please see go to the *Installing* section.

You can also build every examples target.

**##** is replaced with the architecture (32 or 64) and **&&** is replaced with **_d** if in debug mode or nothing otherwise.

### Windows
Please install a [MINGW-W64](https://www.mingw-w64.org/) environment like [MSYS2](https://www.msys2.org/) (recommended) and make sure that either 32bits and/or 64bits GCC is correctly installed with proper CMake package.

Then you should just do the following in a **build/** folder :

    cmake .. -G"MinGW Makefiles"
and then build :

    cmake --build .
I recommand to a add a little bit of power in the build with multicore argument :

    cmake --build . -j8

### Linux
Please install GCC-11 or higher in order to correctly compile :

    sudo apt update && sudo apt install gcc-11 g++-11
You should install Vulkan dependencies too :

    sudo apt-get install libvulkan-dev
And then use CMake :

    cmake .. -G"Unix Makefiles" -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11
    cmake --build . -j8

### Mac OS
For Mac OS you should just be able to do :

    cmake .. -G"Unix Makefiles"
    cmake --build . -j8

### Installing
The **install** target is not supported for now. but you can use *fge_install* tool project in order to create a directory that will contain all necessary elements to use FastEngine.

Once built, you have to place the executable at the root and run it. You just have to follow prompts.

The tool will check for every **build** folder and automatically get the build type (debug/release) and the architecture (64/32).
Here is some possible build folder name :
- cmake-build-debug32
- cmake-build-debug64
- cmake-build-release32
- cmake-build-release64
- build
- cmake-build
- ...

Once executed correctly, it will create a directory structure like this example :
- FastEngine_0.9.1-186-g6cb2357-dirty
  - bin32
  - bin64
  - include
  - lib32
  - lib64
  - require
  - fge_changelog.txt
  - IMAGE_LOGO_LICENSE
  - LICENSE
  - logo.png


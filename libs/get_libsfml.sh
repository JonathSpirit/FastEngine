# Bash Shell  Compile and install SFML

#!/bin/bash
dir=$(pwd)
jarg=8

echo $dir
echo $jarg

git clone --depth 1 --branch "2.5.1" https://github.com/SFML/SFML.git

cd SFML/

mkdir build
cd build/

#build debug

cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH="$dir/libsfml_d/" -DSFML_BUILD_NETWORK=False ..
cmake --build . -j$jarg

cmake --install .

#build release

cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH="$dir/libsfml/" -DSFML_BUILD_NETWORK=False ..
cmake --build . -j$jarg

cmake --install .

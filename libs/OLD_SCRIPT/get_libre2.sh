# Bash Shell  Compile and install re2

#!/bin/bash
dir=$(pwd)
jarg=8

echo $dir
echo $jarg

git clone --depth 1 --branch "2021-11-01" https://github.com/google/re2.git

cd re2/

mkdir buildFolder
cd buildFolder/

cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH="$dir/libre2/" ..
cmake --build . -j$jarg

cmake --install .

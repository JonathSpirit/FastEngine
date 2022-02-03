# Bash Shell  Compile and install fmt

#!/bin/bash
dir=$(pwd)
jarg=8

echo $dir
echo $jarg

git clone --depth 1 --branch "8.0.1" https://github.com/fmtlib/fmt.git

cd fmt/

mkdir build
cd build/

cmake -G"MinGW Makefiles" -DFMT_TEST=OFF -DCMAKE_INSTALL_PREFIX:PATH="$dir/libfmt/" ..
cmake --build . -j$jarg

cmake --install .

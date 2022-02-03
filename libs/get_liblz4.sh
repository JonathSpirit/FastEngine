# Bash Shell  Compile and install SDL

#!/bin/bash
dir=$(pwd)
jarg=8

echo $dir
echo $jarg

git clone --depth 1 --branch "v1.9.3" https://github.com/lz4/lz4.git

cd lz4/

make -j$jarg
make install prefix="" DESTDIR="$dir/liblz4"

# Bash Shell  Compile and install bzip2

#!/bin/bash
dir=$(pwd)
jarg=8

echo $dir
echo $jarg

git clone --depth 1 --branch "master" https://gitlab.com/bzip2/bzip2.git

cd bzip2/

make -j$jarg
make install PREFIX="$dir/libbzip2"

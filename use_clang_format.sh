#!/bin/bash
cd "$(dirname "$0")"

find sources/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --verbose -i

find includes/FastEngine/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --verbose -i

find includes/private/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --verbose -i

find examples/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --verbose -i

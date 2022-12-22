#!/bin/bash
cd "$(dirname "$0")"

set -e

find sources/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --Werror --ferror-limit=1 --verbose -n

find includes/FastEngine/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --Werror --ferror-limit=1 --verbose -n

find includes/private/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --Werror --ferror-limit=1 --verbose -n

find examples/ -iname *.hpp -o -iname *.cpp -o -iname *.inl |
    xargs clang-format --style=file --Werror --ferror-limit=1 --verbose -n

#!/bin/bash
cd "$(dirname "$0")"

clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./sources/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./sources/extra/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./sources/manager/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./sources/object/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./sources/private/*

clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./includes/FastEngine/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./includes/FastEngine/extra/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./includes/FastEngine/manager/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./includes/FastEngine/object/*
clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i ./includes/private/*

find examples/ -iname *.hpp -o -iname *.cpp -o -iname *.inl | xargs clang-format --style=file --Werror --ferror-limit=1 --verbose -n -i

#!/bin/bash
cd "$(dirname "$0")"

clang-format --style=file --verbose -i ./sources/*
clang-format --style=file --verbose -i ./sources/extra/*
clang-format --style=file --verbose -i ./sources/manager/*
clang-format --style=file --verbose -i ./sources/object/*
clang-format --style=file --verbose -i ./sources/private/*

clang-format --style=file --verbose -i ./includes/FastEngine/*
clang-format --style=file --verbose -i ./includes/FastEngine/extra/*
clang-format --style=file --verbose -i ./includes/FastEngine/manager/*
clang-format --style=file --verbose -i ./includes/FastEngine/object/*
clang-format --style=file --verbose -i ./includes/private/*

find examples/ -iname *.hpp -o -iname *.cpp -o -iname *.inl | xargs clang-format --style=file --verbose -i

#!/bin/bash
rm -rf build
CC=clang CXX=clang++ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
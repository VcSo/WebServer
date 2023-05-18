#!/bin/bash
if [ -d "./build" ]; then
  cd build
  rm CMakeCache.txt Makefile serverone cmake_install.cmake
  rm -rf CMakeFiles
else
  mkdir build && cp -r ./resources/ ./build
  cd build
fi
cmake ..
make -j4
./serverone
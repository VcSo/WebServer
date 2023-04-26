#!/bin/bash
rm -rf ./build
mkdir build && cp -r ./resources/ ./build
cd build
cmake ..
make -j4
./serverone
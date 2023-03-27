#!/bin/bash
mkdir build && cd build
cp -r ../resources/ ./
cmake ..
make -j4
./serverone
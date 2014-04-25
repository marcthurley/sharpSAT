#!/bin/bash

mkdir build
cd build

mkdir Release
cd Release

cmake -DCMAKE_BUILD_TYPE=Release ../..

cd ..
mkdir Debug
cd Debug

cmake -DCMAKE_BUILD_TYPE=Debug ../..

cd ..
mkdir Profiling
cd Profiling

cmake -DCMAKE_BUILD_TYPE=Profiling ../..


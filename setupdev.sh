#!/bin/bash

mkdir build
cd build

mkdir Release
cd Release

cmake -DCMAKE_BUILD_TYPE=Release ../..
# cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGMP_INCLUDE_DIR=C:\gmp\include -DGMP_LIB=C:\gmp\lib\libgmp.a -DGMPXX_LIB=C:\gmp\lib\libgmpxx.a ../..

cd ..
mkdir Debug
cd Debug

cmake -DCMAKE_BUILD_TYPE=Debug ../..
# cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGMP_INCLUDE_DIR=C:\gmp\include -DGMP_LIB=C:\gmp\lib\libgmp.a -DGMPXX_LIB=C:\gmp\lib\libgmpxx.a ../..

cd ..
mkdir Profiling
cd Profiling

cmake -DCMAKE_BUILD_TYPE=Profiling ../..
# cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Profiling -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGMP_INCLUDE_DIR=C:\gmp\include -DGMP_LIB=C:\gmp\lib\libgmp.a -DGMPXX_LIB=C:\gmp\lib\libgmpxx.a ../..

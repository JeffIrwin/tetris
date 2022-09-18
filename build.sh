#!/bin/bash

# TODO: BAT

mkdir -p build
pushd build

#cmake -G "Visual Studio 16" ..
cmake -G "Visual Studio 17" ..

cmake --build . --config Release
#cmake --build . --config Debug

popd


#!/bin/sh


#
# NOTE:
# You can probably ignore this file
# the purpose of this file is to generate binaries for the chess library for
# common platforms
#
#

rm -rf binaries
mkdir binaries



# BUILD 1: Linux x86_64 (default for me)

rm -rf build &&
cmake "src" -B build -G Ninja &&
cmake --build build &&
mv "build/libchess.so" "binaries/libchess_linux_x86_64.so"

# # BUILD 2: Windows x86_64
rm -rf build &&
x86_64-w64-mingw32-cmake "src" -B build -G Ninja &&
cmake --build build  &&
mv "build/libchess.dll" "binaries/libchess_win64.dll"

# We unfortunately cannot generate macos binaries here because
# of their strict licensing
#

rm -rf build

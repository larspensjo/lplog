#!/bin/sh -v
#
# Find all dependencies in MinGW to DLL's
#
# Download and unpack 'depends' into local folder 'depends22_x86'
# See http://www.dependencywalker.com/d
#
rm -r *.d
make clean
make -j CXXFLAGS=-O3
mkdir -p distro
depends22_x86/depends.exe -c -f:1 -ot:out.txt lplog.exe
rm -f distro/*
files=`cat out.txt | grep '^\[' | grep mingw | sed 's/^.*c://' | sed 's/\\.DLL.*/.DLL/' | sed 's/\\\\/\\//g'`
rm out.txt
cp $files distro
cp lplog.exe distro
strip distro/lplog.exe

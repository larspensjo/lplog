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
rm -rf distro/*
files=$(awk '/System Information/,/Module List/ { next;}; /^\[.*mingw/ {gsub("\\\\", "/");print $3;}' out.txt)
rm out.txt
cp $files distro
cp lplog.exe distro
cp -r gtk-themes-MinGW/* distro
strip distro/lplog.exe

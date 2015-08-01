# This is experimental support for the meson build system.
#
# Usage:
# $ mkdir Debug Release
# $ (cd Debug; meson --buildtype debug ..; mesonconf -Dcppargs="-DDEBUG"; ninja)
# $ (cd Release; meson --buildtype release ..; ninja)
#

project('lplog', 'cpp')

add_global_arguments('-std=c++11', language : 'cpp')

gtk_dep = dependency('gtk+-3.0')

src = ['Controller.cpp', 'Debug.cpp', 'Document.cpp', 'main.cpp', 'PatternTable.cpp', 'SaveFile.cpp', 'View.cpp']

executable('lplog', sources:src, dependencies : gtk_dep)

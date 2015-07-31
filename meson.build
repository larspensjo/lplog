project('lplog', 'cpp')

add_global_arguments('-std=gnu++11', language : 'cpp')

gtk_dep = dependency('gtk+-3.0')

src = ['Controller.cpp', 'Debug.cpp', 'Document.cpp', 'main.cpp', 'PatternTable.cpp', 'SaveFile.cpp', 'View.cpp']

executable('lplog', sources:src, dependencies : gtk_dep)

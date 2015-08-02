lplog - Log file viewer
=======================

A gtk based viewer for log files.
It is like "tail -F | grep pattern", but the pattern can be changed afterwards.
The purpose is to make it easy to filter for key words.
Features:
* Display text in a window, loaded from a file
* Whenever more lines are added to the file, the window will be updated
* Whenever the file is restarted, the window will be restarted, in a new tab.
* Support filter built as a tree of OR ('|'), AND ('&') and NOT ('!') nodes.
* Parts of the filter can be enabled or disabled by a click to make it easy to change
* Support pasting of clipboard or drag-and-drop into a new tab.
* Incremental search
* Optional display of line numbers

A windows executable can be found at: https://www.dropbox.com/sh/lxneh66393icwb4/7J7vhx1olq

Compiles for Windows and Linux, GTK+-2.0 and gtk+-3.0.

For Windows:
* install MinGW+MSYS
* Download the gtk3 super bundle from http://sourceforge.net/projects/gtk-mingw/
(or GTK2 from http://www.gtk.org/download/win32.php) and unpack it in /usr/local
* Make sure /local/bin is in the PATH if you use the MinGW shell
* If CodeBlocks is used, start it from the MinGW shell (to get the PATH).

Build for Linux:
* Install ```sudo apt-get install libgtk-3-dev```
* ```make```
* To create a debian install package, see "make debian" instructions in Makefile

![Pict](https://dl.dropboxusercontent.com/u/3471992/lplog1.png)

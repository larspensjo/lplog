lplog
=====

A gtk based viewer for log files.
It is like "tail -f | grep pattern", but the pattern can be changed afterwards.
The purpose is to make it easy to filter for key words.
Features:
* Display text in a window, loaded from a file
* Whenever more lines are added to the file, the window will be updated
* Whenever the file is restarted, the window will be restarted
* Support filter built as a tree of OR ('|'), AND ('&') and NOT ('!') nodes.
* Parts of the filter can be enabled or disabled by a click to make it easy to change
* Support pasting of clipboard

Compiles for Windows and Linux, GTK+-2.0 and gtk+-3.0.

For Windows:
* install MinGW+MSYS
* Download the gtk+ super bundle from http://www.gtk.org/download/win32.php and unpack it in /usr/local
* Make sure /local/bin is in the PATH if you use the MinGW shell
* If CodeBlocks is used, start it from the MinGW shell (to get the PATH).

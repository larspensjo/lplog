High priority
=============
* Enable swicthing between case sensitive and insensitive mode.
* The Replace executed from Controller after an event should restore position, depending on autoscrolling.
There should be no Replace() initiated from View.
* Use Ctrl-g to jump to a line.
* Windows header doesn't get updated sometimes.
* Create a mode which maximises the test window and hides all other widgets (using the expander widget).
The purpose is to show a read-only window to display status.
* Get page up/page down to work.
* Enable drag-and-drop to the empty document.
* Enable CTRL-TAB to switch between tabs.
* Deleting or adding filters when filtering is disable in root should not redraw screen.
* Move the pattern edit menu button to the Edit menu.

Medium priority
===============
* Color code selected strings in display window.
* Use CTRL-mouse wheel to change font size.
* Detect ASCII input and convert to UTF-8.
* Define a debug mode where log output is sent to its own tab.
* Create a snapshot function. It should freeze the current tab, and start a new one that continues on the same log file.
* Create a slider that can be used to hide initial part of the log line.
This is useful when the log lines all start with a similar pattern.
* There should be some kind of notification if a search string contains leading or trailing blanks.
This may lead to failed searches.
* The icon should be loaded from the resource file instead from the icon file.
* Busy cursor is needed for big documents.
* Use Shift-O to add another pattern above the current.
* Use '/' to start a search.

Low priority
============
* Deselected patterns should be gray (including sub tree).
* Some log files contains escape characters to get colors in terminal output. These escape characters should be removed, or used to actually create colors.
* Actually allow editing of the file.
This shall correctly identify the line that is edited, and update the original.

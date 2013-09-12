* The Replace executed from Controller after an event should restore position, depending on autoscrolling. There should be no Replace() initiated from View.
* Move Append() to after the Controller event loop, just like Replace.
* Implement File/Close to close the current tab.
* Put the search pattern in a scrolled window, to prevent it from getting too wide.
* - Put the search pattern in a window where the size can be changed by the user
* When a new tab is created from updating the input file, the view should automatically switch to this tab.
* Color code selected strings in display window.
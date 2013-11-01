#pragma once

// Copyright 2013 Lars Pensjö
//
// Lplog is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// Lplog is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Lplog.  If not, see <http://www.gnu.org/licenses/>.
//

#include <gtk/gtk.h>

class SaveFile;

class PatternTable
{
public:
	PatternTable(GtkWindow *win) : mMainWindow(win) {}
	void Display(SaveFile &);
private:
	GtkWindow *mMainWindow = 0;
	GtkTreeView *mTreeView = 0;
};

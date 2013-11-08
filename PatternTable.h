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
#include <string>
#include <vector>

class SaveFile;

class PatternTable
{
public:
	PatternTable(GtkWindow *win) : mMainWindow(win) {}
	~PatternTable();
	bool Display(SaveFile &); // Return true if a new pattern is selected

	void ExecuteCommand(const std::string &name);
private:
	GtkWindow *mMainWindow = nullptr;
	GtkTreeView *mTreeView = nullptr;
	GtkWidget *mDialog = nullptr;
	GtkTreeModel *mStore = nullptr;
	std::vector<std::string> mOriginalNameList;
	GtkTreeIter mIterFoundCurrent = { 0 };

	bool Select(GtkTreeSelection *selection, SaveFile &save);
	void UpdateList(SaveFile &);
};

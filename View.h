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

#pragma once

#include <gtk/gtk.h>
#include <string>
#include <sstream>

class Document;

class View
{
public:
	void Create(GCallback buttonCB, GCallback toggleButtonCB, GCallback keyPressed, GCallback editCell,
				GCallback togglePattern, GCallback changePage, GCallback quitCB, gpointer cbData);
	void SetWindowTitle(const std::string &);
	void Append(Document *); // Append the new lines to the end of the view
	void Replace(Document *); // Replace the lines in the view
	void ToggleLineNumbers(Document *);
	void FilterString(std::stringstream &ss, Document *doc, bool restartFirstLine);
	void About();
	GtkWidget *FileOpenDialog();
	void UpdateStatusBar(Document *doc);
	int AddTab(Document *, gpointer cbData, GCallback dragReceived, GCallback textViewkeyPress, bool switchTab = false);
	void DimCurrentTab();
	void CloseCurrentTab();
	int GetCurrentTabId() const;

	void TogglePattern(gchar *path);
	void OpenPatternForEditing();
	void DeletePattern();
	void AddPatternLine();
	void AddPatternLineIndented();
	void EditPattern(gchar *path, gchar *newString);
	bool RootPatternActive();

	int nextId = 0; // Create a new unique number for each tab. TODO: Should be private
private:
	GtkLabel *mStatusBar = 0;
	GtkWidget *mAutoScroll = 0;
	GtkWindow *mWindow = 0;
	bool mShowLineNumbers = false;
	unsigned mFoundLines = 0;
	GtkWidget *mNotebook = 0;

	GtkTreeStore *mPattern = 0;
	GtkTreeView *mTreeView = 0;
	GtkTreeIter mPatternRoot = { 0 };

	enum class Evaluation {
		Match,
		Nomatch,
		Neither,
	};
	Evaluation isShown(const std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	void AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	GtkWidget *AddMenu(GtkWidget *menubar, const gchar *label);
	bool FindSelectedPattern(GtkTreeIter *selectedPattern) const;
};

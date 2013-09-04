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
	GtkTextBuffer *Create(GtkTreeModel *model, GCallback buttonCB, GCallback toggleButtonCB, GCallback clickPatternSell,
						GCallback keyPressed, GCallback editCell, GCallback textViewkeyPress, GSourceFunc timer, GCallback togglePattern,
						gpointer cbData);
	void SetStatus(const std::string &);
	void SetWindowTitle(const std::string &);
	void Append(Document *); // Append the new lines to the end of the view
	void Replace(Document *); // Replace the lines in the view
	void ToggleLineNumbers(Document *);
	void TogglePattern(gchar *path);
	void OpenPatternForEditing(Document *);
	void EditPattern(gchar *path, gchar *newString);
	void FilterString(std::stringstream &ss);
	std::string Status() const;
private:
	GtkLabel *mStatusBar = 0;
	GtkTreeView *mTreeView = 0;
	GtkTextView *mTextView = 0;
	GtkWidget *mAutoScroll = 0;
	GtkWindow *mWindow = 0;
	GtkScrolledWindow *mScrolledView = 0;
	GtkTreeStore *mPattern = 0;
	GtkTextBuffer *mBuffer = 0;
	bool mShowLineNumbers = false;
	GtkTreeIter mSelectedPatternIter = { 0 };
	GtkTreeIter mPatternRoot = { 0 };
	unsigned mFoundLines = 0;

	enum class Evaluation {
		Match,
		Nomatch,
		Neither,
	};
	Evaluation isShown(std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	void AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	GtkWidget *AddMenu(GtkWidget *menubar, const gchar *label);
};

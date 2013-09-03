// Copyright 2013 Lars Pensj�
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

class Document;

class View
{
public:
	GtkTextBuffer *Create(GtkTreeModel *pattern, GCallback buttonCB, GCallback toggleButtonCB, GCallback clickPatternToggle, gpointer cbData);
	void SetStatus(const std::string &);
	void SetWindowTitle(const std::string &);
	void Append(Document *);
	void ToggleLineNumbers(Document *);
private:
	GtkLabel *mStatusBar = 0;
	GtkTreeView *mTreeView = 0;
	GtkTextView *mTextView = 0;
	GtkWidget *mAutoScroll = 0;
	GtkWindow *mWindow = 0;
	GtkScrolledWindow *mScrolledView = 0;

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb);
	void AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb);
	GtkWidget *AddMenu(GtkWidget *menubar, const gchar *label);
};

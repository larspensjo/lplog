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

class Document;

class View
{
public:
	void Create(Document*);
	void SetStatus(const std::string &);
	bool Update();
	void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void ToggleCell(GtkCellRendererToggle *renderer, gchar *path);
	gboolean TextViewKeyPress(guint keyval);
	void ToggleButton(const std::string &name);
private:
	GtkLabel *mStatusBar = 0;
	GtkTreeStore *mPattern = 0;
	GtkTreeIter mRoot = { 0 };
	GtkTextBuffer *mBuffer = 0;
	GtkTreeView *mTreeView = 0;
	GtkTextView *mTextView = 0;
	GtkWidget *mAutoScroll = 0;
	GtkWindow *mWindow = 0;
	GtkScrolledWindow *mScrolledView = 0;
	bool mShowLineNumbers = false;

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb);
	void AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb);
	GtkWidget *AddMenu(GtkWidget *menubar, const gchar *label);
};

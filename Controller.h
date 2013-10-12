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
#include <map>

#include "View.h"
#include "Document.h"

class View;

class Controller
{
public:
	gboolean TextViewKeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
	gboolean KeyPressedOther(GtkWidget *, GdkEvent *);
	gboolean TextViewKeyPress(guint keyval);                                 // Manage a key in our own way for the text view

	void Run(int argc, char *argv[], GdkPixbuf *icon);
	void FileOpenDialog();
	void OpenURI(const std::string &uri);
	void PatternCellUpdated(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void TogglePattern(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);                              // Click toggle button and other buttons
	void PollInput();
	void About() const { mView.About(); }
	void Help() const;
	void ChangeDoc(int id);                                                  // Change current document
	void Quit() { mQuitNow = true; }                                         // Request application to shut down
	void CloseCurrentTab();
	void InitiateFind();                                                     // User pressed Find, to search in text view
	void Find(const std::string &);
private:
	bool mValidSelectedPatternIter = false;
	View mView;
	Document *mCurrentDoc = 0;
	std::map<int, Document> mDocumentList;
	bool mQuitNow = false;
	bool mQueueReplace = false;
	bool mQueueAppend = false;
	bool mRootPatternDisabled = false;
};

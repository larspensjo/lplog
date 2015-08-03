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
#include <list>

#include "View.h"
#include "Document.h"

class SaveFile;

class Controller
{
public:
	Controller(SaveFile &savefile) : mSaveFile(savefile) {}

	gboolean TextViewKeyEvent(GdkEvent *event);
	gboolean KeyPressedOther(GtkWidget *, GdkEvent *);
	gboolean TextViewKeyPress(guint keyval);                                 // Manage a key in our own way for the text view

	void Run(int argc, char *argv[], GdkPixbuf *icon);
	void OpenURI(const std::string &uri);
	void PatternCellUpdated(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void TogglePattern(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);                              // Click toggle button and other buttons
	void PollInput();
	void ChangeDoc(int id);                                                  // Change current document
	void Quit() { mQuitNow = true; }                                         // Request application to shut down
	void Find(const std::string &);
	void ExecuteCommand(const std::string &); // String is from the button

private:
	void CloseCurrentTab();
	void FileOpenDialog();
	void Help() const;
	gboolean KeyPressed(guint keyval);
	void SaveCurrentPattern(); // Save it to mSaveFile

	bool mValidSelectedPatternIter = false;
	View mView;
	Document *mCurrentDoc = 0;
	std::map<int, Document> mDocumentList;
	bool mQuitNow = false;
	bool mQueueReplace = false;              // The input file is completely replaced, and the display need to be updated.
	bool mQueueAppend = false;               // The input file has grown, and there may be more lines that should be appended to the display
	bool mRootPatternDisabled = false;
	SaveFile &mSaveFile;
};

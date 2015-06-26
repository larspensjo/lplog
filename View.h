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
class SaveFile;

class View
{
public:
	void Create(GdkPixbuf *icon, GCallback buttonCB, GCallback toggleButtonCB, GCallback keyPressedTreeCB, GCallback keyPressOtherCB, GCallback editCell,
				GCallback togglePattern, GCallback changePage, GCallback quitCB, GCallback findCB, gpointer cbData);
	void SetWindowTitle(const std::string &);
	void Append(Document *); // Append the new lines to the end of the view
	void Replace(Document *); // Replace the lines in the view
	void ToggleLineNumbers(Document *);
	void FilterString(std::stringstream &ss, Document *doc, bool restartFirstLine);
	void About() const;
	void Help(const std::string &message) const;
	GtkWidget *FileOpenDialog();
	void UpdateStatusBar(Document *doc);
	int AddTab(Document *, gpointer cbData, GCallback dragReceived, GCallback textViewkeyPress, bool switchTab = false);
	void DimCurrentTab();
	void CloseCurrentTab();
	int GetCurrentTabId() const;
	void Serialize(std::stringstream &ss);
	void DeSerialize(SaveFile &);
	bool DisplayPatternStore(SaveFile &); // Return true if there was a change

	void ToggleIgnoreDuplicateLines() { mIgnoreDuplicateLines = !mIgnoreDuplicateLines; }

	void SetFocusFind();
	void FindNext(Document *, std::string, int direction);
	void FindSetCaseSensitive();
	const std::string GetSearchString() const;

	void TogglePattern(gchar *path);
	void OpenPatternForEditing();
	void DeletePattern();
	void AddPatternLine();
	void AddPatternLineIndented();
	void EditPattern(gchar *path, gchar *newString);
	bool RootPatternActive();

	int nextId = 0; // Create a new unique number for each tab. TODO: Should be private
private:
	GtkLabel *mStatusText = 0;
	GtkWidget *mAutoScroll = 0;
	GtkWindow *mWindow = 0;
	bool mShowLineNumbers = false;
	GtkWidget *mFindEntry = 0;
	unsigned mFoundLines = 0;
	GtkWidget *mNotebook = 0;
	bool mCaseSensitive = false;
	GtkAccelGroup *mAccelGroup = 0;
	bool mIgnoreDuplicateLines = false;

	GtkTreeStore *mPattern = 0;
	GtkTreeView *mTreeView = 0;
	GtkTreeIter mPatternRoot = { 0 };

	enum class Evaluation {
		Match,
		Nomatch,
		Neither,
	};
	// Test if a line is shown, given the specified tree.
	Evaluation isShown(const std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);
	void Serialize(std::stringstream &ss, GtkTreeModel *pattern, GtkTreeIter *iter) const;
	std::string::size_type DeSerialize(const std::string &, GtkTreeIter *parent, GtkTreeIter *node, unsigned level);

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	void AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb, gpointer cbData);
	GtkWidget *AddMenu(GtkWidget *menubar, const gchar *label);
	bool FindSelectedPattern(GtkTreeIter *selectedPattern) const;
};

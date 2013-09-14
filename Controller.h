#pragma once

#include <gtk/gtk.h>
#include <map>

#include "View.h"
#include "Document.h"

class View;

class Controller
{
public:
	void Run(int argc, char *argv[]);
	gboolean TextViewKeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
	void FileOpenDialog();
	void OpenURI(const std::string &uri);
	gboolean TextViewKeyPress(guint keyval);
	void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void TogglePattern(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);
	void PollInput();
	void About() { mView.About(); }
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

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
	gboolean TextViewKeyPress(guint keyval);

	void Run(int argc, char *argv[], GdkPixbuf *icon);
	void FileOpenDialog();
	void OpenURI(const std::string &uri);
	void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void TogglePattern(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);                              // Click toggle button and other buttons
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

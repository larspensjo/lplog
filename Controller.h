#pragma once

#include <gtk/gtk.h>

#include "View.h"
#include "Document.h"

class View;

class Controller
{
public:
	void Run(int argc, char *argv[]);
	gboolean KeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
	void FileOpenDialog();
	gboolean TextViewKeyPress(guint keyval);
	void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void TogglePattern(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);
	void PollInput();
	void About() { mView.About(); }
private:
	bool mValidSelectedPatternIter = false;
	View mView;
	Document mDoc;
};

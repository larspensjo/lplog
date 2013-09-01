#pragma once

#include <gtk/gtk.h>

#include "View.h"
#include "Document.h"

class View;

class Controller
{
public:
	void ClickCell(GtkTreeSelection *selection);
	void Run(int argc, char *argv[]);
	gboolean KeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
	void About();
	void FileOpenDialog();
	gboolean TextViewKeyPress(guint keyval);
	void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString);
	void ToggleCell(GtkCellRendererToggle *renderer, gchar *path);
	void ToggleButton(const std::string &name);
	bool Update();
private:
	bool mValidSelectedPatternIter = false;
	GtkTreeIter mSelectedPatternIter = { 0 };
	View mView;
	Document mDoc;
};

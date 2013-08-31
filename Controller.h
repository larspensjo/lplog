#pragma once

#include <gtk/gtk.h>

#include "View.h"

class View;

class Controller
{
public:
	void ClickCell(GtkTreeSelection *selection);
	void Run(int argc, char *argv[]);
	gboolean KeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
	void About();
private:
	bool mValidSelectedPatternIter = false;
	GtkTreeIter mSelectedPatternIter = { 0 };
	View mView;
};

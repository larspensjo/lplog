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
	void ClickCell(GtkTreeSelection *selection);
	gboolean KeyEvent(GdkEvent *event);
	gboolean KeyPressed(guint keyval);
private:
	GtkLabel *mStatusBar = 0;
	Document *mDoc = 0;
	GtkTreeStore *mPattern = 0;
	GtkTreeIter mSelectedPatternIter = { 0 };
	GtkTreeIter mRoot = { 0 };
	bool mValidSelectedPatternIter = false;
	GtkTextBuffer *mBuffer = 0;
	GtkTreeView *mTreeView = 0;

	void AddButton(GtkWidget *box, const gchar *label, const gchar *name);
};

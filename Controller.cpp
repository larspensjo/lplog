#include "Controller.h"
#include "Document.h"

static void ClickCell(GtkTreeView *treeView, Controller *c)
{
	c->ClickCell(gtk_tree_view_get_selection(treeView));
}

static gboolean KeyPressed(GtkTreeView *, GdkEvent *event, Controller *c) {
	return c->KeyEvent(event);
}

static void ButtonClicked(GtkButton *button, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(button));
	if (name == "quit")
		gtk_main_quit();
	else if (name == "line")
		c->KeyPressed(GDK_KEY_plus);
	else if (name == "remove")
		c->KeyPressed(GDK_KEY_Delete);
	else if (name == "child")
		c->KeyPressed(GDK_KEY_a);
	else if (name == "about")
		c->About();
	else if (name == "open")
		c->FileOpenDialog();
	else if (name == "paste")
		c->TextViewKeyPress(GDK_KEY_Paste);
	else
		cout << "Unknown button: " << name << endl;
}

static gboolean TestForeChanges(Controller *c)
{
	(void)c->Update();
	return true; // Keep timer going for ever
}

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, Controller *c)
{
	c->EditCell(renderer, path, newString);
}

gboolean Controller::KeyPressed(guint keyval) {
	if (!mValidSelectedPatternIter)
		return false;
	GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
	GtkTreeIter child;
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_F2:
		{
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &mSelectedPatternIter);
			gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	case GDK_KEY_Delete:
	case GDK_KEY_KP_Delete:
		if (IterEqual(&mRoot, &mSelectedPatternIter))
			return false;
		if (!gtk_tree_store_remove(mPattern, &mSelectedPatternIter))
			mValidSelectedPatternIter = false;
		stopEvent = true;
		break;
	case GDK_KEY_plus:
	case GDK_KEY_KP_Add:
		{
			if (IterEqual(&mRoot, &mSelectedPatternIter))
				return false;
			gtk_tree_store_insert_after(mPattern, &child, NULL, &mSelectedPatternIter);
			gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1); // Initialize new value with empty string and enabled.
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
			gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	case GDK_KEY_a:
		{
			gtk_tree_store_insert_after(mPattern, &child, &mSelectedPatternIter, NULL);
			gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1); // Initialize new value with empty string and enabled.
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
			gtk_tree_view_expand_to_path(mTreeView, path);
			gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	}
	mDoc->Replace(mBuffer, GTK_TREE_MODEL(mPattern), mShowLineNumbers);
	gtk_label_set_text(mStatusBar, mDoc->Status().c_str());
	this->ClickCell(gtk_tree_view_get_selection(mTreeView));
	return stopEvent; // Stop event from propagating
}

gboolean Controller::KeyEvent(GdkEvent *event) {
	return this->KeyPressed(event->key.keyval);
}

void Controller::ClickCell(GtkTreeSelection *selection) {
	mValidSelectedPatternIter = false;
	if (selection == 0)
		return;
	GtkTreeModel *pattern = 0;
	bool found = gtk_tree_selection_get_selected(selection, &pattern, &mSelectedPatternIter);
	if (!found)
		return;
	mValidSelectedPatternIter = true;
#ifdef DEBUG
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, &mSelectedPatternIter, 0, &val);
	const gchar *str = g_value_get_string(&val);
	if (str != nullptr) {
		cout << "ClickCell: " << str << endl;
	}
	g_value_unset(&val);
#endif
}

void Controller::Run(int argc, char *argv[]) {
	Document doc;
	if (argc > 1) {
		doc.AddSourceFile(argv [1]);
	}
	mView.Create(&doc);
	mView.Update();
	gtk_main ();
}

void Controller::About() {
	const char *license =
		"LPlog is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, version 3.\n\n"
		"LPlog is distributed in the hope that it will be useful\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n";

	const char *authors[] = {
		"Lars Pensj\303\266 <lars.pensjo@gmail.com>",
		NULL
	};

	const gchar* copyright = { "Copyright (c) Lars Pensj\303\266" };

	gtk_show_about_dialog(NULL,
		"version", "1.0",
		"website", "https://github.com/larspensjo/lplog",
		"comments", "A program to display and filter a log file.",
		"authors", authors,
		"license", license,
		"program-name", "LPlog",
		"copyright", copyright,
		NULL);
}

void Controller::FileOpenDialog() {
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Open File",
						  mWindow,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						  NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		mDoc->AddSourceFile(filename);
		gtk_window_set_title(mWindow, ("LPlog " + mDoc->FileName()).c_str());
		Update();
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

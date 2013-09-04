#include <iostream>
#include <string.h>

#include "Controller.h"
#include "Document.h"

using std::cout;
using std::endl;

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
	c->PollInput();
	return true; // Keep timer going for ever
}

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, Controller *c)
{
	c->EditCell(renderer, path, newString);
}

static gboolean TextViewKeyPress(GtkWidget *widget, GdkEvent *event, Controller *c) {
#if 0
	cout << "keyval: " << event->key.keyval;
	cout << " is_modifier: " << event->key.is_modifier;
	cout << " hardware_keycode: " << event->key.hardware_keycode;
	cout << " type: " << event->key.type;
	cout << " string: " << event->key.string;
	cout << endl;
#endif
	return c->TextViewKeyPress(event->key.keyval);
}

static void TogglePattern(GtkCellRendererToggle *renderer, gchar *path, Controller *c) {
	c->TogglePattern(renderer, path);
}

static void ToggleButton(GtkToggleButton *togglebutton, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(togglebutton));
	c->ToggleButton(name);
}

void Controller::PollInput() {
	if (mDoc.UpdateInputData()) {
		mView.Append(&mDoc);
	}
}

void Controller::TogglePattern(GtkCellRendererToggle *renderer, gchar *path) {
	mView.TogglePattern(path);
	mView.Replace(&mDoc);
}

void Controller::ToggleButton(const std::string &name) {
	if (name == "autoscroll")
		mView.UpdateStatusBar(&mDoc); // This will use the new automatic scrolling
	else if (name == "linenumbers") {
		mView.ToggleLineNumbers(&mDoc);
	} else
		cout << "Unknown toggle button: " << name << endl;
}

void Controller::EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	mView.EditPattern(path, newString);
	mView.Replace(&mDoc);
}

gboolean Controller::TextViewKeyPress(guint keyval) {
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_Paste:
		{
			GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			gchar *p = gtk_clipboard_wait_for_text(clipboard);
			if (p != nullptr) {
				// cout << "Pasted text: " << p << " key " << endl;
				unsigned size = strlen(p);
				mDoc.AddSourceText(p, size);
				g_free(p);
				mView.Replace(&mDoc);
			}
			stopEvent = true;
		}
		break;
	}
	return stopEvent; // Stop event from propagating
}

gboolean Controller::KeyPressed(guint keyval) {
	if (!mValidSelectedPatternIter)
		return false;
	GtkTreeIter child;
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_F2:
		{
			mView.OpenPatternForEditing(&mDoc);
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
			GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
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
			GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
			gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	}
	mView.Replace(&mDoc);
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
	mView.Create(::ButtonClicked, ::ToggleButton, ::ClickCell);
	mDoc.Create();
	if (argc > 1) {
		mDoc.AddSourceFile(argv [1]);
		mView.SetWindowTitle(argv[1]);
	} else
		mView.SetWindowTitle("");

	mDoc.UpdateInputData();
	mView.Replace(&mDoc);
	gtk_main ();
}

void Controller::FileOpenDialog() {
	GtkWidget *dialog = mView.FileOpenDialog();
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		mDoc.AddSourceFile(filename);
		g_free(filename);
		gtk_window_set_title(mWindow, ("LPlog " + mDoc.FileName()).c_str());
		mDoc.UpdateInputData();
		mView.Replace(&mDoc);
	}
	gtk_widget_destroy(dialog);
}

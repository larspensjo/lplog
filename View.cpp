#include <assert.h>
#include <iostream>

#include "Document.h"
#include "View.h"

using std::cout;
using std::endl;

static bool IterEqual(GtkTreeIter *a, GtkTreeIter *b) {
	// I know, the proper way is to compare the iter to a path first.
	return a->stamp == b->stamp &&
		a->user_data == b->user_data &&
		a->user_data2 == b->user_data2 &&
		a->user_data3 == b->user_data3;
}

static void ClickCell(GtkTreeView *treeView, View *view)
{
	view->ClickCell(gtk_tree_view_get_selection(treeView));
}

static gboolean KeyPressed(GtkTreeView *, GdkEvent *event, View *view) {
	return view->KeyEvent(event);
}

gboolean View::KeyEvent(GdkEvent *event) {
	return this->KeyPressed(event->key.keyval);
}

gboolean View::KeyPressed(guint keyval) {
	if (!mValidSelectedPatternIter)
		return false;
	GtkTreeIter child;
	bool stopEvent = false;
	switch(keyval) {
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
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
			gtk_tree_view_set_cursor(mTreeView, path, 0, false);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	case GDK_KEY_a:
		{
			gtk_tree_store_insert_after(mPattern, &child, &mSelectedPatternIter, NULL);
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
			gtk_tree_view_expand_to_path(mTreeView, path);
			gtk_tree_view_set_cursor(mTreeView, path, 0, false);
			gtk_tree_path_free(path);
			stopEvent = true;
		}
		break;
	}
	mDoc->Apply(mBuffer, GTK_TREE_MODEL(mPattern));
	gtk_label_set_text(mStatusBar, mDoc->Status().c_str());
	this->ClickCell(gtk_tree_view_get_selection(mTreeView));
	return stopEvent; // Stop event from propagating
}

static void ButtonClicked(GtkButton *button, View *view) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(button));
	if (name == "quit")
		exit(1);
	else if (name == "line")
		view->KeyPressed(GDK_KEY_plus);
	else if (name == "remove")
		view->KeyPressed(GDK_KEY_Delete);
	else if (name == "child")
		view->KeyPressed(GDK_KEY_a);
}

static gboolean TestForeChanges(View *view)
{
	return view->Update();
}

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, View *view)
{
	view->EditCell(renderer, path, newString);
}

void View::EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	assert(mPattern != 0);
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( mPattern ), &iter, path );
	assert(found);
	gtk_tree_store_set(mPattern, &iter, 0, newString, 1, true, -1);
	assert(mBuffer != 0);
	mDoc->Apply(mBuffer, GTK_TREE_MODEL(mPattern));
	gtk_label_set_text(mStatusBar, mDoc->Status().c_str());
}

void View::Create(Document *doc)
{
	mDoc = doc;

	GtkWidget *win = NULL;

	/* Create the main window */
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (win), 1);
	gtk_window_set_title (GTK_WINDOW (win), "LPlog");
	gtk_widget_realize (win);
	g_signal_connect (win, "destroy", gtk_main_quit, NULL);
	gtk_window_set_default_size(GTK_WINDOW (win), 800, 640);

	GtkWidget *mainbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), mainbox);

	mStatusBar = GTK_LABEL(gtk_label_new("Status"));
	gtk_box_pack_end(GTK_BOX (mainbox), GTK_WIDGET(mStatusBar), FALSE, FALSE, 0);

	/* Create a vertical box with buttons */
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	GtkWidget *buttonBox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), buttonBox, FALSE, FALSE, 0);

	AddButton(buttonBox, "_Quit", "quit");
	AddButton(buttonBox, "_Line add", "line");
	AddButton(buttonBox, "_Remove", "remove");
	AddButton(buttonBox, "_Child add", "child");

	// Create the tree model
	mPattern = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);

	// Add some test data to it
	gtk_tree_store_append(mPattern, &mRoot, NULL);
	gtk_tree_store_set(mPattern, &mRoot, 0, "&", -1);

	GtkTreeIter child;
	gtk_tree_store_insert_after(mPattern, &child, &mRoot, NULL);
	gtk_tree_store_set(mPattern, &child, 0, "lars", 1, true, -1);
	gtk_tree_store_insert_after(mPattern, &child, &mRoot, NULL);
	gtk_tree_store_set(mPattern, &child, 0, "Dis", 1, true, -1);

	// Create the tree view
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (mPattern));
	mTreeView = GTK_TREE_VIEW(tree);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree), false);
	g_signal_connect(G_OBJECT(tree), "cursor-changed", G_CALLBACK(::ClickCell), this );
	g_signal_connect(G_OBJECT(tree), "key-press-event", G_CALLBACK(::KeyPressed), this );
	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(::EditCell), this );
	auto column = gtk_tree_view_column_new_with_attributes ("Configure", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	gtk_box_pack_start(GTK_BOX(hbox), tree, FALSE, FALSE, 0);

	// Create the text display window
	auto scrollview = gtk_scrolled_window_new( NULL, NULL );
	gtk_container_set_border_width(GTK_CONTAINER(scrollview), 1);
	auto textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), false);
	gtk_widget_set_size_request(textview, 5, 5);
	mBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_container_add(GTK_CONTAINER (scrollview), textview);
	gtk_box_pack_start(GTK_BOX(hbox), scrollview, TRUE, TRUE, 0);

	mDoc->Apply(mBuffer, GTK_TREE_MODEL(mPattern));
	SetStatus(mDoc->Status());

	g_timeout_add(1000, (GSourceFunc)TestForeChanges, this);

	/* Enter the main loop */
	gtk_widget_show_all (win);
}

void View::SetStatus(const std::string &str) {
	gtk_label_set_text(mStatusBar, str.c_str());
}

bool View::Update() {
	if (mDoc->Update()) {
		mDoc->Apply(mBuffer, GTK_TREE_MODEL(mPattern));
		SetStatus(mDoc->Status());
		return true;
	}
	return false;
}

void View::ClickCell(GtkTreeSelection *selection) {
	GtkTreeModel *pattern;
	bool found = gtk_tree_selection_get_selected(selection, &pattern, &mSelectedPatternIter);
	mValidSelectedPatternIter = false;
	if (!found)
		return;
	mValidSelectedPatternIter = true;
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, &mSelectedPatternIter, 0, &val);
	const gchar *str = g_value_get_string(&val);
	if (str != nullptr) {
		cout << "ClickCell: " << str << endl;
	}
	g_value_unset(&val);
}

void View::AddButton(GtkWidget *box, const gchar *label, const gchar *name) {
	GtkWidget *button = gtk_button_new_with_mnemonic(label);
	gtk_button_set_focus_on_click(GTK_BUTTON(button), false);
	gtk_widget_set_name(GTK_WIDGET(button), name);
	g_signal_connect (button, "clicked", G_CALLBACK(ButtonClicked), this);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

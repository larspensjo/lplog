#include <stdlib.h>
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "Document.h"

// TODO: Move these into an instance class, to enable multiple files.
Document doc;
GtkTextBuffer *buffer = 0;
GtkLabel *statusBar = 0;

using std::cout;
using std::endl;

static gboolean Timeout(GtkTreeStore *pattern)
{
	if (doc.Update())
		doc.Apply(buffer, GTK_TREE_MODEL(pattern));
	return true;
}

static void editCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, GtkTreeStore *pattern)
{
	assert(pattern != 0);
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( pattern ), &iter, path );
	assert(found);
	gtk_tree_store_set(pattern, &iter, 0, newString, 1, true, -1);
	assert(buffer != 0);
	doc.Apply(buffer, GTK_TREE_MODEL(pattern));
}

static void clickCell(GtkTreeView *treeView, gpointer user_data)
{
	auto selection = gtk_tree_view_get_selection(treeView);
	GtkTreeIter iter;
	GtkTreeModel *pattern;
	bool found = gtk_tree_selection_get_selected(selection, &pattern, &iter);
	assert(found);
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, &iter, 0, &val);
	auto str = g_value_get_string(&val);
	// cout << "clickCell: " << str << endl;
	g_value_unset(&val);
}

int main (int argc, char *argv[])
{
	GtkWidget *button = NULL;
	GtkWidget *win = NULL;

	/* Initialize GTK+ */
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init (&argc, &argv);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	/* Create the main window */
	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (win), 1);
	gtk_window_set_title (GTK_WINDOW (win), "LPlog");
	gtk_widget_realize (win);
	g_signal_connect (win, "destroy", gtk_main_quit, NULL);
	gtk_window_set_default_size(GTK_WINDOW (win), 800, 640);

	auto mainbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), mainbox);

	statusBar = GTK_LABEL(gtk_label_new("Status"));
	gtk_box_pack_end(GTK_BOX (mainbox), GTK_WIDGET(statusBar), FALSE, FALSE, 0);

	/* Create a vertical box with buttons */
	auto hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect (button, "clicked", gtk_main_quit, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Create the tree model
	GtkTreeStore *treeModel = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);

	// Add some test data to it
	GtkTreeIter iter;
	gtk_tree_store_append(treeModel, &iter, NULL);
	gtk_tree_store_set(treeModel, &iter, 0, "&", -1);

	GtkTreeIter child;
	gtk_tree_store_insert_after(treeModel, &child, &iter, NULL);
	gtk_tree_store_set(treeModel, &child, 0, "lars", 1, true, -1);
	gtk_tree_store_insert_after(treeModel, &child, &iter, NULL);
	gtk_tree_store_set(treeModel, &child, 0, "Dis", 1, true, -1);

	// Create the tree view
	auto tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (treeModel));
	g_signal_connect(G_OBJECT(tree), "cursor-changed", G_CALLBACK(clickCell), treeModel );
	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(editCell), treeModel );
	auto column = gtk_tree_view_column_new_with_attributes ("Configure", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	gtk_box_pack_start(GTK_BOX(hbox), tree, FALSE, FALSE, 0);

	// Create the text display window
	auto scrollview = gtk_scrolled_window_new( NULL, NULL );
	gtk_container_set_border_width(GTK_CONTAINER(scrollview), 1);
	auto textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), false);
	gtk_widget_set_size_request(textview, 5, 5);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	if (argc > 1) {
		doc.AddSource(argv [1]);
		doc.Apply(buffer, GTK_TREE_MODEL(treeModel));
	} else {
		gtk_text_buffer_set_text(buffer, "hej hopp\nadasdasdada\nnew lines\n and\n more text", -1);
	}
	gtk_container_add(GTK_CONTAINER (scrollview), textview);
	gtk_box_pack_start(GTK_BOX(hbox), scrollview, TRUE, TRUE, 0);

	g_timeout_add(1000, (GSourceFunc)Timeout, treeModel);

	/* Enter the main loop */
	gtk_widget_show_all (win);
	gtk_main ();
	return 0;
}

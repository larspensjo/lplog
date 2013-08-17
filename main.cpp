#include <stdlib.h>
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "Filter.h"

Filter filter;
GtkTextBuffer *buffer = 0;

static void editCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, GtkTreeStore *pattern)
{
	assert(pattern != 0);
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( pattern ), &iter, path );
	assert(found);
	gtk_tree_store_set(pattern, &iter, 0, newString, 1, true, -1);
	assert(buffer != 0);
	filter.Apply(buffer, GTK_TREE_MODEL(pattern));
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

	/* Create a vertical box with buttons */
	auto hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect (button, "clicked", gtk_main_quit, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Create the tree model
	auto treeModel = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);

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
		filter.AddSource(argv [1]);
		filter.Apply(buffer, GTK_TREE_MODEL(treeModel));
	} else {
		gtk_text_buffer_set_text(buffer, "hej hopp\nadasdasdada\nnew lines\n and\n more text", -1);
	}
	gtk_container_add(GTK_CONTAINER (scrollview), textview);
	gtk_box_pack_start(GTK_BOX(hbox), scrollview, TRUE, TRUE, 0);

	/* Enter the main loop */
	gtk_widget_show_all (win);
	gtk_main ();
	return 0;
}

#include <stdlib.h>
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>

static void helloWorld (GtkWidget *wid, GtkWidget *win)
{
	GtkWidget *dialog = NULL;

	dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Hello World!");
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void LoadFile(const char *fileName, GtkTextBuffer *dest) {
	using std::ios;
	using std::cout;
	using std::endl;
	std::ifstream input(fileName);
	if (!input.is_open()) {
		std::cerr << "Failed to open " << fileName << endl;
		exit(1);
	}
	auto begin = input.tellg();
	input.seekg (0, ios::end);
	auto end = input.tellg();
	input.seekg (0, ios::beg);
	auto size = end - begin;
	char *buff = new char[size+1];
	input.read(buff, size);
	const char *last;
	bool ok = g_utf8_validate(buff, size, &last);
	if (!ok) {
		cout << "Bad character at pos " << last - buff << endl;
		size = last - buff;
	}
	buff[size] = 0;
	std::cout << "Read " << size << " characters from " << fileName << endl;
	gtk_text_buffer_set_text(dest, buff, -1);
	delete [] buff;
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
	gtk_container_set_border_width (GTK_CONTAINER (win), 8);
	gtk_window_set_title (GTK_WINDOW (win), "LPglog");
	gtk_widget_realize (win);
	g_signal_connect (win, "destroy", gtk_main_quit, NULL);
	gtk_window_set_default_size(GTK_WINDOW (win), 800, 640);

	auto mainbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), mainbox);

	/* Create a vertical box with buttons */
	auto hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (helloWorld), (gpointer) win);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect (button, "clicked", gtk_main_quit, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

	// Create the tree model
	auto treeModel = gtk_tree_store_new(1, G_TYPE_STRING);
	GtkTreeIter iter;
	gtk_tree_store_append(treeModel, &iter, 0);
	GValue tmp = G_VALUE_INIT;
	g_value_init(&tmp, G_TYPE_STRING);
	g_value_set_string(&tmp, "Search");
	gtk_tree_store_set_value(treeModel, &iter, 0, &tmp);

	gtk_tree_store_append(treeModel, &iter, 0);
	g_value_set_string(&tmp, "mark");
	gtk_tree_store_set_value(treeModel, &iter, 0, &tmp);

	// Create the tree view
	auto tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL (treeModel));
	auto renderer = gtk_cell_renderer_text_new();
	auto column = gtk_tree_view_column_new_with_attributes ("Configure", renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	gtk_box_pack_start(GTK_BOX(hbox), tree, FALSE, FALSE, 0);

	// Create the text display window
	auto scrollview = gtk_scrolled_window_new( NULL, NULL );
	auto textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), false);
	gtk_widget_set_size_request(textview, 5, 5);
	auto buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	if (argc > 1) {
		LoadFile(argv [1], buffer);
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

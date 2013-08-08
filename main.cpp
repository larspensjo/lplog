#include <stdlib.h>
#include <gtk/gtk.h>

static void helloWorld (GtkWidget *wid, GtkWidget *win)
{
	GtkWidget *dialog = NULL;

	dialog = gtk_message_dialog_new (GTK_WINDOW (win), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Hello World!");
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
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

	/* Create a vertical box with buttons */
	auto hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), hbox);

	button = gtk_button_new_from_stock (GTK_STOCK_DIALOG_INFO);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (helloWorld), (gpointer) win);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect (button, "clicked", gtk_main_quit, NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);

	auto textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), false);
	gtk_widget_set_size_request(textview, 5, 5);
	auto buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(buffer, "hej hopp\nadasdasdada\nnew lines\n and\n more text", -1);
	gtk_box_pack_start(GTK_BOX(hbox), textview, TRUE, TRUE, 0);

	/* Enter the main loop */
	gtk_widget_show_all (win);
	gtk_main ();
	return 0;
}

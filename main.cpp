#include <stdlib.h>
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "View.h"
#include "Document.h"

using std::cout;
using std::endl;

int main (int argc, char *argv[])
{
	/* Initialize GTK+ */
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, (GLogFunc) gtk_false, NULL);
	gtk_init (&argc, &argv);
	g_log_set_handler ("Gtk", G_LOG_LEVEL_WARNING, g_log_default_handler, NULL);

	View view;
	Document doc;
	if (argc > 1) {
		doc.AddSource(argv [1]);
	}
	view.Create(&doc);
	gtk_main ();
	return 0;
}

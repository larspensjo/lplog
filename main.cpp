// Copyright 2013 Lars Pensjö
//
// Lplog is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// Lplog is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Lplog.  If not, see <http://www.gnu.org/licenses/>.
//

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
	view.Update();
	gtk_main ();
	return 0;
}

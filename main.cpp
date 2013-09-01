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
#include <string>

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#endif // unix

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include "View.h"
#include "Document.h"
#include "Controller.h"

// Return the full path to the application, including the application name
static std::string getexepath() {
#ifdef __linux__
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	return std::string( result, (count > 0) ? count : 0 );
#endif // unix
#ifdef _WIN32
	char result[ PATH_MAX ];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	return std::string( result, (count > 0) ? count : 0 );
#endif // _WIN32
}

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
		doc.AddSourceFile(argv [1]);
	}
	std::string path = getexepath();
	auto pos = path.rfind('/');
	GError *err = 0;
	auto icon = gdk_pixbuf_new_from_file((path.substr(0, pos) + "/icon.bmp").c_str(), &err); // Name of file must be lplog.bmp
	if (icon != nullptr && err == 0)
		gtk_window_set_default_icon(icon);
	else
		cout << err->message << endl;
	Controller c;
	c.Run(argc, argv);
	return 0;
}

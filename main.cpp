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
static std::string GetInstallDir() {
#ifdef __linux__
	return "/usr/share/lplog/";
#endif // unix
#ifdef _WIN32
	char result[ MAX_PATH ];
	auto path = std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
	auto pos = path.rfind('\\');
	return path.substr(0,pos+1);
#endif // _WIN32
}

int main (int argc, char *argv[])
{
	g_debug("main: Argc before %d", argc);
	/* Initialize GTK+ */
	gtk_init(&argc, &argv);
	g_debug("main: Argc after %d", argc);

	GError *err = 0;
	const std::string iconFile = GetInstallDir() + "applicon.xpm";
	auto icon = gdk_pixbuf_new_from_file(iconFile.c_str(), &err); // Name of file must be lplog.bmp
	if (icon == nullptr)
		g_debug("main: Failed to load icon %s (%s)", iconFile.c_str(), err->message);
	Controller c;
	c.Run(argc, argv, icon);
	return 0;
}

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

#include "Controller.h"
#include "SaveFile.h"
#include "Debug.h"

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
	LPLOG("Argc before %d", argc);
	/* Initialize GTK+ */
	gtk_init(&argc, &argv);
	LPLOG("Argc after %d", argc);

	GError *err = 0;
	const std::string iconFile = GetInstallDir() + "lplog.ico";
	auto icon = gdk_pixbuf_new_from_file(iconFile.c_str(), &err); // Name of file must be lplog.bmp
	if (icon == nullptr)
		LPLOG("Failed to load icon %s (%s)", iconFile.c_str(), err->message);
	SaveFile saveFile("lplog");
	saveFile.Read();
	Controller(saveFile).Run(argc, argv, icon);
	saveFile.Write();
	return 0;
}

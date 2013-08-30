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

#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Document
{
public:
	void AddSource(const std::string &fileName);
	void Apply(GtkTextBuffer *dest, GtkTreeModel *pattern);
	// Update from file, return true if there were any changes.
	bool Update();
	std::string Status() const;
	const std::string &FileName() const;
private:
	enum class Evaluation {
		Match,
		Nomatch,
		Neither,
	};

	Evaluation isShown(std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);
	std::vector<std::string> mLines;
	std::string mFileName;
	std::ifstream::pos_type mCurrentPosition = 0;
	unsigned mFoundLines = 0;
};

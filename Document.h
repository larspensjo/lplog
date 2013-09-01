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
#include <sstream>

class Document
{
public:
	// Add a source file
	void AddSourceFile(const std::string &fileName);

	// Add text
	void AddSourceText(char *, unsigned size);

	// Replace the shown text with new, with the pattern applied
	void Replace(GtkTextBuffer *dest, GtkTreeModel *pattern, bool showLineNumbers);

	// Append new lines to the shown text, with the pattern applied
	void Append(GtkTextBuffer *dest, GtkTreeModel *pattern, bool showLineNumbers);

	// Update from file, return true if there were any changes.
	bool UpdateInputData();
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
	unsigned mFirstNewLine = 0; // After updating, this is the first line with new data
	void FilterString(std::stringstream &ss, GtkTextBuffer *dest, GtkTreeModel *pattern, bool showLineNumbers, unsigned firstLine);
	void SplitLines(char *, unsigned size); // This will modify the argument
	bool mStopUpdates = false;
};

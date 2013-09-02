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
	void Create(GtkTextBuffer *);
	void AddSourceFile(const std::string &fileName); // Add a source file
	void AddSourceText(char *, unsigned size); // Add text
	void Replace(); // Replace the shown text with new, with the pattern applied
	void Append(); // Append new lines to the shown text, with the pattern applied
	bool UpdateInputData(); // Update from file, return true if there were any changes.
	std::string Status() const;
	const std::string &FileName() const;
	void ToggleLineNumbers();
	void TogglePattern(gchar *path);
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
	void FilterString(std::stringstream &ss);
	void SplitLines(char *, unsigned size); // This will modify the argument
	bool mStopUpdates = false;
	std::string mIncompleteLastLine; // If the last line didn't end with a newline, stash it away for later
	GtkTextBuffer *mBuffer = 0;
	GtkTreeStore *mPattern = 0;
	GtkTreeIter mPatternRoot = { 0 };
	bool mShowLineNumbers = false;
};

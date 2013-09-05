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
#include <functional>

class Document
{
public:
	void AddSourceFile(const std::string &fileName); // Add a source file
	void AddSourceText(char *, unsigned size); // Add text
	bool UpdateInputData(); // Update from file, return true if there were any changes.
	const std::string &FileName() const;
	void IterateLines(std::function<void (const std::string&, unsigned)> f);
	unsigned GetNumLines() { return mLines.size(); }
private:
	std::vector<std::string> mLines;
	std::string mFileName;
	std::ifstream::pos_type mCurrentPosition = 0;
	unsigned mFirstNewLine = 0; // After updating, this is the first line with new data
	void SplitLines(char *, unsigned size); // This will modify the argument
	bool mStopUpdates = false;
	std::string mIncompleteLastLine; // If the last line didn't end with a newline, stash it away for later
};

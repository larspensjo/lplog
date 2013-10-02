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
#include <ctime>

// This class represents the "model" of MVC.

class Document
{
public:
	void AddSourceFile(const std::string &fileName); // Add a source file
	void AddSourceText(char *, unsigned size); // Add text
	enum class UpdateResult {
		NoChange, // The same content, no change
		Grow,     // New content added
		Replaced  // New file
	};
	UpdateResult UpdateInputData(); // Update from file
	const std::string &GetFileName() const;
	std::string GetFileNameShort() const; // Get the last part of the filename
	// Iterate a function over the lines in the input document. 'f' shall return true for lines that were added.
	void IterateLines(std::function<bool (const std::string&, unsigned)> f, bool restartFirstLine);
	unsigned GetNumLines() { return mLines.size(); }
	std::string Date() const;
	void StopUpdate();

	GtkScrolledWindow *mScrolledView = 0; // TODO: Should not be public, manage in a better way.
	GtkTextView *mTextView = 0;           // TODO: Should not be public, manage in a better way.
	unsigned mNextSearchLine = 0;          // To know where "find next" should continue
private:
	std::vector<std::string> mLines;        // The input document
	std::string mFileName;
	std::ifstream::pos_type mCurrentPosition = 0; // Position in input buffer where next read should start.
	unsigned mFirstNewLine = 0; // After updating, this is the first line with new data
	bool mStopUpdates = false;
	std::string mIncompleteLastLine; // If the last line didn't end with a newline, stash it away for later
	std::time_t mFileTime = {0};
	std::vector<unsigned> mLineMap;         // Map from printed line number to document line number
	unsigned mChecksum = 0;                 // Checksum of the current file
	unsigned mChecksumSize = 0;             // The number of initial bytes used for the checksum

	void SplitLines(char *, unsigned size); // This will modify the buffer content
	unsigned Checksum(std::ifstream &input, unsigned size);
};

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
#include <vector>
#include <string>
#include <functional>
#include <ctime>
#include <cstdio>

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
	void IterateLines(std::function<bool (std::string&, unsigned)> f, bool restartFirstLine);
	unsigned GetNumLines() { return mLines.size(); }
	std::string Date() const;
	void StopUpdate();

	GtkScrolledWindow *mScrolledView = 0; // TODO: Should not be public, manage in a better way.
	GtkTextView *mTextView = 0;           // TODO: Should not be public, manage in a better way.
	int mLastSearchLine = -1;             // To know where "find next" should continue. -1 means before first line.
	void ResetSearch() { mLastSearchLine = -1; }
private:
	std::vector<std::string> mLines;        // The input document
	std::string mFileName;
	long mCurrentPosition = 0; // Position in input buffer where next read should start.
	unsigned mFirstNewLine = 0; // After updating, this is the first line with new data
	bool mStopUpdates = false;
	std::string mIncompleteLastLine; // If the last line didn't end with a newline, stash it away for later
	std::time_t mFileTime = {0};
	long mFileSize = 0;
	std::vector<unsigned> mLineMap;         // Map from printed line number to document line number
	void SplitLines(char *, unsigned size); // This will modify the buffer content
	void RemoveColorEscapeSequences();

	static const unsigned cTestSize = 4*1024; // Small enough to be quick to read, big enough to consistently detect changed file content
	char mTestBuffer[cTestSize];
	unsigned mTestBufferCurrentSize = 0;
	void CopyToTestBuffer(std::FILE *input, unsigned size);
	bool EqualToTestBuffer(std::FILE *input, unsigned size);

	enum class InputType {
		Ascii,
		UTF16LittleEndian,
		UTF16BigEndian,
	};
	InputType mInputType = InputType::Ascii;
	void DetectFileType(const unsigned char *, unsigned size);
};

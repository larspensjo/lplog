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
#undef __STRICT_ANSI__ // Needed for "struct stat" in MinGW.

#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Document.h"

using std::ios;
using std::cout;
using std::endl;

static bool findNL(const char *source, unsigned &length, const char *&next) {
	const char *p = source;
	for (; *p != 0; ++p) {
		if (p[0] == '\r' && p[1] == '\n') {
			// Windows format
			next = p+2;
			length = p-source;
			return true;
		}
		if (p[0] == '\n' && p[1] == '\r') {
			// Mac format
			next = p+2;
			length = p-source;
			return true;
		}
		if (p[0] == '\n') {
			// Unix format
			next = p+1;
			length = p-source;
			return true;
		}
	}
	if (p == source)
		return false;
	// Just a 0-byte terminator
	next = p; // Will fail next time
	length = p-source;
	return true;
}

std::string Document::Date() const {
    int ret;
    char buf[100];

	std::tm *tm = std::localtime(&mFileTime);
    ret = std::strftime(buf, sizeof buf, "%c", tm);
    if (ret == 0)
        return "";

    return buf;
}

void Document::AddSourceFile(const std::string &fileName) {
	mStopUpdates = false;
	mFileName = fileName;
	mCurrentPosition = 0;
	mLines.clear();
	struct stat st;
	if (stat(fileName.c_str(), &st) == 0) {
		mFileTime = st.st_ctime;
	} else {
		mFileTime = 0;
	}
}

void Document::AddSourceText(char *text, unsigned size) {
	mStopUpdates = true;
	mFileName = "[Paste]";
	mCurrentPosition = 0;
	mLines.clear();
	this->SplitLines(text, size);
	std::localtime(&mFileTime);
}

bool Document::UpdateInputData() {
	if (mFileName == "" || mStopUpdates)
		return false;
	std::ifstream input(mFileName);
	if (!input.is_open()) {
		// There is no file to open
		if (mCurrentPosition != 0) {
			// There was a file last time we tried
			mCurrentPosition = 0;
			mLines.clear();
			return true;
		}
		return false;
	}
	std::ifstream::pos_type startPos = mCurrentPosition;
	input.seekg (0, ios::end);
	std::ifstream::pos_type end = input.tellg();
	if (end == mCurrentPosition)
		return false; // No change.
	mCurrentPosition = end;
	if (mCurrentPosition <= startPos) {
		// There is a new file
		startPos = 0;
		mLines.clear();
	}
	mFirstNewLine = mLines.size(); // Remember where the new lines started after the update
	auto size = mCurrentPosition - startPos;
	input.seekg (startPos, ios::beg);
	char *buff = new char[size+1];
	input.read(buff, size);
	this->SplitLines(buff, size);
	delete [] buff;
	return true;
}

void Document::IterateLines(std::function<void (const std::string&, unsigned)> f) {
	for (unsigned line = mFirstNewLine; line < mLines.size(); line++) {
		f(mLines[line], line);
	}
}

void Document::SplitLines(char *buff, unsigned size) {
	const char *last;
	while(!g_utf8_validate(buff, size, &last)) {
		unsigned pos = last - buff;
		// cout << "Bad character at pos " << pos << endl;
		buff[pos] = ' ';
	}
	buff[size] = 0;
	// Split the source into list of lines
	for (const char *p=buff;;) {
		unsigned len;
		const char *next;
		if (!findNL(p, len, next))
			break;
		if (p[len] == '\0') {
			// No newline, means the line is incomplete.
			mIncompleteLastLine += std::string(p, len);
			// cout << "Incomplete last line: '" << mIncompleteLastLine << "'" << endl;
			break;
		}
		// Add a new line
		mLines.push_back(mIncompleteLastLine + std::string(p, len));
		mIncompleteLastLine = "";
		p = next;
	}
}

std::string Document::GetFileNameShort() const {
	std::string::size_type pos = 0;
	auto pos1 = mFileName.rfind('/');
	if (pos1 != mFileName.npos)
		pos = pos1+1;
	pos1 = mFileName.rfind('\\');
	if (pos1 != mFileName.npos && pos1 > pos)
		pos = pos1+1;
	return mFileName.substr(pos);
}

const std::string &Document::GetFileName() const {
	return mFileName;
}

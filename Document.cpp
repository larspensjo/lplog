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
#include <algorithm>
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

static bool findNL(const char *source, unsigned *length, const char **next) {
	const char *p = source;
	for (; *p != 0; ++p) {
		if (p[0] == '\r' && p[1] == '\n') {
			// Windows format
			*next = p+2;
			*length = p-source;
			return true;
		}
		if (p[0] == '\n' && p[1] == '\r') {
			// Mac format
			*next = p+2;
			*length = p-source;
			return true;
		}
		if (p[0] == '\n') {
			// Unix format
			*next = p+1;
			*length = p-source;
			return true;
		}
	}
	if (p == source)
		return false;
	// Just a 0-byte terminator
	*next = p; // Will fail next time
	*length = p-source;
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

void Document::StopUpdate() {
	mStopUpdates = true;
}

void Document::AddSourceFile(const std::string &fileName) {
	mFileName = fileName;
#ifdef _WIN32
	// Change name path /C:/xx into C:/xx, but do not change //server/xx
	if (mFileName[0] == '/' && mFileName[1] != '/')
		mFileName = mFileName.substr(1);
#endif
	mStopUpdates = false;
	mCurrentPosition = 0;
	mLines.clear();
	struct stat st = { 0 };
	if (stat(mFileName.c_str(), &st) == 0) {
		g_debug("Document::AddSourceFile %s, size %u", mFileName.c_str(), (unsigned)st.st_size);
	} else {
		g_debug("Document::AddSourceFile failed to open '%s' (err %d)", mFileName.c_str(), errno);
	}
}

void Document::AddSourceText(char *text, unsigned size) {
	mStopUpdates = true;
	mFileName = "[Paste]";
	mCurrentPosition = 0;
	mLines.clear();
	this->SplitLines(text, size);
	g_debug("Document::AddSourceText %d characters %u lines", size, (unsigned)mLines.size());
	mFileTime = std::time(nullptr);
}

unsigned Document::Checksum(std::ifstream &input, unsigned size) {
	g_assert(input.is_open());
	g_assert(size > 0);
	char buff[size];
	input.read(buff, size);
	unsigned count = input.gcount(); // On Windows, the actual size will not be the same
	unsigned sum = 0;
	for (unsigned i = 0; i<count; i++) {
		// Just something simple
		sum = (sum << 2) + sum + buff[i];
	}
	return sum;
}

Document::UpdateResult Document::UpdateInputData() {
	mFirstNewLine = mLines.size(); // Remember where the new lines started after the update
	if (mFileName == "" || mStopUpdates)
		return UpdateResult::NoChange;

	// Update the time stamp of the file to latest
	struct stat st = { 0 };
	bool statFailed = stat(mFileName.c_str(), &st) != 0;

	std::ifstream input(mFileName);
	if (statFailed || !input.is_open()) {
		// There is no file
		if (mCurrentPosition != 0) {
			g_debug("Document::UpdateInputData file removed");
			mStopUpdates = true;
			return UpdateResult::Replaced;
		}
		return UpdateResult::NoChange;
	}

	bool documentIsModified = (st.st_mtime != mFileTime);
	mFileTime = st.st_mtime;

	if (mCurrentPosition > st.st_size) {
		// There is a new file
		g_debug("Document::UpdateInputData new content");
		mStopUpdates = true;
		return UpdateResult::Replaced;
	}

	if (mCurrentPosition == st.st_size)
		return UpdateResult::NoChange;

	unsigned requestedChecksumSize = 1024; // Small enough to be quick to read, big enough to consistently detect changed file content
	if (documentIsModified && mChecksumSize < requestedChecksumSize) {
		mChecksumSize = std::min(off_t(requestedChecksumSize), st.st_size);
		mChecksum = Checksum(input, mChecksumSize);
		g_debug("Document::UpdateInputData Checksum %04X, size %u", mChecksum, mChecksumSize);
	}
	auto size = st.st_size - mCurrentPosition;
	char *buff = new char[size+1]; // Reserve space for null byte
	input.seekg(mCurrentPosition, ios::beg);
	input.read(buff, size);
	g_debug("Document::UpdateInputData start %u size %u, got %u", (unsigned)mCurrentPosition, (unsigned)size, (unsigned)input.gcount());
	mCurrentPosition += input.gcount();
	// On MinGW, the actual number of characters will be smaller as CRNL is converted to NL.
	this->SplitLines(buff, input.gcount());
	delete [] buff;
	return UpdateResult::Grow;
}

void Document::IterateLines(std::function<bool (const std::string&, unsigned)> f, bool restartFirstLine) {
	if (restartFirstLine) {
		mFirstNewLine = 0;
		mLineMap.clear();
	}
	g_debug("Document::IterateLines from line %u restart '%s' printed line# %u", mFirstNewLine, restartFirstLine?"true":"false", (unsigned)mLineMap.size());
	for (unsigned line = mFirstNewLine; line < mLines.size(); line++) {
		bool accepted = f(mLines[line], line);
		if (accepted) {
			mLineMap.push_back(line);
		}
	}
}

void Document::SplitLines(char *buff, unsigned size) {
	const char *last;
	while(!g_utf8_validate(buff, size, &last)) {
		// TODO: Convert from ASCII to utf-8 instead
		unsigned pos = last - buff;
		// cout << "Bad character at pos " << pos << endl;
		buff[pos] = ' ';
	}
	buff[size] = 0;
	// Split the source into list of lines
	for (const char *p=buff;;) {
		unsigned len;
		const char *next;
		if (!findNL(p, &len, &next))
			break;
		if (p[len] == '\0') {
			// No newline, means the line is incomplete.
			mIncompleteLastLine += std::string(p, len);
			g_debug("Document::SplitLines incomplete last line (%u chars) '%s'", len, mIncompleteLastLine.c_str());
			break;
		}
		// Add a new line
		mLines.push_back(mIncompleteLastLine + std::string(p, len));
		if (mIncompleteLastLine != "")
			g_debug("Document::SplitLines merged incomplete last line '%s'", (mIncompleteLastLine + std::string(p, len)).c_str());
		mIncompleteLastLine = "";
		p = next;
	}
	g_debug("Document::SplitLines total %u,%s document %p", (unsigned)mLines.size(), mIncompleteLastLine != "" ? " incomplete last, " : "", this);
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

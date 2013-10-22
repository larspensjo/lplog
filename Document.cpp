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

#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Document.h"
#include "Defer.h"

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
	DetectFileType((const unsigned char *)text, size);
	this->SplitLines(text, size);
	g_debug("Document::AddSourceText %d characters %u lines", size, (unsigned)mLines.size());
	mFileTime = std::time(nullptr);
}

void Document::CopyToTestBuffer(std::FILE *input, unsigned size) {
	std::fseek(input, 0, SEEK_SET);
	mTestBufferCurrentSize = std::min(unsigned(sizeof mTestBuffer), size);
	unsigned count = std::fread(mTestBuffer, 1, mTestBufferCurrentSize, input);
	g_assert(count == mTestBufferCurrentSize);
}

bool Document::EqualToTestBuffer(std::FILE *input, unsigned size) {
	std::fseek(input, 0, SEEK_SET);
	if (size > mTestBufferCurrentSize)
		size = mTestBufferCurrentSize;
	char localBuffer[size];
	unsigned count = std::fread(localBuffer, 1, size, input);
	g_assert(count == size);
	return strncmp(localBuffer, mTestBuffer, size) == 0;
}

void Document::DetectFileType(const unsigned char *p, unsigned size) {
	if (size >= 4 && p[0] == 0xff && p[1] == 0xfe && p[3] == 0) {
		mInputType = InputType::UTF16LittleEndian;
		g_debug("Document::DetectFileType UTF-16 little endian");
		return;
	}
	if (size >= 4 && p[0] == 0xfe && p[1] == 0xff && p[2] == 0) {
		mInputType = InputType::UTF16BigEndian;
		g_debug("Document::DetectFileType UTF-16 big endian");
		return;
	}
	mInputType = InputType::Ascii;
	g_debug("Document::DetectFileType ASCII");
}

Document::UpdateResult Document::UpdateInputData() {
	mFirstNewLine = mLines.size(); // Remember where the new lines started after the update
	if (mFileName == "" || mStopUpdates)
		return UpdateResult::NoChange;

	// Update the time stamp of the file to latest
	struct stat st = { 0 };
	bool statFailed = stat(mFileName.c_str(), &st) != 0;

	std::FILE *input = std::fopen(mFileName.c_str(), "rb");
	Defer close([input]() { if (input != nullptr) std::fclose(input);});
	if (statFailed || input == nullptr) {
		// There is no file
		if (mCurrentPosition != 0) {
			g_debug("Document::UpdateInputData file removed");
			mStopUpdates = true;
			return UpdateResult::Replaced;
		}
		return UpdateResult::NoChange;
	}

	bool firstTime = (mFileTime == 0);
	bool documentIsModified = (st.st_mtime != mFileTime);
	if (!documentIsModified) {
		// g_debug("Document::UpdateResult not modified"); // Too verbose for normal debugging
		return UpdateResult::NoChange; // The usual case for a document that wasn't changed
	}
	mFileTime = st.st_mtime;

	if (!firstTime && documentIsModified && (st.st_size < mCurrentPosition || !EqualToTestBuffer(input, st.st_size))) {
		// There is a replaced file
		g_debug("Document::UpdateInputData new content");
		mStopUpdates = true;
		return UpdateResult::Replaced;
	}

	if (mCurrentPosition == st.st_size) {
		g_debug("Document::UpdateResult same size [%s] [%s]", firstTime?"first":"notfirst",
			documentIsModified?"modifed":"notmodifed");
		return UpdateResult::NoChange;
	}

	if (documentIsModified && mTestBufferCurrentSize < sizeof mTestBuffer) {
		CopyToTestBuffer(input, st.st_size);
		DetectFileType((const unsigned char *)mTestBuffer, mTestBufferCurrentSize);
	}
	auto addedSize = st.st_size - mCurrentPosition;
	char *buff = new char[addedSize+1]; // Reserve space for null byte. Heap allocation needed, as it may be too big for stack.
	Defer b([buff](){ delete[]buff;});
	std::fseek(input, mCurrentPosition, SEEK_SET);
	unsigned n = (unsigned)std::fread(buff, 1, addedSize, input);
	g_debug("Document::UpdateInputData start %u size %u, got %u", (unsigned)mCurrentPosition, (unsigned)addedSize, n);
	mCurrentPosition += n;
	this->SplitLines(buff, n);
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
	unsigned pos = 0;
	unsigned numBad = 0;
	Defer freeTmp; // Default, nothing done
	if (mInputType == InputType::UTF16BigEndian || mInputType == InputType::UTF16LittleEndian) {
		long numWritten;
		char *ret = g_utf16_to_utf8((const gunichar2 *)buff, size, NULL, &numWritten, NULL);
		if (ret == nullptr) {
			g_debug("Document::SplitLines failed conversion");
		} else {
			freeTmp = [ret]() { g_free(ret); };
			g_debug("Document::SplitLines parsed %ld chars from %d",numWritten, size);
			buff = ret; // Use this buffer instead
			size = numWritten;
		}
	} else {
		for(char *p = buff; !g_utf8_validate(p, size - pos, &last); p += pos) {
			// TODO: Convert from ASCII to utf-8 instead
			unsigned pos = last - buff;
			if (buff[pos] == 0) {
				g_debug("Document::SplitLines premature zero byte at pos %d", pos);
				size = pos;
				break;
			}
			buff[pos] = ' ';
			numBad++;
		}
		if (numBad > 0)
			g_debug("Document::SplitLines %d bad characters", numBad);
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

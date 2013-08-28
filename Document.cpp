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

#include <string.h>
#include <sstream>

#include "Document.h"

using std::ios;
using std::cout;
using std::endl;

bool findNL(const char *source, unsigned &length, const char *&next) {
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

void Document::AddSource(const std::string &fileName) {
	mFileName = fileName;
	mCurrentPosition = 0;
	mLines.clear();
}

bool Document::Update() {
	if (mFileName == "")
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
	auto size = mCurrentPosition - startPos;
	input.seekg (startPos, ios::beg);
	char *buff = new char[size+1];
	input.read(buff, size);
	const char *last;
	while(!g_utf8_validate(buff, size, &last)) {
		unsigned pos = last - buff;
		cout << "Bad character at pos " << pos << endl;
		buff[pos] = ' ';
	}
	buff[size] = 0;
	// std::cout << "Read " << size << " characters from " << mFileName << endl;

	// auto oldSize = mLines.size();
	// Split the source into list of lines
	for (const char *p=buff;;) {
		unsigned len;
		const char *next;
		if (!findNL(p, len, next))
			break;
		mLines.push_back(std::string(p, len));
		p = next;
	}
	delete [] buff;
	// cout << "Added " << mLines.size() - oldSize << " lines." << endl;
	return true;
}

void Document::Apply(GtkTextBuffer *dest, GtkTreeModel *pattern) {
	GtkTreeIter iter;
	bool empty = !gtk_tree_model_get_iter_first(pattern, &iter);
	std::string result;
	mFoundLines = 0;
	for (auto it = mLines.begin(); it != mLines.end(); ++it) {
		if (empty || isShown(*it, pattern, &iter)) {
			result += *it + '\n';
			++mFoundLines;
		}
	}
	gtk_text_buffer_set_text(dest, result.c_str(), -1);
}

bool Document::isShown(std::string &line, GtkTreeModel *pattern, GtkTreeIter *iter) {
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, iter, 1, &val);
	bool active = g_value_get_boolean(&val);
	g_value_unset(&val);
	if (!active)
		return true;
	gtk_tree_model_get_value(pattern, iter, 0, &val);
	bool ret = false;
	const gchar *str = g_value_get_string(&val);
	GtkTreeIter child;
	bool childFound = gtk_tree_model_iter_children(pattern, &child, iter);
	if (str == 0) {
		ret = true;
	} else if (strcmp(str, "|") == 0 && childFound) {
		ret = false;
		do {
			if (isShown(line, pattern, &child)) {
				ret = true;
				break;
			}
			childFound = gtk_tree_model_iter_next(pattern, &child);
		} while (childFound);
	} else if (strcmp(str, "&") == 0 && childFound) {
		ret = true;
		do {
			if (!isShown(line, pattern, &child)) {
				ret = false;
				break;
			}
			childFound = gtk_tree_model_iter_next(pattern, &child);
		} while (childFound);
	} else if (strcmp(str, "!") == 0 && childFound) {
		ret = !isShown(line, pattern, &child);
	} else {
		auto pos = line.find(str);
		if (pos != std::string::npos)
			ret = true;
	}
	g_value_unset(&val);
	return ret;
}

std::string Document::Status() const {
	std::stringstream ss;
	ss << mFileName << ": " << mFoundLines << " (" << mLines.size() << ")";
	return ss.str();
}

const std::string &Document::FileName() const {
	return mFileName;
}

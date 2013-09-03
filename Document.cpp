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
#include <sstream>

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

void Document::AddSourceFile(const std::string &fileName) {
	mStopUpdates = false;
	mFileName = fileName;
	mCurrentPosition = 0;
	mLines.clear();
}

void Document::AddSourceText(char *text, unsigned size) {
	mStopUpdates = true;
	mFileName = "Paste data";
	mCurrentPosition = 0;
	mLines.clear();
	this->SplitLines(text, size);
}

void Document::EditPattern(gchar *path, gchar *newString) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( mPattern ), &iter, path );
	g_assert(found);
	gtk_tree_store_set(mPattern, &iter, 0, newString, -1);
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

void Document::ToggleLineNumbers() {
	mShowLineNumbers = !mShowLineNumbers;
}

void Document::TogglePattern(gchar *path) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( mPattern ), &iter, path );
	g_assert(found);
	GValue val = { 0 };
	gtk_tree_model_get_value(GTK_TREE_MODEL(mPattern), &iter, 1, &val);
	bool current = g_value_get_boolean(&val);
	gtk_tree_store_set(mPattern, &iter, 1, !current, -1);
	g_value_unset(&val);
}

void Document::FilterString(std::stringstream &ss) {
	GtkTreeIter iter;
	bool empty = !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mPattern), &iter);
	g_assert(!empty);
	std::string separator = ""; // Start empty
	if (mFoundLines > 0)
		separator = '\n';
	// Add the lines to ss, one at a time. The last line shall not have a newline.
	for (unsigned line = mFirstNewLine; line < mLines.size(); line++) {
		if (isShown(mLines[line], GTK_TREE_MODEL(mPattern), &iter) != Evaluation::Nomatch) {
			ss << separator;
			if (mShowLineNumbers) {
				ss.width(5);
				ss.setf(ss.left);
				ss << line+1 << " ";
			}
			ss << mLines[line];
			separator = "\n";
			++mFoundLines;
		}
	}
}

void Document::Replace() {
	mFoundLines = 0;
	std::stringstream ss;
	this->FilterString(ss);
	gtk_text_buffer_set_text(mBuffer, ss.str().c_str(), -1);
}

void Document::Append() {
	std::stringstream ss;
	this->FilterString(ss);
	GtkTextIter last;
	gtk_text_buffer_get_end_iter(mBuffer, &last);
	gtk_text_buffer_insert(mBuffer, &last, ss.str().c_str(), -1);
}

Document::Evaluation Document::isShown(std::string &line, GtkTreeModel *pattern, GtkTreeIter *iter) {
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, iter, 1, &val);
	bool active = g_value_get_boolean(&val);
	g_value_unset(&val);
	if (!active)
		return Evaluation::Neither;
	gtk_tree_model_get_value(pattern, iter, 0, &val);
	Evaluation ret = Evaluation::Neither; // Use this as default
	const gchar *str = g_value_get_string(&val);
	GtkTreeIter child;
	bool childFound = gtk_tree_model_iter_children(pattern, &child, iter);
	if (str == 0) {
	} else if (strcmp(str, "|") == 0 && childFound) {
		do {
			auto current = isShown(line, pattern, &child);
			if (current == Evaluation::Match) {
				ret = Evaluation::Match;
				break;
			} else if (current == Evaluation::Nomatch) {
				// At least one Nomatch found, continue looking for Match.
				ret = Evaluation::Nomatch;
			}
			childFound = gtk_tree_model_iter_next(pattern, &child);
		} while (childFound);
	} else if (strcmp(str, "&") == 0 && childFound) {
		do {
			auto current = isShown(line, pattern, &child);
			if (current == Evaluation::Nomatch) {
				ret = Evaluation::Nomatch;
				break;
			} else if (current == Evaluation::Match) {
				// At least one Match found, continue looking for Nomatch.
				ret = Evaluation::Match;
			}
			childFound = gtk_tree_model_iter_next(pattern, &child);
		} while (childFound);
	} else if (strcmp(str, "!") == 0 && childFound) {
		switch (isShown(line, pattern, &child)) {
		case Evaluation::Match:
			ret = Evaluation::Nomatch;
			break;
		case Evaluation::Neither:
			ret = Evaluation::Neither;
			break;
		case Evaluation::Nomatch:
			ret = Evaluation::Match;
			break;
		}
	} else {
		auto pos = line.find(str);
		if (pos != std::string::npos)
			ret = Evaluation::Match;
		else
			ret = Evaluation::Nomatch;
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

void Document::Create(GtkTextBuffer *buffer) {
	mBuffer = buffer;
	// Create the tree model
	mPattern = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);

	// Add some test data to it
	gtk_tree_store_append(mPattern, &mPatternRoot, NULL);
	gtk_tree_store_set(mPattern, &mPatternRoot, 0, "|", 1, true, -1);

	GtkTreeIter child;
	gtk_tree_store_insert_after(mPattern, &child, &mPatternRoot, NULL);
	gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1);
}

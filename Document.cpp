#include <string.h>

#include "Document.h"

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

void Document::AddSource(const char *fileName) {
	using std::ios;
	using std::cout;
	using std::endl;
	std::ifstream input(fileName);
	if (!input.is_open()) {
		std::cerr << "Failed to open " << fileName << endl;
		exit(1);
	}
	auto begin = input.tellg();
	input.seekg (0, ios::end);
	auto end = input.tellg();
	input.seekg (0, ios::beg);
	auto size = end - begin;
	char *buff = new char[size+1];
	input.read(buff, size);
	const char *last;
	bool ok = g_utf8_validate(buff, size, &last);
	if (!ok) {
		cout << "Bad character at pos " << last - buff << endl;
		size = last - buff;
	}
	buff[size] = 0;
	std::cout << "Read " << size << " characters from " << fileName << endl;

	// Split the source into list of lines
	for (const char *p=buff;;) {
		unsigned len;
		const char *next;
		if (!findNL(p, len, next))
			break;
		mLines.push_back(std::string(p, len));
		p = next;
	}
	cout << "Parsed " << mLines.size() << " lines." << endl;
	delete [] buff;
}

void Document::Apply(GtkTextBuffer *dest, GtkTreeModel *pattern) {
	GtkTreeIter iter;
	bool empty = !gtk_tree_model_get_iter_first(pattern, &iter);
	std::string result;
	for (auto it = mLines.begin(); it != mLines.end(); ++it) {
		if (empty || isShown(*it, pattern, &iter))
			result += *it + '\n';
	}
	gtk_text_buffer_set_text(dest, result.c_str(), -1);
}

bool Document::isShown(std::string &line, GtkTreeModel *pattern, GtkTreeIter *iter) {
	GValue val = G_VALUE_INIT;
	gtk_tree_model_get_value(pattern, iter, 0, &val);
	bool ret = false;
	auto str = g_value_get_string(&val);
	GtkTreeIter child;
	bool childFound = gtk_tree_model_iter_children(pattern, &child, iter);
	if (strcmp(str, "|") == 0 && childFound) {
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

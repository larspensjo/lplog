#include "Filter.h"

Filter::Filter() : mSource(0)
{
	//ctor
}

Filter::~Filter()
{
	delete[] mSource;
}

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

void Filter::AddSource(const char *fileName) {
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
	delete [] mSource;
	mSource = buff;

	// Split the source into list of lines
	for (const char *p=mSource;;) {
		unsigned len;
		const char *next;
		if (!findNL(p, len, next))
			break;
		std::string line(p, len);
		mLines.push_back(line);
		p = next;
	}
	cout << "Parsed " << mLines.size() << " lines." << endl;
}

void Filter::Apply(GtkTextBuffer *dest) {
	gtk_text_buffer_set_text(dest, mSource, -1);
}

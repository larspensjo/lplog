#include "Filter.h"

Filter::Filter() : mSource(0)
{
	//ctor
}

Filter::~Filter()
{
	delete[] mSource;
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
}

void Filter::Apply(GtkTextBuffer *dest) {
	gtk_text_buffer_set_text(dest, mSource, -1);
}

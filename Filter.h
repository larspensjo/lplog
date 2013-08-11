#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Filter
{
public:
	Filter();
	~Filter();
	void AddSource(const char *fileName);
	void Apply(GtkTextBuffer *dest);
protected:
private:
	const char *mSource;
	std::vector<std::string> mLines;
};

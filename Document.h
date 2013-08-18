#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Document
{
public:
	void AddSource(const char *fileName);
	void Apply(GtkTextBuffer *dest, GtkTreeModel *pattern);
private:
	bool isShown(std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);
	std::vector<std::string> mLines;
};

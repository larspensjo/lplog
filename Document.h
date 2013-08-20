#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

class Document
{
public:
	void AddSource(const std::string &fileName);
	void Apply(GtkTextBuffer *dest, GtkTreeModel *pattern);
	// Update from file, return true if there were any changes.
	bool Update();
private:
	bool isShown(std::string &, GtkTreeModel *pattern, GtkTreeIter *iter);
	std::vector<std::string> mLines;
	std::string mFileName;
	std::ifstream::pos_type mCurrentPosition = 0;
};

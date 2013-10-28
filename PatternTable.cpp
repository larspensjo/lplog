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
#include <string>
#include <vector>

#include "PatternTable.h"
#include "SaveFile.h"

using std::string;

void PatternTable::Display(SaveFile &save) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Pattern selection", mMainWindow,
										GTK_DIALOG_MODAL,
										GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
										GTK_STOCK_CANCEL, GTK_STOCK_CANCEL,
										NULL);
	g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER (content_area), mainbox);

	// Create a list store, and populate it
	// ====================================
	GtkListStore *store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	std::vector<string> originalNameList;
	auto f = [store, &originalNameList](const string &name, const string &pattern) {
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, name.c_str(), 1, pattern.c_str(), -1);
		originalNameList.push_back(name);
	};
	save.IteratePatterns(f);

	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_end(GTK_BOX(mainbox), tree, FALSE, FALSE, 0);

	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	auto column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 0, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Pattern", renderer, "text", 1, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(response) {
	case GTK_RESPONSE_NONE:
		break;
	case GTK_RESPONSE_OK: {
		for (auto &name : originalNameList)
			save.SetPattern(name, ""); // Clear entry
		break;
	}
	case GTK_RESPONSE_CANCEL:
		break;
	}
}

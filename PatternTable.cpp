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
#include "Defer.h"

using std::string;

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, GtkListStore *store) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &iter, path);
	g_assert(found);
	gtk_list_store_set(store, &iter, 0, newString, -1);
}

enum { BUTTON_USE };

void PatternTable::Display(SaveFile &save) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Pattern selection", mMainWindow,
										GTK_DIALOG_MODAL,
										GTK_STOCK_OK, GTK_RESPONSE_OK,
										"Use", BUTTON_USE,
										GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										NULL);
	GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	GtkWidget *mainbox = gtk_vbox_new (FALSE, 0);
#endif // GTK_CHECK_VERSION
	gtk_container_add(GTK_CONTAINER (content_area), mainbox);

	// Create a list store, and populate it
	// ====================================
	GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	std::vector<string> originalNameList;
	auto f = [store, &originalNameList](const string &name, const string &pattern) {
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, name.c_str(), 1, pattern.c_str(), -1);
		originalNameList.push_back(name);
	};
	save.IteratePatterns(f);

	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	mTreeView = GTK_TREE_VIEW(tree);
	gtk_box_pack_end(GTK_BOX(mainbox), tree, FALSE, FALSE, 0);
	gtk_tree_view_set_grid_lines(mTreeView, GTK_TREE_VIEW_GRID_LINES_BOTH);

	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	auto column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 0, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(mTreeView, column);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(::EditCell), store);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Pattern", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(mTreeView, column);

	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	g_debug("PatternTable::Display response %d", response);
	switch(response) {
	case GTK_RESPONSE_NONE:
		g_debug("PatternTable::Display no response");
		break;
	case GTK_RESPONSE_OK: {
		g_debug("PatternTable::Display Ok");
		for (auto &name : originalNameList) {
			g_debug("Clear pattern %s", name.c_str());
			save.SetPattern(name, ""); // Clear entry
		}
		GtkTreeIter iter;
		for (bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter); valid;
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter))
		{
			gchar *patternName, *patternValue;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
							   0, &patternName,
							   1, &patternValue,
							   -1);
			g_debug("Set pattern %s to %s", patternName, patternValue);
			save.SetPattern(patternName, patternValue);
			g_free(patternName);
			g_free(patternValue);
		}
	}
	case GTK_RESPONSE_CANCEL:
		g_debug("PatternTable::Display cancel");
		break;
	case BUTTON_USE: {
		g_debug("PatternTable::Display use");
		GtkTreeSelection *selection = gtk_tree_view_get_selection(mTreeView);
		if (selection != nullptr) {
			GtkTreeIter selectedPattern = { 0 };
			GtkTreeModel *pattern = 0;
			bool found = gtk_tree_selection_get_selected(selection, &pattern, &selectedPattern);
			g_assert(found);
			GValue val = { 0 };
			gtk_tree_model_get_value(pattern, &selectedPattern, 1, &val);
			Defer valFree([&val](){g_value_unset(&val);});
			const gchar *str = g_value_get_string(&val);
			g_debug("PatternTable::Display use '%s'", str);
		}
		break;
	}
	default:
		g_warning("PatternTable::Display: Unknown return code");
		break;
	}
	gtk_widget_destroy(dialog);
}

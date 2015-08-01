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

#include <set>

#include "PatternTable.h"
#include "SaveFile.h"
#include "Defer.h"
#include "Debug.h"

using std::string;

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, GtkListStore *store) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &iter, path);
	g_assert(found);
	gtk_list_store_set(store, &iter, 0, newString, -1);
}

static void ButtonClicked(GtkButton *button, PatternTable *c) {
	string name = gtk_widget_get_name(GTK_WIDGET(button));
	LPLOG("%s", name.c_str());
	c->ExecuteCommand(name);
}

void PatternTable::ExecuteCommand(const string &name) {
	GtkTreeSelection *selection = gtk_tree_view_get_selection(mTreeView);
	if (selection == nullptr)
		return; // None selected
	GtkTreeIter selectedPattern = { 0 };
	GtkTreeModel *store = 0;
	bool found = gtk_tree_selection_get_selected(selection, &store, &selectedPattern);
	g_assert(found);

	if (name == "delete") {
		LPLOG("delete");
		(void)gtk_list_store_remove(GTK_LIST_STORE(store), &selectedPattern);
	} else if (name == "copy") {
		LPLOG("copy");
		gchar *patternName, *patternValue;
		gtk_tree_model_get(store, &selectedPattern, 0, &patternName, 1, &patternValue, -1);
		gtk_list_store_insert_after(GTK_LIST_STORE(store), &selectedPattern, &selectedPattern);
		gtk_list_store_set(GTK_LIST_STORE(store), &selectedPattern, 0, patternName, 1, patternValue, -1);
		LPLOG("copy pattern %s:%s", patternName, patternValue);
		g_free(patternName);
		g_free(patternValue);
	}
}

bool PatternTable::Display(SaveFile &save) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Select pattern to use", mMainWindow,
										GTK_DIALOG_MODAL,
										"_OK", GTK_RESPONSE_OK,
										"_Cancel", GTK_RESPONSE_CANCEL,
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
	mStore = GTK_TREE_MODEL(store);
	const string currentSelectedName = save.GetStringOption("CurrentPattern");
	// The save file may have changed by another instance. Load it, to make sure we have the latest.
	const string currentPattern = save.GetPattern(currentSelectedName, "");
	LPLOG("Current pattern %s: %s", currentSelectedName.c_str(), currentPattern.c_str());
	save.ClearAllPatterns();
	save.Read(true);
	save.SetPattern(currentSelectedName, currentPattern); // Make sure we keep the current pattern in used
	auto f = [store, this, currentSelectedName](const string &name, const string &pattern) {
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, name.c_str(), 1, pattern.c_str(), -1);
		mOriginalNameList.push_back(name);
		if (name == currentSelectedName) {
			LPLOG("Detect current %s: %s", currentSelectedName.c_str(), pattern.c_str());
			mIterFoundCurrent = iter;
		}
	};
	save.IteratePatterns(f);

	GtkWidget *tree = gtk_tree_view_new_with_model(mStore);
	mTreeView = GTK_TREE_VIEW(tree);
	gtk_box_pack_start(GTK_BOX(mainbox), tree, FALSE, FALSE, 0);
	gtk_tree_view_set_grid_lines(mTreeView, GTK_TREE_VIEW_GRID_LINES_BOTH);

	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &mIterFoundCurrent);
	if (path != nullptr) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), path, nullptr, false);
		gtk_tree_path_free(path);
	}

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

	// Add extra buttons
	// =================
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
	GtkWidget *buttonBox = gtk_hbox_new (FALSE, 0);
#endif // GTK_CHECK_VERSION
	gtk_box_pack_start(GTK_BOX(mainbox), buttonBox, FALSE, FALSE, 0);

	GtkWidget *button = gtk_button_new_with_mnemonic("Delete");
	gtk_button_set_focus_on_click(GTK_BUTTON(button), false);
	gtk_widget_set_name(GTK_WIDGET(button), "delete");
	g_signal_connect (button, "clicked", G_CALLBACK(::ButtonClicked), this);
	gtk_box_pack_start(GTK_BOX(buttonBox), button, FALSE, FALSE, 0);

	button = gtk_button_new_with_mnemonic("Copy");
	gtk_button_set_focus_on_click(GTK_BUTTON(button), false);
	gtk_widget_set_name(GTK_WIDGET(button), "copy");
	g_signal_connect (button, "clicked", G_CALLBACK(::ButtonClicked), this);
	gtk_box_pack_start(GTK_BOX(buttonBox), button, FALSE, FALSE, 0);

retry:
	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	LPLOG("response %d", response);
	bool ret = false;
	switch(response) {
	case GTK_RESPONSE_NONE:
		LPLOG("no response");
		break;
	case GTK_RESPONSE_OK: {
		LPLOG("Ok");
		if (DetectDuplicateNames()) {
			LPLOG("Duplicates found, retry");
			GtkWidget *popup = gtk_message_dialog_new(mMainWindow,
													GTK_DIALOG_MODAL,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CLOSE,
													"Duplicate pattern names\nnot allowed!");
			gtk_widget_show_all(popup);
			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);
			goto retry;
		}
		UpdateList(save);
		GtkTreeSelection *selection = gtk_tree_view_get_selection(mTreeView);
		if (selection != nullptr)
			ret = Select(selection, save);
		break;
	}
	case GTK_RESPONSE_CANCEL:
		LPLOG("cancel");
		break;
	default:
		g_warning("PatternTable::Display: Unknown return code");
		break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

bool PatternTable::Select(GtkTreeSelection *selection, SaveFile &save) {
	GtkTreeIter selectedPattern = { 0 };
	GtkTreeModel *pattern = 0;
	bool found = gtk_tree_selection_get_selected(selection, &pattern, &selectedPattern);
	if (!found)
		return false;
	GValue val = { 0 };
	gtk_tree_model_get_value(pattern, &selectedPattern, 0, &val);
	Defer valFree([&val](){g_value_unset(&val);});
	const gchar *str = g_value_get_string(&val);
	LPLOG("'%s'", str);
	save.SetStringOption("CurrentPattern", str);
	return true;
}

void PatternTable::UpdateList(SaveFile &save) {
	for (auto &name : mOriginalNameList) {
		LPLOG("Clear pattern %s", name.c_str());
		save.SetPattern(name, ""); // Clear entry
	}
	GtkTreeIter iter;
	for (bool valid = gtk_tree_model_get_iter_first(mStore, &iter); valid;
		valid = gtk_tree_model_iter_next(mStore, &iter))
	{
		gchar *patternName, *patternValue;
		gtk_tree_model_get(mStore, &iter,
						   0, &patternName,
						   1, &patternValue,
						   -1);
		LPLOG("Set pattern %s to %s", patternName, patternValue);
		save.SetPattern(patternName, patternValue);
		g_free(patternName);
		g_free(patternValue);
	}
}

bool PatternTable::DetectDuplicateNames() const {
	GtkTreeIter iter;
	std::set<string> nameList;
	bool ret = false;
	for (bool valid = gtk_tree_model_get_iter_first(mStore, &iter); valid;
		valid = gtk_tree_model_iter_next(mStore, &iter))
	{
		gchar *patternName, *patternValue;
		gtk_tree_model_get(mStore, &iter,
						   0, &patternName,
						   1, &patternValue,
						   -1);
        if (nameList.find(string(patternName)) != nameList.end()) {
			LPLOG("Set pattern %s is duplicate!", patternName);
            ret = true;
        }
        nameList.insert(patternName);
		g_free(patternName);
		g_free(patternValue);
	}
	return ret;
}

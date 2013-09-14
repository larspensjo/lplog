// Copyright 2013 Lars Pensj�
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
#include <iostream>
#include <stdlib.h>

#include "Document.h"
#include "View.h"

using std::cout;
using std::endl;

static bool IterEqual(GtkTreeIter *a, GtkTreeIter *b) {
	// I know, the proper way is to compare the iter to a path first.
	return a->stamp == b->stamp &&
		a->user_data == b->user_data &&
		a->user_data2 == b->user_data2 &&
		a->user_data3 == b->user_data3;
}

void View::SetWindowTitle(const std::string &str) {
	std::string newTitle = "LPlog 1.1b       " + str;
	gtk_window_set_title(mWindow, newTitle.c_str());
}

static gboolean DragDrop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data) {
	GList *list = gdk_drag_context_list_targets(context);
	g_assert(list != nullptr);
	GdkAtom target_type = GDK_POINTER_TO_ATOM(g_list_nth_data(list, 0));
	gtk_drag_get_data(widget, context, target_type, time);
	g_debug("DragDrop");
	return true;
}

void View::Create(GCallback buttonCB, GCallback toggleButtonCB, GCallback keyPressed, GCallback editCell,
				  GCallback togglePattern, GCallback changePage, GCallback quitCB, GCallback findCB, gpointer cbData)
{
	// Create the main window
	// ======================
	GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	mWindow = GTK_WINDOW(win);
	gtk_container_set_border_width(GTK_CONTAINER (win), 1);
	gtk_widget_realize (win);
	g_signal_connect(win, "destroy", quitCB, cbData);
	gtk_window_set_default_size(mWindow, 1024, 480);

#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	GtkWidget *mainbox = gtk_vbox_new(FALSE, 0);
#endif // GTK_CHECK_VERSION
	gtk_container_add (GTK_CONTAINER (win), mainbox);

	// Create menus
	// ============

	GtkWidget *menubar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX (mainbox), GTK_WIDGET(menubar), FALSE, FALSE, 0);

	auto menu = this->AddMenu(menubar, "_File");
	this->AddMenuButton(menu, "_Open", "open", buttonCB, cbData);
	this->AddMenuButton(menu, "_Close", "close", buttonCB, cbData);
	this->AddMenuButton(menu, "_Exit", "quit", buttonCB, cbData);

	menu = this->AddMenu(menubar, "_Edit");
	this->AddMenuButton(menu, "_Paste", "paste", buttonCB, cbData);
	this->AddMenuButton(menu, "_Find", "find", buttonCB, cbData);

	menu = this->AddMenu(menubar, "_Help");
	this->AddMenuButton(menu, "_About", "about", buttonCB, cbData);

	// Create the status bar
	// =====================
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *statusBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
#else
	GtkWidget *statusBar = gtk_hbox_new(FALSE, 1);
#endif // GTK_CHECK_VERSION
	gtk_box_pack_end(GTK_BOX (mainbox), GTK_WIDGET(statusBar), FALSE, FALSE, 0);

	mStatusText = GTK_LABEL(gtk_label_new("Status"));
	gtk_box_pack_end(GTK_BOX (statusBar), GTK_WIDGET(mStatusText), TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,6,0)
	mFindEntry = gtk_search_entry_new();
#else
	GtkWidget *findLabel = gtk_label_new("Find:");
	gtk_box_pack_start(GTK_BOX (statusBar), GTK_WIDGET(findLabel), FALSE, FALSE, 0);
	mFindEntry = gtk_entry_new();
#endif
	g_signal_connect(G_OBJECT(mFindEntry), "changed", findCB, cbData);
	gtk_box_pack_start(GTK_BOX (statusBar), GTK_WIDGET(mFindEntry), FALSE, FALSE, 0);

	// Create left pane with buttons
	// =============================
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
#endif // GTK_CHECK_VERSION
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *buttonBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	GtkWidget *buttonBox = gtk_vbox_new(FALSE, 0);
#endif // GTK_CHECK_VERSION
	gtk_box_pack_start(GTK_BOX(hbox), buttonBox, FALSE, FALSE, 0);

	AddButton(buttonBox, "_Line add", "line", buttonCB, cbData);
	AddButton(buttonBox, "_Remove", "remove", buttonCB, cbData);
	AddButton(buttonBox, "_Child add", "child", buttonCB, cbData);

	auto widget = gtk_label_new("Toggle");
	gtk_box_pack_start(GTK_BOX(buttonBox), widget, FALSE, FALSE, 10);

	mAutoScroll = gtk_toggle_button_new_with_label("Autoscroll");
	gtk_widget_set_name(GTK_WIDGET(mAutoScroll), "autoscroll");
	g_signal_connect(G_OBJECT(mAutoScroll), "toggled", G_CALLBACK(toggleButtonCB), cbData );
	gtk_box_pack_start(GTK_BOX(buttonBox), mAutoScroll, FALSE, FALSE, 0);

	auto toggleButton = gtk_toggle_button_new_with_label("Line numbers");
	gtk_widget_set_name(GTK_WIDGET(toggleButton), "linenumbers");
	g_signal_connect(G_OBJECT(toggleButton), "toggled", G_CALLBACK(toggleButtonCB), cbData );
	gtk_box_pack_start(GTK_BOX(buttonBox), toggleButton, FALSE, FALSE, 0);

	// Create the horizontal pane for tree view and text view
	// ======================================================
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget *hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
#else
	GtkWidget *hpaned = gtk_hpaned_new();
#endif // GTK_CHECK_VERSION
	GtkWidget *frame1 = gtk_frame_new (NULL);
	GtkWidget *frame2 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);
	gtk_paned_pack1(GTK_PANED (hpaned), frame1, FALSE, TRUE);
	gtk_paned_pack2(GTK_PANED (hpaned), frame2, TRUE, FALSE);
	gtk_widget_set_size_request (frame2, 300, -1);
	gtk_box_pack_start(GTK_BOX(hbox), hpaned, TRUE, TRUE, 0);

	// Create the tree model and add some initial pattern
	// ==================================================
	mPattern = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
	gtk_tree_store_append(mPattern, &mPatternRoot, NULL);
	gtk_tree_store_set(mPattern, &mPatternRoot, 0, "|", 1, true, -1);
	GtkTreeIter child;
	gtk_tree_store_insert_after(mPattern, &child, &mPatternRoot, NULL);
	gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1);

	// Create the tree view
	// ====================
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(mPattern));
	mTreeView = GTK_TREE_VIEW(tree);
	gtk_tree_view_set_enable_search(mTreeView, false);
	g_signal_connect(G_OBJECT(tree), "key-press-event", keyPressed, cbData );

	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", editCell, cbData );
	auto column = gtk_tree_view_column_new_with_attributes("Pattern", renderer, "text", 0, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(mTreeView, column);

	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled", togglePattern, cbData );
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "active", 1, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(mTreeView, column);

	GtkWidget *scrollview = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollview), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scrollview, 150, -1);
	gtk_container_add(GTK_CONTAINER(scrollview), tree);
	gtk_container_add(GTK_CONTAINER(frame1), scrollview);
	gtk_tree_view_expand_all(mTreeView);

	// Create the notebook
	// ===================
	mNotebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(frame2), mNotebook);
	g_signal_connect(G_OBJECT(mNotebook), "switch-page", changePage, cbData );

	// Create tags
	// ===========
	GtkTextTagTable *mTextTagTable = gtk_text_tag_table_new();
	GtkTextTag *mBoldTag = gtk_text_tag_new("bold");

	gtk_widget_show_all(win);
}

void View::SetFocusFind() {
	gtk_widget_grab_focus(mFindEntry);
}

int View::AddTab(Document *doc, gpointer cbData, GCallback dragReceived, GCallback textViewkeyPress, bool switchTab) {
	GtkWidget *labelWidget = gtk_label_new(doc->GetFileNameShort().c_str());
	std::stringstream ss;
	ss << nextId;
	gtk_widget_set_name(labelWidget, ss.str().c_str());

	// Create the text display window
	GtkWidget *scrollview = gtk_scrolled_window_new( NULL, NULL );
	doc->mScrolledView = GTK_SCROLLED_WINDOW(scrollview);
	gtk_scrolled_window_set_policy(doc->mScrolledView, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(scrollview), 1);
	auto textview = gtk_text_view_new();
	gtk_drag_dest_set(textview, GTK_DEST_DEFAULT_DROP, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(textview);
	g_signal_connect(G_OBJECT(textview), "drag-drop", G_CALLBACK(DragDrop), cbData );
	g_signal_connect(G_OBJECT(textview), "drag-data-received", dragReceived, cbData );
	g_signal_connect(G_OBJECT(textview), "key-press-event", textViewkeyPress, cbData );
	doc->mTextView = GTK_TEXT_VIEW(textview);
	gtk_text_view_set_wrap_mode(doc->mTextView, GTK_WRAP_CHAR);
	PangoFontDescription *font = pango_font_description_from_string("Monospace Regular 8");
	gtk_widget_modify_font(textview, font);
	gtk_container_add(GTK_CONTAINER (scrollview), textview);
	int page = gtk_notebook_prepend_page(GTK_NOTEBOOK(mNotebook), scrollview, labelWidget);
	gtk_widget_show_all(scrollview);
	g_debug("[%d] View::AddTab id %d prev page %d new page %d switching %d", GetCurrentTabId(), nextId, gtk_notebook_get_current_page(GTK_NOTEBOOK(mNotebook)), page, switchTab);
	if (switchTab)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(mNotebook), page);

	return nextId++;
}

void View::CloseCurrentTab() {
	int id = GetCurrentTabId();
	int tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(mNotebook));
	g_debug("[%d] View::CloseCurrentTab tab %d", id, tab);
	gtk_notebook_remove_page(GTK_NOTEBOOK(mNotebook), tab);
}

int View::GetCurrentTabId() const {
	int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(mNotebook));
	GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(mNotebook), page);
	if (child == nullptr)
		return -1;
	GtkWidget *labelWidget = gtk_notebook_get_tab_label(GTK_NOTEBOOK(mNotebook), child);
	if (labelWidget == nullptr)
		return -1;
	const char *name = gtk_widget_get_name(labelWidget);
	return atoi(name);
}

void View::DimCurrentTab() {
	int page = gtk_notebook_get_current_page(GTK_NOTEBOOK(mNotebook));
	GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(mNotebook), page);
	GtkWidget *labelWidget = gtk_notebook_get_tab_label(GTK_NOTEBOOK(mNotebook), child);
#if GTK_CHECK_VERSION(3,0,0)
	GdkRGBA color = {0.7, 0.7, 0.7, 1};
	gtk_widget_override_background_color(labelWidget, GTK_STATE_FLAG_NORMAL, &color);
#else
	GdkColor color;
	gdk_color_parse ("black", &color);
	color.red=128<<8; color.green=128<<8; color.blue=128<<8;
	gtk_widget_modify_bg(labelWidget, GTK_STATE_NORMAL, &color);
#endif
	g_debug("[%d] View::DimCurrentTab page %d", GetCurrentTabId(), page);
}

GtkWidget *View::FileOpenDialog() {
	return gtk_file_chooser_dialog_new("Open File", mWindow, GTK_FILE_CHOOSER_ACTION_OPEN,
										GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										NULL);
}

void View::ToggleLineNumbers(Document *doc) {
	mShowLineNumbers = !mShowLineNumbers;
	// Remember the current scrollbar value
	g_assert(doc->mScrolledView != nullptr);
	auto adj = gtk_scrolled_window_get_vadjustment(doc->mScrolledView);
	gdouble pos = gtk_adjustment_get_value(adj);
	this->Replace(doc);
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll)))
		gtk_adjustment_set_value(adj, pos+0.1); // A delta is needed, or it will be a noop!
	g_debug("[%d] View::ToggleLineNumbers scrollbar pos %f", GetCurrentTabId(), pos);
}

void View::FilterString(std::stringstream &ss, Document *doc, bool restartFirstLine) {
	GtkTreeIter iter;
	bool empty = !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(mPattern), &iter);
	g_assert(!empty);
	std::string separator = ""; // Start empty
	unsigned startLine = mFoundLines;
	if (mFoundLines > 0)
		separator = '\n';
	// Add the lines to ss, one at a time. The last line shall not have a newline.
	auto TestLine = [&] (const std::string &str, unsigned line) {
		if (isShown(str, GTK_TREE_MODEL(mPattern), &iter) != Evaluation::Nomatch) {
			ss << separator;
			if (mShowLineNumbers) {
				ss.width(5);
				ss.setf(ss.left);
				ss << line+1 << " ";
			}
			ss << str;
			separator = "\n";
			++mFoundLines;
		}
	};
	doc->IterateLines(TestLine, restartFirstLine);
	g_debug("[%d] View::FilterString starting line %d, ending %d", GetCurrentTabId(), startLine, mFoundLines);
}

void View::OpenPatternForEditing() {
	GtkTreeSelection *selection = gtk_tree_view_get_selection(mTreeView);
	GtkTreeModel *pattern = 0;
	GtkTreeIter selectedPattern = { 0 };
	bool found = gtk_tree_selection_get_selected(selection, &pattern, &selectedPattern);
	if (!found)
		return;
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &selectedPattern);
	if (path == nullptr)
		return;
	GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
	gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
	gtk_tree_path_free(path);
}

bool View::RootPatternActive() {
	GValue val = { 0 };
	gtk_tree_model_get_value(GTK_TREE_MODEL(mPattern), &mPatternRoot, 1, &val);
	bool active = g_value_get_boolean(&val);
	g_value_unset(&val);
	return active;
}

View::Evaluation View::isShown(const std::string &line, GtkTreeModel *pattern, GtkTreeIter *iter) {
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

void View::TogglePattern(gchar *path) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( mPattern ), &iter, path );
	g_assert(found);
	GValue val = { 0 };
	gtk_tree_model_get_value(GTK_TREE_MODEL(mPattern), &iter, 1, &val);
	bool current = g_value_get_boolean(&val);
	gtk_tree_store_set(mPattern, &iter, 1, !current, -1);
	g_value_unset(&val);
}

void View::Append(Document *doc) {
	// Remember the current scrollbar value
	g_assert(doc->mScrolledView != nullptr);
	auto adj = gtk_scrolled_window_get_vadjustment(doc->mScrolledView);
	gdouble pos = gtk_adjustment_get_value(adj);
	g_debug("[%d] View::Append scrollbar %f", GetCurrentTabId(), pos);
	std::stringstream ss;
	this->FilterString(ss, doc, false);
	GtkTextIter last;
	g_assert(doc->mTextView != nullptr);
	auto buffer = gtk_text_view_get_buffer(doc->mTextView);
	gtk_text_buffer_get_end_iter(buffer, &last);
	gtk_text_buffer_insert(buffer, &last, ss.str().c_str(), -1);
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll)))
		gtk_adjustment_set_value(adj, pos+0.1); // A delta is needed, or it will be a noop!
}

void View::Replace(Document *doc) {
	mFoundLines = 0;
	g_debug("[%d] View::Replace", GetCurrentTabId());
	std::stringstream ss;
	this->FilterString(ss, doc, true);
	g_assert(doc->mTextView != nullptr);
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(doc->mTextView), ss.str().c_str(), -1);
}

void View::FindNext(Document *doc, const std::string &str) {
	GtkTextBuffer *buff = gtk_text_view_get_buffer(doc->mTextView);
	unsigned lineCount = gtk_text_buffer_get_line_count(buff);
	GtkTextIter lineStart;
	gtk_text_buffer_get_iter_at_offset(buff, &lineStart, 0);
	for (unsigned line=0; line < lineCount; line++) {
		GtkTextIter lineEnd;
		if (line == lineCount-1)
			gtk_text_buffer_get_iter_at_offset(buff, &lineEnd, -1);
		else
			gtk_text_buffer_get_iter_at_line(buff, &lineEnd, line+1);
		const std::string currentLine = gtk_text_buffer_get_text(buff, &lineStart, &lineEnd, FALSE);
		if (currentLine.find(str) != std::string::npos) {
			// g_debug("FindNext: line %d: '%s'", line, currentLine.c_str());
			gtk_text_view_scroll_to_iter(doc->mTextView, &lineStart, 0.0, true, 0.5, 0.5);
			return;
		}
		lineStart = lineEnd;
	}
}

void View::UpdateStatusBar(Document *doc) {
	if (doc == nullptr) {
		gtk_label_set_text(mStatusText, "");
		return;
	}
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll))) {
		GtkTextIter lastLine;
		g_assert(doc->mTextView != nullptr);
		auto buffer = gtk_text_view_get_buffer(doc->mTextView);
		gtk_text_buffer_get_end_iter(buffer, &lastLine);
		GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, NULL, &lastLine, true);
		gtk_text_view_scroll_to_mark(doc->mTextView, mark, 0.0, true, 0.0, 1.0);
	}
	std::stringstream ss;
	ss << doc->GetFileName() << "   " << doc->Date() << "                     " << mFoundLines << " (" << doc->GetNumLines() << ")";
	gtk_label_set_text(mStatusText, ss.str().c_str());
}

void View::EditPattern(gchar *path, gchar *newString) {
	GtkTreeIter iter;
	bool found = gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( mPattern ), &iter, path );
	g_assert(found);
	gtk_tree_store_set(mPattern, &iter, 0, newString, -1);
}

bool View::FindSelectedPattern(GtkTreeIter *selectedPattern) const {
	GtkTreeSelection *selection = gtk_tree_view_get_selection(mTreeView);
	GtkTreeModel *pattern = 0;
	bool found = gtk_tree_selection_get_selected(selection, &pattern, selectedPattern);
	g_assert(found || pattern == GTK_TREE_MODEL(mPattern));
	return found;
}

void View::DeletePattern() {
	GtkTreeIter selectedPattern = { 0 };
	bool found = FindSelectedPattern(&selectedPattern);
	if (!found || IterEqual(&mPatternRoot, &selectedPattern))
		return; // Don't allow deletion of the root pattern
	(void)gtk_tree_store_remove(mPattern, &selectedPattern);
}

void View::AddPatternLine() {
	GtkTreeIter selectedPattern = { 0 };
	bool found = FindSelectedPattern(&selectedPattern);
	if (!found || IterEqual(&mPatternRoot, &selectedPattern))
		return; // Don't allow new pattern parallel with root
	GtkTreeIter child;
	gtk_tree_store_insert_after(mPattern, &child, NULL, &selectedPattern);
	gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1); // Initialize new value with empty string and enabled.
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
	GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
	gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
	gtk_tree_path_free(path);
}

void View::AddPatternLineIndented() {
	GtkTreeIter selectedPattern = { 0 };
	bool found = FindSelectedPattern(&selectedPattern);
	if (!found)
		return;
	GtkTreeIter child;
	gtk_tree_store_insert_after(mPattern, &child, &selectedPattern, NULL);
	gtk_tree_store_set(mPattern, &child, 0, "", 1, true, -1); // Initialize new value with empty string and enabled.
	GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(mPattern), &child);
	gtk_tree_view_expand_to_path(mTreeView, path);
	GtkTreeViewColumn *firstColumn = gtk_tree_view_get_column(mTreeView, 0);
	gtk_tree_view_set_cursor(mTreeView, path, firstColumn, true);
	gtk_tree_path_free(path);
}

GtkWidget *View::AddMenu(GtkWidget *menubar, const gchar *label) {
	GtkWidget *menu = gtk_menu_new();
	auto menuItem = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuItem);
	return menu;
}

void View::AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb, gpointer cbData) {
	auto menuItem = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	gtk_widget_set_name(GTK_WIDGET(menuItem), name);
	g_signal_connect(menuItem, "activate", cb, cbData);
}

void View::AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb, gpointer cbData) {
	GtkWidget *button = gtk_button_new_with_mnemonic(label);
	gtk_button_set_focus_on_click(GTK_BUTTON(button), false);
	gtk_widget_set_name(GTK_WIDGET(button), name);
	g_signal_connect (button, "clicked", cb, cbData);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

void View::About() {
	const char *license =
		"LPlog is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, version 3.\n\n"
		"LPlog is distributed in the hope that it will be useful\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n";

	const char *authors[] = {
		"Lars Pensj\303\266 <lars.pensjo@gmail.com>",
		NULL
	};

	const gchar* copyright = { "Copyright (c) Lars Pensj\303\266" };

	gtk_show_about_dialog(NULL,
		"version", "1.1 beta",
		"website", "https://github.com/larspensjo/lplog",
		"comments", "A program to display and filter a log file.",
		"authors", authors,
		"license", license,
		"program-name", "LPlog",
		"copyright", copyright,
		NULL);
}

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

#include <iostream>
#include <gdk/gdkkeysyms.h> // Needed for GTK+-2.0
#include <string.h>

#include "Document.h"
#include "View.h"

using std::cout;
using std::endl;

void View::SetWindowTitle(const std::string &str) {
	gtk_window_set_title (mWindow, ("LPlog " + str).c_str());
}

GtkTextBuffer *View::Create(GtkTreeModel *model, GCallback buttonCB, GCallback toggleButtonCB, GCallback clickPatternToggle, gpointer cbData)
{
	/* Create the main window */
	GtkWidget *win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	mWindow = GTK_WINDOW(win);
	gtk_container_set_border_width (GTK_CONTAINER (win), 1);
	gtk_widget_realize (win);
	g_signal_connect (win, "destroy", gtk_main_quit, NULL);
	gtk_window_set_default_size(mWindow, 800, 640);

	GtkWidget *mainbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (win), mainbox);

	GtkWidget *menubar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX (mainbox), GTK_WIDGET(menubar), FALSE, FALSE, 0);

	auto menu = this->AddMenu(menubar, "_File");
	this->AddMenuButton(menu, "_Open", "open", buttonCB);
	this->AddMenuButton(menu, "_Exit", "quit", buttonCB);

	menu = this->AddMenu(menubar, "_Edit");
	this->AddMenuButton(menu, "_Paste", "paste", buttonCB);

	menu = this->AddMenu(menubar, "_Help");
	this->AddMenuButton(menu, "_About", "about", buttonCB);

	mStatusBar = GTK_LABEL(gtk_label_new("Status"));
	gtk_box_pack_end(GTK_BOX (mainbox), GTK_WIDGET(mStatusBar), FALSE, FALSE, 0);

	/* Create a vertical box with buttons */
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_end (GTK_BOX (mainbox), hbox, TRUE, TRUE, 0);

	GtkWidget *buttonBox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), buttonBox, FALSE, FALSE, 0);

	AddButton(buttonBox, "_Line add", "line", buttonCB);
	AddButton(buttonBox, "_Remove", "remove", buttonCB);
	AddButton(buttonBox, "_Child add", "child", buttonCB);

	mAutoScroll = gtk_check_button_new_with_label("Autoscroll");
	gtk_widget_set_name(GTK_WIDGET(mAutoScroll), "autoscroll");
	g_signal_connect(G_OBJECT(mAutoScroll), "toggled", G_CALLBACK(toggleButtonCB), cbData );
	gtk_box_pack_start(GTK_BOX(buttonBox), mAutoScroll, FALSE, FALSE, 0);

	auto toggleButton = gtk_check_button_new_with_label("Line numbers");
	gtk_widget_set_name(GTK_WIDGET(toggleButton), "linenumbers");
	g_signal_connect(G_OBJECT(toggleButton), "toggled", G_CALLBACK(toggleButtonCB), cbData );
	gtk_box_pack_start(GTK_BOX(buttonBox), toggleButton, FALSE, FALSE, 0);

	// Create the tree view
	GtkWidget *tree = gtk_tree_view_new_with_model(pattern);
	mTreeView = GTK_TREE_VIEW(tree);
	gtk_tree_view_set_enable_search(mTreeView, false);
	g_signal_connect(G_OBJECT(tree), "cursor-changed", clickPatternToggle, cbData );
	g_signal_connect(G_OBJECT(tree), "key-press-event", G_CALLBACK(::KeyPressed), cbData );

	auto renderer = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(renderer), "editable", TRUE, "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(::EditCell), cbData );
	auto column = gtk_tree_view_column_new_with_attributes("Pattern", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(mTreeView, column);

	renderer = gtk_cell_renderer_toggle_new();
	// g_object_set(G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(::TogglePattern), cbData );
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer, "active", 1, NULL);
	gtk_tree_view_append_column(mTreeView, column);

	gtk_box_pack_start(GTK_BOX(hbox), tree, FALSE, FALSE, 0);
	gtk_tree_view_expand_all(mTreeView);

	PangoFontDescription *font = pango_font_description_from_string("Monospace Regular 8");

	// Create the text display window
	GtkWidget *scrollview = gtk_scrolled_window_new( NULL, NULL );
	mScrolledView = GTK_SCROLLED_WINDOW(scrollview);
	gtk_scrolled_window_set_policy(mScrolledView, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width(GTK_CONTAINER(scrollview), 1);
	auto textview = gtk_text_view_new();
	g_signal_connect(G_OBJECT(textview), "key-press-event", G_CALLBACK(::TextViewKeyPress), cbData );
	mTextView = GTK_TEXT_VIEW(textview);
	gtk_text_view_set_wrap_mode(mTextView, GTK_WRAP_CHAR);
	gtk_widget_modify_font(textview, font);
	gtk_text_view_set_editable(mTextView, false);
	gtk_widget_set_size_request(textview, 5, 5);
	auto buffer = gtk_text_view_get_buffer(mTextView);
	gtk_container_add(GTK_CONTAINER (scrollview), textview);
	gtk_box_pack_start(GTK_BOX(hbox), scrollview, TRUE, TRUE, 0);

	g_timeout_add(1000, G_CALLBACK(::TestForeChanges), cbData);

	/* Enter the main loop */
	gtk_widget_show_all (win);
	return buffer;
}

void ToggleLineNumbers(Document *doc) {
	doc->ToggleLineNumbers();
	// Remember the current scrollbar value
	auto adj = gtk_scrolled_window_get_vadjustment(mScrolledView);
	gdouble pos = gtk_adjustment_get_value(adj);
	doc->Replace(mBuffer, GTK_TREE_MODEL(mPattern));
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll)))
		gtk_adjustment_set_value(adj, pos+0.1); // A delta is needed, or it will be a noop!
}

void View::Append(Document *doc) {
	// Remember the current scrollbar value
	auto adj = gtk_scrolled_window_get_vadjustment(mScrolledView);
	gdouble pos = gtk_adjustment_get_value(adj);
	doc->Append();
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll)))
		gtk_adjustment_set_value(adj, pos+0.1); // A delta is needed, or it will be a noop!
	SetStatus(mDoc->Status());
}

void View::SetStatus(const std::string &str) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mAutoScroll))) {
		GtkTextIter lastLine;
		gtk_text_buffer_get_end_iter(mBuffer, &lastLine);
		GtkTextMark *mark = gtk_text_buffer_create_mark(mBuffer, NULL, &lastLine, true);
		gtk_text_view_scroll_to_mark(mTextView, mark, 0.0, true, 0.0, 1.0);
	}
	gtk_label_set_text(mStatusBar, str.c_str());
}

GtkWidget *View::AddMenu(GtkWidget *menubar, const gchar *label) {
	GtkWidget *menu = gtk_menu_new();
	auto menuItem = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuItem);
	return menu;
}

void View::AddMenuButton(GtkWidget *menu, const gchar *label, const gchar *name, GCallback cb) {
	auto menuItem = gtk_menu_item_new_with_mnemonic(label);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	gtk_widget_set_name(GTK_WIDGET(menuItem), name);
	g_signal_connect (menuItem, "activate", cb, cbData);
}

void View::AddButton(GtkWidget *box, const gchar *label, const gchar *name, GCallback cb) {
	GtkWidget *button = gtk_button_new_with_mnemonic(label);
	gtk_button_set_focus_on_click(GTK_BUTTON(button), false);
	gtk_widget_set_name(GTK_WIDGET(button), name);
	g_signal_connect (button, "clicked", cb, cbData);
	gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
}

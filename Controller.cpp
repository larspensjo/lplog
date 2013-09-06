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

#include <iostream>
#include <string.h>
#include <gdk/gdkkeysyms.h> // Needed for GTK+-2.0

#include "Controller.h"
#include "Document.h"

using std::cout;
using std::endl;

static gboolean KeyPressed(GtkTreeView *, GdkEvent *event, Controller *c) {
	return c->KeyEvent(event);
}

static void ButtonClicked(GtkButton *button, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(button));
	if (name == "quit")
		gtk_main_quit();
	else if (name == "line")
		c->KeyPressed(GDK_KEY_o);
	else if (name == "remove")
		c->KeyPressed(GDK_KEY_Delete);
	else if (name == "child")
		c->KeyPressed(GDK_KEY_a);
	else if (name == "about")
		c->About();
	else if (name == "open")
		c->FileOpenDialog();
	else if (name == "paste")
		c->TextViewKeyPress(GDK_KEY_Paste);
	else
		cout << "Unknown button: " << name << endl;
}

static gboolean TestForeChanges(Controller *c)
{
	c->PollInput();
	return true; // Keep timer going for ever
}

static void EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString, Controller *c)
{
	c->EditCell(renderer, path, newString);
}

static gboolean TextViewKeyPress(GtkWidget *widget, GdkEvent *event, Controller *c) {
#if 0
	cout << "keyval: " << event->key.keyval;
	cout << " is_modifier: " << event->key.is_modifier;
	cout << " hardware_keycode: " << event->key.hardware_keycode;
	cout << " type: " << event->key.type;
	cout << " string: " << event->key.string;
	cout << endl;
#endif
	return c->TextViewKeyPress(event->key.keyval);
}

static void TogglePattern(GtkCellRendererToggle *renderer, gchar *path, Controller *c) {
	c->TogglePattern(renderer, path);
}

static void ToggleButton(GtkToggleButton *togglebutton, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(togglebutton));
	c->ToggleButton(name);
}

void Controller::PollInput() {
	if (mDoc.UpdateInputData()) {
		mView.Append(&mDoc);
	}
}

void Controller::TogglePattern(GtkCellRendererToggle *renderer, gchar *path) {
	mView.TogglePattern(path);
	mView.Replace(&mDoc);
}

void Controller::ToggleButton(const std::string &name) {
	if (name == "autoscroll")
		mView.UpdateStatusBar(&mDoc); // This will use the new automatic scrolling
	else if (name == "linenumbers") {
		mView.ToggleLineNumbers(&mDoc);
	} else
		cout << "Unknown toggle button: " << name << endl;
}

void Controller::EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	mView.EditPattern(path, newString);
	mView.Replace(&mDoc);
}

gboolean Controller::TextViewKeyPress(guint keyval) {
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_Paste:
		{
			GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			gchar *p = gtk_clipboard_wait_for_text(clipboard);
			if (p != nullptr) {
				// cout << "Pasted text: " << p << " key " << endl;
				unsigned size = strlen(p);
				mDoc.AddSourceText(p, size);
				g_free(p);
				mView.Replace(&mDoc);
				mView.SetWindowTitle("LPlog Pasted");
			}
			stopEvent = true;
		}
		break;
	}
	return stopEvent; // Stop event from propagating
}

gboolean Controller::KeyPressed(guint keyval) {
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_F2:
		mView.OpenPatternForEditing(&mDoc);
		stopEvent = true;
		break;
	case GDK_KEY_Delete:
	case GDK_KEY_KP_Delete:
		mView.DeletePattern();
		stopEvent = true;
		break;
	case GDK_KEY_o:
	case GDK_KEY_plus:
	case GDK_KEY_KP_Add:
		mView.AddPatternLine();
		stopEvent = true;
		break;
	case GDK_KEY_a:
		mView.AddPatternLineIndented();
		stopEvent = true;
		break;
	}
	if (!stopEvent)
		return false;
	mView.Replace(&mDoc);
	mView.UpdateStatusBar(&mDoc);
	return true; // Stop event from propagating
}

gboolean Controller::KeyEvent(GdkEvent *event) {
	return this->KeyPressed(event->key.keyval);
}

void Controller::Run(int argc, char *argv[]) {
	mView.Create(G_CALLBACK(::ButtonClicked), G_CALLBACK(::ToggleButton), G_CALLBACK(::KeyPressed), G_CALLBACK(::EditCell),
				G_CALLBACK(::TextViewKeyPress), GSourceFunc(::TestForeChanges), G_CALLBACK(::TogglePattern), this);
	if (argc > 1) {
		mDoc.AddSourceFile(argv [1]);
		mView.SetWindowTitle(argv[1]);
	} else
		mView.SetWindowTitle("");

	mDoc.UpdateInputData();
	mView.Replace(&mDoc);
	gtk_main ();
}

void Controller::FileOpenDialog() {
	GtkWidget *dialog = mView.FileOpenDialog();
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		mDoc.AddSourceFile(filename);
		g_free(filename);
		mView.SetWindowTitle("LPlog " + mDoc.FileName());
		mDoc.UpdateInputData();
		mView.Replace(&mDoc);
	}
	gtk_widget_destroy(dialog);
}

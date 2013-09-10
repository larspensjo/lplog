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
#include <stdlib.h>
#include <gdk/gdkkeysyms.h> // Needed for GTK+-2.0

#include "Controller.h"
#include "Document.h"

using std::cout;
using std::endl;

static const std::string filePrefixURI = "file://";

static gboolean KeyPressed(GtkTreeView *, GdkEvent *event, Controller *c) {
	return c->KeyEvent(event);
}

static void ButtonClicked(GtkButton *button, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(button));
	if (name == "quit")
		c->Quit();
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
	(void)c->TextViewKeyPress(event->key.keyval);
	return true; // Return true, to consume all key presses.
}

static void TogglePattern(GtkCellRendererToggle *renderer, gchar *path, Controller *c) {
	c->TogglePattern(renderer, path);
}

static void ToggleButton(GtkToggleButton *togglebutton, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(togglebutton));
	c->ToggleButton(name);
}

static void DragDataReceived(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, Controller *c) {
	g_assert(info == 0); // Only support one type for now
	char **str = gtk_selection_data_get_uris(data);
	bool success = false;
	if (str != nullptr) {
		c->OpenURI(str[0]); // Only get the first reference for now
		g_strfreev(str);
		success = true;
	}
	gtk_drag_finish(context, success, false, time);
}

static void ChangeCurrentPage(GtkNotebook *notebook, GtkWidget *page, gint tab, Controller *c) {
	GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), tab);
	GtkWidget *labelWidget = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), child);
	const char *name = gtk_widget_get_name(labelWidget);
	// g_print("Notebook CB: page %d, id %d\n", tab, atoi(name));
	c->ChangeDoc(atoi(name));
}

static gboolean DestroyWindow(GtkWidget *widget, Controller *c) {
	c->Quit();
	return false;
}

void Controller::ChangeDoc(int id) {
	mCurrentDoc = &mDocumentList[id];
	g_print("Change to doc %d, lines %d, doc %x\n", id, mCurrentDoc->GetNumLines(), mCurrentDoc);
	this->PollInput();
	mForceReplace = true;
}

void Controller::OpenURI(const std::string &uri) {
	unsigned prefixSize = filePrefixURI.size();
	if (uri.size() < prefixSize)
		return;
	if (uri.substr(0, prefixSize) != filePrefixURI)
		return;
	const std::string filename = uri.substr(prefixSize);
	mCurrentDoc = &mDocumentList[mView.nextId];
	mCurrentDoc->AddSourceFile(filename);
	mCurrentDoc->UpdateInputData();
	mView.AddTab(mCurrentDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), true);
}

void Controller::PollInput(bool forceUpdate) {
	if (mCurrentDoc == nullptr)
		return; // There is no current document
	unsigned lines = mCurrentDoc->GetNumLines();
	if (mCurrentDoc->UpdateInputData() || forceUpdate) {
		if (mCurrentDoc->GetNumLines() < lines) {
			mView.DimCurrentTab();
			mCurrentDoc->StopUpdate(); // The old tab shall no longer update
			std::string fn = mCurrentDoc->GetFileName();
			mCurrentDoc = &mDocumentList[mView.nextId]; // Restarted new file
			mCurrentDoc->AddSourceFile(fn);
			mView.AddTab(mCurrentDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress));
			mForceReplace = true;
		} else {
			mView.Append(mCurrentDoc);
		}
	}
}

void Controller::TogglePattern(GtkCellRendererToggle *renderer, gchar *path) {
	mView.TogglePattern(path);
	mForceReplace = true;
}

void Controller::ToggleButton(const std::string &name) {
	if (name == "autoscroll")
		mView.UpdateStatusBar(mCurrentDoc); // This will use the new automatic scrolling
	else if (name == "linenumbers") {
		mView.ToggleLineNumbers(mCurrentDoc);
	} else
		cout << "Unknown toggle button: " << name << endl;
}

void Controller::EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	mView.EditPattern(path, newString);
	mForceReplace = true;
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
				mCurrentDoc = &mDocumentList[mView.nextId];
				unsigned size = strlen(p);
				mCurrentDoc->AddSourceText(p, size);
				g_free(p);
				mView.AddTab(mCurrentDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), true);
				mForceReplace = true;
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
		mView.OpenPatternForEditing(mCurrentDoc);
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
	mForceReplace = true;
	return true; // Stop event from propagating
}

gboolean Controller::KeyEvent(GdkEvent *event) {
	return this->KeyPressed(event->key.keyval);
}

void Controller::Run(int argc, char *argv[]) {
	mView.Create(G_CALLBACK(::ButtonClicked), G_CALLBACK(::ToggleButton), G_CALLBACK(::KeyPressed), G_CALLBACK(::EditCell),
				 G_CALLBACK(::TogglePattern), G_CALLBACK(::ChangeCurrentPage), G_CALLBACK(::DestroyWindow), this);
	if (argc > 1) {
		this->OpenURI(filePrefixURI + argv[1]);
	}
	g_timeout_add(1000, GSourceFunc(::TestForeChanges), this);
	while (!mQuitNow) {
		gtk_main_iteration();
		if (mForceReplace) {
			mForceReplace = false;
			mView.Replace(mCurrentDoc);
			mView.UpdateStatusBar(mCurrentDoc);
		}
	}
}

void Controller::FileOpenDialog() {
	GtkWidget *dialog = mView.FileOpenDialog();
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		this->OpenURI(filePrefixURI + filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

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
	g_debug("ButtonClicked %s", name.c_str());
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
	else if (name == "close")
		c->CloseCurrentTab();
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
	g_debug("DragDataReceived %s", str[0]);
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
	g_debug("ChangeCurrentPage tab %d widget %s", tab, name);
	c->ChangeDoc(atoi(name));
}

static gboolean DestroyWindow(GtkWidget *widget, Controller *c) {
	c->Quit();
	return false;
}

void Controller::CloseCurrentTab() {
	int id = mView.GetCurrentTabId();
	if (id == -1)
		return;
	g_debug("[%d] Controller::CloseCurrentTab tab", id);
	if (mCurrentDoc == &mDocumentList[id])
		mCurrentDoc = nullptr;
	mDocumentList.erase(id);
	mView.CloseCurrentTab();
	mView.SetWindowTitle("");
	mView.UpdateStatusBar(nullptr);
}

void Controller::ChangeDoc(int id) {
	mCurrentDoc = &mDocumentList[id];
	g_debug("[%d] Controller::ChangeDoc doc (%p), lines %u", id, mCurrentDoc, mCurrentDoc->GetNumLines());
	this->PollInput();
	mView.SetWindowTitle(mCurrentDoc->GetFileNameShort());
	mQueueReplace = true;
}

void Controller::OpenURI(const std::string &uri) {
	unsigned prefixSize = filePrefixURI.size();
	if (uri.size() < prefixSize)
		return;
	if (uri.substr(0, prefixSize) != filePrefixURI)
		return;
	const std::string filename = uri.substr(prefixSize);
	mCurrentDoc = &mDocumentList[mView.nextId];
	g_debug("[%d] Controller::OpenURI %s new document %p", mView.GetCurrentTabId(), uri.c_str(), mCurrentDoc);
	mCurrentDoc->AddSourceFile(filename);
	mView.AddTab(mCurrentDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), true);
}

void Controller::PollInput() {
	if (mCurrentDoc == nullptr)
		return; // There is no current document
	Document::UpdateResult res = mCurrentDoc->UpdateInputData();
	switch (res) {
	case Document::UpdateResult::Replaced: {
		mView.DimCurrentTab();
		mCurrentDoc->StopUpdate(); // The old tab shall no longer update
		std::string fn = mCurrentDoc->GetFileName();
		Document *newDoc = &mDocumentList[mView.nextId]; // Restarted new file
		g_debug("[%d] Controller::PollInput new document %p for %s", mView.GetCurrentTabId(), newDoc, fn.c_str());
		newDoc->AddSourceFile(fn);
		mView.AddTab(newDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), false);
		break;
	}
	case Document::UpdateResult::Grow:
		mQueueAppend = true;
		break;
	case Document::UpdateResult::NoChange:
		break;
	}
}

void Controller::TogglePattern(GtkCellRendererToggle *renderer, gchar *path) {
	g_debug("[%d] Controller::TogglePattern %s", mView.GetCurrentTabId(), path);
	mView.TogglePattern(path);
	// Inhibit update if root pattern is disabled
	if (mRootPatternDisabled && mView.RootPatternActive()) {
		g_debug("[%d] Controller::ToggleButton Root pattern enabled", mView.GetCurrentTabId());
		mRootPatternDisabled = false;
	}
	if (!mRootPatternDisabled)
		mQueueReplace = true;
	if (!mView.RootPatternActive()) {
		g_debug("[%d] Controller::ToggleButton Root pattern disabled", mView.GetCurrentTabId());
		mRootPatternDisabled = true;
	}
}

void Controller::ToggleButton(const std::string &name) {
	g_debug("[%d] Controller::ToggleButton %s", mView.GetCurrentTabId(), name.c_str());
	if (name == "autoscroll")
		mView.UpdateStatusBar(mCurrentDoc); // This will use the new automatic scrolling
	else if (name == "linenumbers") {
		mView.ToggleLineNumbers(mCurrentDoc);
	} else
		cout << "Unknown toggle button: " << name << endl;
}

void Controller::EditCell(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	mView.EditPattern(path, newString);
	// Inhibit update if root pattern is disabled
	if (!mRootPatternDisabled)
		mQueueReplace = true;
}

gboolean Controller::TextViewKeyPress(guint keyval) {
	g_debug("[%d] Controller::TextViewKeyPress keyval %x", mView.GetCurrentTabId(), keyval);
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_Paste:
		{
			GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			gchar *p = gtk_clipboard_wait_for_text(clipboard);
			if (p != nullptr) {
				// cout << "Pasted text: " << p << " key " << endl;
				mCurrentDoc = &mDocumentList[mView.nextId];
				g_debug("[%d] Controller::TextViewKeyPress new document %p", mView.GetCurrentTabId(), mCurrentDoc);
				unsigned size = strlen(p);
				mCurrentDoc->AddSourceText(p, size);
				g_free(p);
				mView.AddTab(mCurrentDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), true);
				mQueueReplace = true;
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
	mQueueReplace = true;
	return true; // Stop event from propagating
}

gboolean Controller::KeyEvent(GdkEvent *event) {
	g_debug("[%d] Controller::KeyEvent event type %d", mView.GetCurrentTabId(), event->type);
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
		if (mQueueReplace) {
			g_debug("[%d] Controller::Run queued replace", mView.GetCurrentTabId());
			mView.Replace(mCurrentDoc);
			mView.UpdateStatusBar(mCurrentDoc);
		} else if (mQueueAppend) {
			g_debug("[%d] Controller::Run queued append", mView.GetCurrentTabId());
			mView.Append(mCurrentDoc);
			mView.UpdateStatusBar(mCurrentDoc);
		}
		mQueueAppend = false;
		mQueueReplace = false;
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

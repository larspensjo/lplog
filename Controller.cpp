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

#include <string.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h> // Needed for GTK+-2.0
#include <string>

#include "Controller.h"
#include "Document.h"
#include "SaveFile.h"
#include "Defer.h"
#include "Debug.h"

static const std::string filePrefixURI = "file://";

// A key was pressed elsewhere than the tree view
static gboolean KeyPressedOther(GtkWidget *widget, GdkEvent *event, Controller *c) {
	return c->KeyPressedOther(widget, event);
}

gboolean Controller::KeyPressedOther(GtkWidget *widget, GdkEvent *event) {
	const std::string name = gtk_widget_get_name(widget);
	guint state = event->key.state;
	gint keyval = event->key.keyval;
	LPLOG("state 0x%x key 0x%x name %s", event->key.state, keyval, name.c_str());
	if (!(state & GDK_CONTROL_MASK) && !(state & GDK_SHIFT_MASK) && keyval == GDK_KEY_F3) {
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), 1);
		return true;
	}
	if (!(state & GDK_CONTROL_MASK) && (state & GDK_SHIFT_MASK) && keyval == GDK_KEY_F3) {
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), -1);
		return true;
	}
	if ((state & GDK_CONTROL_MASK) && keyval == GDK_KEY_F4) {
		mView.CloseCurrentTab();
		return true;
	}
	return false;
}

// A key was pressed in the tree view
static gboolean TreeViewKeyPressed(GtkWidget *, GdkEvent *event, Controller *c) {
	guint state = event->key.state;
	if (state & GDK_CONTROL_MASK)
		return false;
	LPLOG("state 0x%x", event->key.state);
	// GdkModifierType a;
	return c->TextViewKeyEvent(event);
}

static void ButtonClicked(GtkButton *button, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(button));
	LPLOG("%s", name.c_str());
	c->ExecuteCommand(name);
}

void Controller::ExecuteCommand(const std::string &name) {
	if (name == "quit")
		Quit();
	else if (name == "line")
		KeyPressed(GDK_KEY_o);
	else if (name == "remove")
		KeyPressed(GDK_KEY_Delete);
	else if (name == "child")
		KeyPressed(GDK_KEY_a);
	else if (name == "about")
		mView.About();
	else if (name == "help")
		Help();
	else if (name == "open")
		FileOpenDialog();
	else if (name == "find")
		mView.SetFocusFind();
	else if (name == "close")
		CloseCurrentTab();
	else if (name == "paste")
		TextViewKeyPress(GDK_KEY_Paste);
	else if (name == "patternstore") {
		SaveCurrentPattern(); // Ensure current pattern is in mSaveFile.
		bool changed = mView.DisplayPatternStore(mSaveFile);
		if (changed) {
			mView.DeSerialize(mSaveFile);
			mQueueReplace = true;
			mSaveFile.Write(); // Force early save, instead of waiting for application termination.
		}
	} else
		LPLOG("Unknown button: %s", name.c_str());
}

static gboolean TestForeChanges(Controller *c)
{
	c->PollInput();
	return true; // Keep timer going for ever
}

static void PatternCellUpdated(GtkCellRenderer *renderer, gchar *path, gchar *newString, Controller *c)
{
	c->PatternCellUpdated(renderer, path, newString);
}

// A key was pressed in the main text editor.
static gboolean TextViewKeyPress(GtkWidget *widget, GdkEvent *event, Controller *c) {
	gint keyval = event->key.keyval;
	LPLOG("%s: 0x%x modifier %d state 0x%x hardware_keycode 0x%x type %d",
			event->key.string, keyval, event->key.is_modifier, event->key.state, event->key.hardware_keycode, event->key.type);
	if (event->key.is_modifier)
		return false; // Ignore all SHIFT, CTRL, etc.
	if (!(event->key.state & GDK_CONTROL_MASK) && !(keyval >= GDK_KEY_F1 && keyval <= GDK_KEY_F35))
		return true; // Consume all normal characters, to prevent from being inserted
	else if ((event->key.state & GDK_CONTROL_MASK) && keyval == GDK_KEY_c)
		return false; // Copy

	if (!(event->key.state & GDK_CONTROL_MASK) && !(event->key.state & GDK_SHIFT_MASK) && event->key.keyval == GDK_KEY_F3)
		keyval = GDK_KEY_Next;
	else if (!(event->key.state & GDK_CONTROL_MASK) && (event->key.state & GDK_SHIFT_MASK) && event->key.keyval == GDK_KEY_F3)
		keyval = GDK_KEY_Prior;
	else {
		switch(keyval) {
		case GDK_KEY_v:
			keyval = GDK_KEY_Paste;
			break;
		case GDK_KEY_f:
			keyval = GDK_KEY_Find;
			break;
		default:
			return true; // Consume and ignore
		}
	}
	return c->TextViewKeyPress(keyval);
}

static void TogglePattern(GtkCellRendererToggle *renderer, gchar *path, Controller *c) {
	c->TogglePattern(renderer, path);
}

static void ToggleButton(GtkToggleButton *togglebutton, Controller *c) {
	std::string name = gtk_widget_get_name(GTK_WIDGET(togglebutton));
	c->ToggleButton(name);
}

static void DragDataReceived(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, Controller *c) {
	if (info != 0) {
		LPLOG("unknown info %d", info);
		return;
	}
	char **str = gtk_selection_data_get_uris(data);
	bool success = false;
	LPLOG("%s", str[0]);
	if (str != nullptr) {
		c->OpenURI(str[0]); // Only get the first reference for now
		g_strfreev(str);
		success = true;
	}
	gtk_drag_finish(context, success, false, time);
}

// Callback when another tab was selected.
static void ChangeCurrentPage(GtkNotebook *notebook, GtkWidget *page, gint tab, Controller *c) {
	GtkWidget *child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), tab);
	GtkWidget *labelWidget = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), child);
	const char *name = gtk_widget_get_name(labelWidget);
	LPLOG("tab %d widget %s", tab, name);
	c->ChangeDoc(atoi(name));
}

static gboolean DestroyMainWindow(GtkWidget *widget, Controller *c) {
	c->Quit();
	return false;
}

static void EditEntry(GtkEditable *editable, Controller *c) {
	LPLOG("");
	const std::string str = gtk_editable_get_chars(editable, 0, -1);
	c->Find(str);
}

void Controller::Find(const std::string &str) {
	if (mCurrentDoc == nullptr)
		return;
	LPLOG("[%d] '%s'", mView.GetCurrentTabId(), str.c_str());
	mCurrentDoc->ResetSearch();
	mView.FindNext(mCurrentDoc, str, 1);
}

void Controller::CloseCurrentTab() {
	int id = mView.GetCurrentTabId();
	if (id == -1)
		return;
	LPLOG("[%d] tab", id);
	if (mCurrentDoc == &mDocumentList[id])
		mCurrentDoc = nullptr;
	mDocumentList.erase(id);
	mView.CloseCurrentTab();
	mView.SetWindowTitle("");
	mView.UpdateStatusBar(nullptr);
}

void Controller::ChangeDoc(int id) {
	mCurrentDoc = &mDocumentList[id];
	LPLOG("[%d] doc (%p), lines %u", id, mCurrentDoc, mCurrentDoc->GetNumLines());
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
	LPLOG("[%d] %s new document %p", mView.GetCurrentTabId(), filename.c_str(), mCurrentDoc);
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
		LPLOG("[%d] new document %p for %s", mView.GetCurrentTabId(), newDoc, fn.c_str());
		newDoc->AddSourceFile(fn);
		mView.AddTab(newDoc, this, G_CALLBACK(::DragDataReceived), G_CALLBACK(::TextViewKeyPress), true);
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
	LPLOG("[%d] %s", mView.GetCurrentTabId(), path);
	mView.TogglePattern(path);
	// Inhibit update if root pattern is disabled
	if (mRootPatternDisabled && mView.RootPatternActive()) {
		LPLOG("[%d] Root pattern enabled", mView.GetCurrentTabId());
		mRootPatternDisabled = false;
	}
	if (!mRootPatternDisabled)
		mQueueReplace = true;
	if (!mView.RootPatternActive()) {
		LPLOG("[%d] Root pattern disabled", mView.GetCurrentTabId());
		mRootPatternDisabled = true;
	}
}

void Controller::ToggleButton(const std::string &name) {
	LPLOG("[%d] %s", mView.GetCurrentTabId(), name.c_str());
	if (name == "autoscroll")
		mView.UpdateStatusBar(mCurrentDoc); // This will use the new automatic scrolling
	else if (name == "casesensitive") {
		mView.FindSetCaseSensitive();
		if (mCurrentDoc) {
			mCurrentDoc->ResetSearch();
			mView.FindNext(mCurrentDoc, mView.GetSearchString(), 1);
		}
	} else if (name == "ignoreduplicates") {
		mView.ToggleIgnoreDuplicateLines();
		mQueueReplace = true;
	} else if (name == "findnext")
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), 1);
	else if (name == "findprev")
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), -1);
	else if (name == "linenumbers")
		mView.ToggleLineNumbers(mCurrentDoc);
	else
		LPLOG("[%d] unknown %s", mView.GetCurrentTabId(), name.c_str());
}

void Controller::PatternCellUpdated(GtkCellRenderer *renderer, gchar *path, gchar *newString) {
	mView.EditPattern(path, newString);
	// Inhibit update if root pattern is disabled
	if (!mRootPatternDisabled)
		mQueueReplace = true;
}

gboolean Controller::TextViewKeyPress(guint keyval) {
	LPLOG("[%d] keyval 0x%x", mView.GetCurrentTabId(), keyval);
	bool stopEvent = false;
	switch(keyval) {
	case GDK_KEY_Next:
		stopEvent = true;
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), 1);
		break;
	case GDK_KEY_Prior:
		stopEvent = true;
		mView.FindNext(mCurrentDoc, mView.GetSearchString(), -1);
		break;
	case GDK_KEY_Find:
		mView.SetFocusFind();
		stopEvent = true;
		break;
	case GDK_KEY_Paste:
		{
			GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			gchar *p = gtk_clipboard_wait_for_text(clipboard);
			Defer pFree([p](){if (p != nullptr) (g_free(p));});
			if (p != nullptr) {
				mCurrentDoc = &mDocumentList[mView.nextId];
				LPLOG("[%d] new document %p", mView.GetCurrentTabId(), mCurrentDoc);
				unsigned size = strlen(p);
				mCurrentDoc->AddSourceText(p, size);
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
		mView.OpenPatternForEditing();
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

gboolean Controller::TextViewKeyEvent(GdkEvent *event) {
	LPLOG("[%d] event type %d", mView.GetCurrentTabId(), event->type);
	return this->KeyPressed(event->key.keyval);
}

void Controller::Run(int argc, char *argv[], GdkPixbuf *icon) {
	mView.Create(icon, G_CALLBACK(::ButtonClicked), G_CALLBACK(::ToggleButton), G_CALLBACK(::TreeViewKeyPressed), G_CALLBACK(::KeyPressedOther), G_CALLBACK(::PatternCellUpdated),
				 G_CALLBACK(::TogglePattern), G_CALLBACK(::ChangeCurrentPage), G_CALLBACK(::DestroyMainWindow), G_CALLBACK(::EditEntry), this);
	mView.SetWindowTitle("");
	if (argc > 1) {
		this->OpenURI(filePrefixURI + argv[1]);
	}
	mView.DeSerialize(mSaveFile);
	g_timeout_add(mSaveFile.GetIntOption("PollPeriod", 1000), GSourceFunc(::TestForeChanges), this);
	while (!mQuitNow) {
		gtk_main_iteration();
		if (mQueueReplace && mCurrentDoc != nullptr) {
			LPLOG("[%d] queued replace", mView.GetCurrentTabId());
			mView.Replace(mCurrentDoc);
			mView.UpdateStatusBar(mCurrentDoc);
		} else if (mQueueAppend && mCurrentDoc != nullptr) {
			LPLOG("[%d] queued append", mView.GetCurrentTabId());
			mView.Append(mCurrentDoc);
			mView.UpdateStatusBar(mCurrentDoc);
		}
		mQueueAppend = false;
		mQueueReplace = false;
	}
	SaveCurrentPattern();
}

void Controller::SaveCurrentPattern() {
	std::stringstream ss;
	mView.Serialize(ss);
	auto current = mSaveFile.GetStringOption("CurrentPattern", "default"); // Find what current pattern name to use
	mSaveFile.SetPattern(current, ss.str());
}

void Controller::Help() const {
	const std::string msg =
		"LPlog\n"
		"\n"
		"Enter pattern in tree on the left side.\n"
		"Add additional patterns with '+' and\n"
		"new children with 'a'\n"
		"For example, the NOT operator '!' takes one child.\n";
	mView.Help(msg);
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

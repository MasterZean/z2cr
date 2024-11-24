#include "ZideWindow.h"

void ZideWindow::DoMainMenu(Bar& bar) {
	int i = tabs.GetCursor();
	bool tab = i != -1;
	
	bar.Add("File",       THISBACK(DoMenuFile));
	
	if (tab) {
		bar.Add("Edit",   THISBACK(DoMenuEdit));
		bar.Add("Format", THISBACK(DoMenuFormat));
		//bar.Add("Build",  THISBACK(DoMenuBuild));
	}
	
	bar.Add("Help",       THISBACK(DoMenuHelp));
}

void ZideWindow::DoMenuFile(Bar& bar) {
	int i = tabs.GetCursor();
	bool tab = i != -1;
	
	bar.Add("&New project", CtrlImg::new_doc(), Callback())
		.Key(K_CTRL | K_N).Enable(false);
	bar.Add("&New package", CtrlImg::MkDir(), Callback())
		.Key(K_CTRL | K_N).Enable(false);
	bar.Separator();
	
	bar.Add("&Open project", CtrlImg::open(), Callback())
		.Key(K_CTRL | K_O).Enable(false);
	bar.Add("&Open package", CtrlImg::open(), THISBACK(OnMenuFileLoadPackage))
		.Key(K_CTRL | K_O);
	bar.Add("&Open file",                    THISBACK(OnMenuFileLoadFile));
	bar.Add(tab, "&Save", CtrlImg::save(),   THISBACK(OnMenuFileSaveFile))
		.Key(K_CTRL | K_S);
	bar.Add("&Save all",                     THISBACK(OnMenuFileSaveAll));
	bar.Separator();
	
	if (RecentPackages.GetCount())
		bar.Add("&Recent packages",           THISBACK(DoMenuRecent));
	else
		bar.Add("&Recent packages",           Callback()).Enable(false);
	bar.Separator();
	
	bar.Add("Show package paths",             THISBACK(OnMenuShowPackagePaths))
		.Check(oShowPakPaths);
	bar.Separator();
	
	bar.Add("E&xit",                         THISBACK(Close))
		.Key(K_ALT | K_F4);
}

void ZideWindow::OnMenuFileLoadPackage() {
	FileSel fs;
	fs.PreSelect(LastPackage);
	
	if (fs.ExecuteSelectDir()) {
		if (!tabs.PromptSaves())
			return;
			
		LoadPackage(fs.Get());
	}
}

void ZideWindow::OnMenuFileLoadFile() {
	FileSel fs;
	fs.PreSelect(openDialogPreselect);
	if (fs.ExecuteOpen()) {
		openDialogPreselect = fs.Get();
		tabs.Open(openDialogPreselect);
	}
}

void ZideWindow::OnMenuFileSaveFile() {
	tabs.Save(tabs.GetCursor());
}

void ZideWindow::OnMenuFileSaveAll() {
	tabs.SaveAll();
}

void ZideWindow::OnMenuShowPackagePaths() {
	oShowPakPaths = !oShowPakPaths;
	asbAss.SetShowPaths(oShowPakPaths);
}

void ZideWindow::DoMenuRecent(Bar& bar) {
	int m = min(RecentPackages.GetCount(), 9);
	for (int i = 0; i < m; i++) {
		bar.Add("&" + IntStr(i + 1) + "       " + RecentPackages[i], THISBACK1(OoMenuRecent, RecentPackages[i]));
	}
}

void ZideWindow::OoMenuRecent(const String& path) {
	LoadPackage(path);
	
	int i = RecentPackages.Find(LastPackage);
	if (i == -1)
		RecentPackages.Insert(0, LastPackage);
	else {
		RecentPackages.Remove(i);
		RecentPackages.Insert(0, LastPackage);
	}
}

void ZideWindow::DoMenuEdit(Bar& bar) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	bool selection = editor.IsAnySelection();

	bar.Add("Undo", CtrlImg::undo(),             callback(&editor, &LineEdit::Undo))
		.Key(K_CTRL_Z)
		.Enable(editor.IsUndo())
		.Help("Undo changes to text");
	bar.Add("Redo", CtrlImg::redo(),             callback(&editor, &LineEdit::Redo))
		.Key(K_SHIFT|K_CTRL_Z)
		.Enable(editor.IsRedo())
		.Help("Redo undone changes");
	bar.Separator();
	
	bar.Add("Cut", CtrlImg::cut(),               callback(&editor, &LineEdit::Cut))
		.Key(K_CTRL_X)
		.Enable(selection)
		.Help("Cut selection and place it on the system clipboard");
	bar.Add("Copy", CtrlImg::copy(),             callback(&editor, &LineEdit::Copy))
		.Key(K_CTRL_C)
		.Enable(selection)
		.Help("Copy current selection on the system clipboard");
	bar.Add("Paste", CtrlImg::paste(),           callback(&editor, &LineEdit::Paste))
		.Key(K_CTRL_V)
		.Help("Insert text from clipboard at cursor location");
	bar.Separator();
	bar.Add("Select all", CtrlImg::select_all(), callback(&editor, &LineEdit::SelectAll))
		.Key(K_CTRL_A);
	bar.Separator();
	
	bar.Add("Go to line...",                     THISBACK1(OnMenuEditGoTo, &editor)).Key(K_CTRL | K_G);
	bar.Separator();
	
	bar.Add("Find...",                           callback3(&editor, &CodeEditor::FindReplace, false, false, false))
		.Help("Search for text or text pattern")
		.Key(K_CTRL | K_F);
	bar.Add("Replace...",                        callback3(&editor, &CodeEditor::FindReplace, false, false, true))
		.Help("Search for text or text pattern, with replace option")
		.Key(K_CTRL | K_H);
	bar.Add("Find next",                         callback(&editor, &CodeEditor::FindNext))
		.Help("Find next occurrence")
		.Key(K_F3);
	bar.Add("Find previous",                     callback(&editor, &CodeEditor::FindPrev))
		.Help("Find previous occurrence")
		.Key(K_SHIFT | K_F3);
}

void ZideWindow::OnMenuEditGoTo(CodeEditor* editor) {
	static int line = 1;
	if (EditNumber(line, "Go to line numer", "Line:")) {
		editor->GotoLine(line - 1);
		editor->SetFocus();
	}
}

void ZideWindow::DoMenuFormat(Bar& menu) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	//menu.Add("Settings...",         THISBACK(OnMenuFormatShowSettings));
	//menu.Separator();
	
	menu.Add("Spaces to tabs",      callback1(&editor, &CodeEditor::MakeTabsOrSpaces, true))
		.Help("Convert leading blanks on each line to tabs");
	menu.Add("Tabs to spaces",      callback1(&editor, &CodeEditor::MakeTabsOrSpaces, false))
		.Help("Convert all tabs to spaces");
	menu.Add("Trim line endings",   callback(&editor, &CodeEditor::MakeLineEnds))
		.Help("Remove tabs and spaces at line endings");
	menu.Add("Duplicate line",      callback(&editor, &CodeEditor::DuplicateLine))
	    .Help("Duplice the current line").Key(K_CTRL | K_D);
}

void ZideWindow::DoMenuHelp(Bar& bar) {
	//bar.Add("Export documentation", THISBACK(OnMenuHelpRebuildDocs));
	//bar.Add("Reload documentation", callback(&asbAss, &AssemblyBrowser::ReloadDocs));
	//bar.Separator();
	bar.Add("About", THISBACK(OnMenuHelpAbout));
}

void ZideWindow::OnMenuHelpAbout() {
	PromptOK("zide!!!!!!!!!!!!!!!!!!!!!!");
}


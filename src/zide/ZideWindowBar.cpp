#include "ZideWindow.h"
#include "BuildMethodsWindow.h"

LocalProcess globalExecutor;
void* globalProcesID;

void ExecutableThread(ZideWindow* zide, const String& file, bool newConsole) {
#ifdef PLATFORM_WIN32
	if (newConsole) {
		String command = "cmd.exe + /C \"" + file + "\"; & pause";
		int n = command.GetLength() + 1;
		Buffer<char> cmd(n);
		memcpy(cmd, command, n);
		
		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
		si.cb = sizeof(STARTUPINFO);
		
		if (CreateProcess(NULL, cmd, &sa, &sa, TRUE,
			             NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE,
		                NULL, NULL, &si, &pi)) {
		    globalProcesID = (void*)pi.hProcess;
		    
		    WaitForSingleObject(pi.hProcess, INFINITE);
		                    
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			
			globalProcesID = nullptr;
		}
		else
			;//PutConsole("Unable to launch " + String(_cmdline));
	}
	else {
		
#endif

		String t, tt;
		
		globalExecutor.Kill();
		globalExecutor.ConvertCharset(false);
		globalExecutor.Start(file);
		
		while (globalExecutor.Read(t)) {
			if (t.GetCount()) {
				PostCallback(callback1(zide, &ZideWindow::AddOutputLine, t));
				Sleep(5);
			}
		}
		
#ifdef PLATFORM_WIN32
	}
#endif
	
	PostCallback(callback(zide, &ZideWindow::OutPutEnd));
}

struct FormatDlg: TabDlg {
	ColorPusher hl_color[CodeEditor::HL_COUNT];
};

int AdjustForTabs(const String& text, int col, int tabSize) {
	int pos = 1;
	
	for (int i = 0; i < text.GetLength(); i++) {
		if (text[i] == '\t') {
			int newpos = (pos + tabSize - 1) / tabSize * tabSize + 1;
			col -= newpos - pos - 1;
			pos = newpos;
		}
		else
			pos++;
	}
	
	return col;
}

void ZideWindow::DoMainMenu(Bar& bar) {
	int i = tabs.GetCursor();
	bool tab = i != -1;
	
	bar.Add("File",       THISBACK(DoMenuFile));
	
	if (tab) {
		bar.Add("Edit",   THISBACK(DoMenuEdit));
		bar.Add("Format", THISBACK(DoMenuFormat));
		bar.Add("Build",  THISBACK(DoMenuBuild));
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
	
	menu.Add("Settings...",         THISBACK(OnMenuFormatShowSettings));
	menu.Separator();
	
	menu.Add("Spaces to tabs",      callback1(&editor, &CodeEditor::MakeTabsOrSpaces, true))
		.Help("Convert leading blanks on each line to tabs");
	menu.Add("Tabs to spaces",      callback1(&editor, &CodeEditor::MakeTabsOrSpaces, false))
		.Help("Convert all tabs to spaces");
	menu.Add("Trim line endings",   callback(&editor, &CodeEditor::MakeLineEnds))
		.Help("Remove tabs and spaces at line endings");
	menu.Add("Duplicate line",      callback(&editor, &CodeEditor::DuplicateLine))
	    .Help("Duplice the current line").Key(K_CTRL | K_D);
}

void ZideWindow::OnMenuFormatShowSettings() {
	Settings backSettings = settings;
	
	FormatDlg dlg;
	dlg.Title("Settings").Sizeable();
	dlg.SetMinSize(Size(602, 413));
	dlg.Cancel().Apply();
	
	WithSetupEditorLayout<ParentCtrl> edt;
	
	edt.filetabs
		.Add(AlignedFrame::LEFT, "Left")
		.Add(AlignedFrame::TOP, "Top")
		.Add(AlignedFrame::RIGHT, "Right")
		.Add(AlignedFrame::BOTTOM, "Bottom")
		.Add(-1, "Off");
		
	edt.tabs_crosses
		.Add(AlignedFrame::LEFT, "Left")
		.Add(AlignedFrame::RIGHT, "Right")
		.Add(-1, "Off");
		
	edt.tabsize.MinMax(1, 100).NotNull();
	edt.optScope.SetVertical();
	edt.optBracket.SetVertical();
	
	CtrlRetriever rtvr;
	rtvr
		(edt.showtabs, settings.ShowTabs)
		(edt.showspaces, settings.ShowSpaces)
		(edt.shownewlines, settings.ShowNewlines)
		(edt.warnwhitespace, settings.WarnSpaces)
		(edt.tabsize, settings.TabSize)
		(edt.indentspaces, settings.IndentSpaces)
		(edt.ShowLineNums, settings.ShowLineNums)
		(edt.HighlightLine, settings.HighlightLine)
		(edt.filetabs, settings.TabPos)
		(edt.tabs_crosses, settings.TabClose)
		(edt.LinePos, settings.LinePos)
		(edt.LineColor, settings.LineColor)
		(edt.optScope, settings.ScopeHighlight)
		(edt.optBracket, settings.Brackets)
		(edt.optThousands, settings.Thousands);
	rtvr <<= dlg.Breaker(222);
	
	dlg.Add(edt, "Editor");

	dlg.WhenClose = dlg.Acceptor(IDEXIT);
	
	int dialogAction = 0;
	
	for(;;) {
		dialogAction = dlg.Run();
		
		rtvr.Retrieve();
		
		tabs.SetTabSettings(settings);
		
		auto temp = GetEditor();
		if (temp) {
			CodeEditor& editor = *temp;
			String file = tabs.ActiveFile();
			
			EditorManager::SetSettings(editor, settings, EditorSyntax::GetSyntaxForFilename(file.ToString()));
		}
		
		if (dialogAction == IDYES || dialogAction == IDEXIT || dialogAction == IDCANCEL)
			break;
	}
	
	if (dialogAction == IDEXIT || dialogAction == IDCANCEL)
		settings = backSettings;
	
	tabs.SetSettings(settings);
}

void ZideWindow::DoMenuBuild(Bar& bar) {
	if (running) {
		bar.Add("Kill current process",  THISBACK(OnMenuBuildKill))
			.Key(K_CTRL | K_F5);
		bar.Separator();
	}
	
	bar.Add("Compile && run current file in console",  THISBACK1(OnMenuBuildRun, false))
		.Key(K_F5)
		.Enable(!running && canBuild);
	bar.Add("Compile && run current file in new console",  THISBACK1(OnMenuBuildRun, true))
		.Key(K_CTRL | K_F5)
		.Enable(!running && canBuild);
	bar.Separator();
	
	/*bar.Add("Compile current file", CtrlImg::Toggle(), THISBACK(OnMenuBuildBuild))
		.Key(K_F7)
		.Enable(!running && canBuild);
	bar.Add("Frontend only compile current file",      THISBACK(OnMenuBuildFrontend))
		.Key(K_F8)
		.Enable(!running && canBuild);
	bar.Separator();
	
	bar.Add("Mirror mode",                             THISBACK(OnMenuBuildMirror))
		.Check(mirrorMode);
	bar.Separator();*/
	
	bar.Add("Build methods...",                        THISBACK(OnMenuBuildMethods));
	bar.Separator();
	/*bar.Add("Optimize backend code",                   THISBACK(DoMenuOptimize));
	bar.Add("Library mode",                            THISBACK(OnMenuBuildLibMode))
		.Check(libMode);
	bar.Separator();*/
	
	bar.Add("Show log",                                THISBACK(OnMenuBuildShowLog))
		.Check(splBottom.IsShown());
}

void ZideWindow::OnMenuBuildKill() {
#ifdef PLATFORM_WIN32
	if (globalProcesID) {
		TerminateProcess((HANDLE)globalProcesID, -1);
		globalProcesID = nullptr;
	}
	else {
		globalExecutor.Kill();
	}
#else
	globalExecutor.Kill();
#endif
	
	console << "Process killed!";
	console.ScrollEnd();
	
	running = false;
}

void ZideWindow::OnMenuBuildMethods() {
	BuildMethodsWindow bmw(methods);
	bmw.Run(true);
}

void ZideWindow::OnMenuBuildRun(bool newConsole) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	String file = NativePath(tabs.ActiveFile());
	tabs.SaveAllIfNeeded();
	
	// TODO:
	//editor.ClearErrors();
	//editor.RefreshLayoutDeep();
	editor.Errors(Vector<Point>());
		
	splBottom.Show();

	bool res = false;
	String t, tt;
	console.Set(String());
	tt = Build(file, true, res);

	if (res) {
		String ename = GetFileDirectory(file) + GetFileTitle(file) + BuildMethod::ExeName("");
		
		if (!newConsole)
			console.Set(tt + "Running " + ename + "\n");
		else
			console.Set(tt);
		Title("ZIDE - Executing: " + ename);
		
		sw.Reset();
		running = true;
		
		Thread().Run(callback3(ExecutableThread, this, ename, newConsole));
	}
	else {
		console.Set(tt);
		console.ScrollEnd();
		
		int errors = 0;
		Vector<String> lines = Split(tt, '\n');
		Vector<Point> errorList;
		for (int i = 0; i < lines.GetCount(); i++) {
			String s = TrimBoth(lines[i]);
			
			int ii = s.Find('(');
			if (ii != -1) {
				int jj = s.Find(')', ii);
				if (ii < jj) {
					String path = s.Mid(0, ii);
					if (path == file) {
						String ll = s.Mid(ii + 1, jj - ii - 1);
						Vector<String> s2 = Split(ll, ',');
						if (s2.GetCount() == 2) {
							int line = StrInt(s2[0]) - 1;
							int col = StrInt(s2[1]) - 1;
							
							String text = editor.GetUtf8Line(line);
							col = AdjustForTabs(text, col, settings.TabSize);
							
							errorList.Add(Point(col, line));
							errors++;
						}
					}
				}
			}
		}
		
		if (errors) {
			//editor.Errors(std::move(errorList));
			//TODO:
			editor.RefreshLayoutDeep();
		}
	}
}

void ZideWindow::OnMenuBuildShowLog() {
	splBottom.Show(!splBottom.IsShown());
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


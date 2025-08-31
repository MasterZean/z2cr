#include "ZideWindow.h"
#include "BuildMethodsWindow.h"

LocalProcess globalExecutor;
void* globalProcesID;
bool doCapture = false;

void ExecutableThreadBuild(BuildData* data) {
	String command = data->cmd;
	DUMP(command);
	
	String t, tt;
	
	globalExecutor.Kill();
	globalExecutor.ConvertCharset(false);
	globalExecutor.Start(command);
	
	while (globalExecutor.Read(t)) {
		if (t.GetCount()) {
			DUMP(t);
			PostCallback(callback1(data->zide, &ZideWindow::AddOutputLine, t));
			tt << t;
			//Sleep(5);
		}
	}
	
	data->result = tt;
	data->runOk = BuildMethod::IsSuccessCode(globalExecutor.GetExitCode());
	
	PostCallback(callback1(data->zide, &ZideWindow::OnFinishedBuild, data));
}

void ExecutableThreadRun(ZideWindow* zide, const String& file, bool newConsole) {
	String command;
	bool res = false;
	
#ifdef PLATFORM_WIN32
	if (newConsole) {
		command = "cmd.exe /C cd \"" + GetFileDirectory(file) + "\" && \"" + file + "\" && pause";
		DUMP(command);
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
		command = "cmd.exe /C cd \"" + GetFileDirectory(file) + "\" && \"" + file + "\"";
		DUMP(command);
		
		String t;
		
		globalExecutor.Kill();
		globalExecutor.ConvertCharset(false);
		globalExecutor.Start(command);
		
		
		StopWatch sw;
		
		while (doCapture && globalExecutor.Read(t)) {
			if (t.GetCount()) {
				//DUMP(t.GetCount());
				GuiLock __;
				//PostCallback(callback1(zide, &ZideWindow::AddOutputLine, t));
				zide->AddOutputLine(t);
				//Sleep(1000);
			}
		}
		
		//GuiLock __;
		//zide->AddOutputLine(AsString(sw.Elapsed()));
		
		res = BuildMethod::IsSuccessCode(globalExecutor.GetExitCode());
#ifdef PLATFORM_WIN32
	}
#endif
	
	if (doCapture)
		PostCallback(callback1(zide, &ZideWindow::OutPutEnd, res));
}

struct FormatDlg: TabDlg {
	ColorPusher hl_color[CodeEditor::HL_COUNT];
};

void HlPusherFactory(One<Ctrl>& ctrl) {
	ctrl.Create<ColorPusher>().NotNull().Track();
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
	
	WithSetupHlLayout<ParentCtrl> hlt;
	
	hlt.arcStyle.AddColumn("Style");
	hlt.arcStyle.AddColumn("Color").Ctrls(HlPusherFactory);
	hlt.arcStyle.AddColumn("Bold").Ctrls<Option>();
	hlt.arcStyle.AddColumn("Italic").Ctrls<Option>();
	hlt.arcStyle.AddColumn("Underline").Ctrls<Option>();
	hlt.arcStyle.ColumnWidths("211 80 45 45 80");
	hlt.arcStyle.EvenRowColor().NoHorzGrid().SetLineCy(EditField::GetStdHeight() + 2);
	ReadHlStyles(hlt.arcStyle);
		
	hlt.lstLanguage.Add("Z2");
	hlt.lstLanguage.SetIndex(0);
	
	hlt.lstScheme.Add("Light");
	hlt.lstScheme.Add("Dark");
	hlt.lstScheme.SetIndex(IsDarkTheme()/*settings.Theme*/);
	hlt.lstScheme << dlg.Breaker(224);
		
	rtvr <<= dlg.Breaker(222);
	hlt.arcStyle.WhenCtrlsAction = dlg.Breaker(222);
	hlt.btnRestore <<= dlg.Breaker(223);
	
	dlg.Add(edt, "Editor");
	dlg.Add(hlt, "Syntax highlighting");

	dlg.WhenClose = dlg.Acceptor(IDEXIT);
	
	int dialogAction = 0;
	
	for(;;) {
		dialogAction = dlg.Run();
		
		auto temp = GetEditor();
		if (temp) {
			CodeEditor& editor = *temp;
		
			if (dialogAction == 223) {
				settings.Theme = hlt.lstScheme.GetIndex();
				tabs.SetColors(settings.Theme);
				ReadHlStyles(hlt.arcStyle);
			}
			else if (dialogAction == 224) {
				settings.Theme = hlt.lstScheme.GetIndex();
				tabs.SetColors(settings.Theme);
				ReadHlStyles(hlt.arcStyle);
			}
			
			for(int i = 0; i < colors.GetCount(); i++) {
				int j = colors[i];
				editor.SetHlStyle(j, hlt.arcStyle.Get(i, 1), hlt.arcStyle.Get(i, 2),
				                     hlt.arcStyle.Get(i, 3), hlt.arcStyle.Get(i, 4));
			}
			
			rtvr.Retrieve();
			
			tabs.SetTabSettings(settings);
			
			auto temp = GetEditor();
				
			String file = tabs.ActiveFile();
				
			EditorManager::SetSettings(editor, settings, EditorSyntax::GetSyntaxForFilename(file.ToString()));
		}
		
		if (dialogAction == IDYES || dialogAction == IDEXIT || dialogAction == IDCANCEL)
			break;
	}
	
	if (dialogAction == IDEXIT || dialogAction == IDCANCEL) {
		settings = backSettings;
		tabs.SetColors(settings.Theme);
		ReadHlStyles(hlt.arcStyle);
	}
	
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
		doCapture = false;
		//globalExecutor.Kill();
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

String ZideWindow::BuildCmd(const String& file, bool scu, bool& res, Point p) {
	String cmd = CompilerExe;
	if (cmd.GetCount() == 0)
		cmd = BuildMethod::ExeName("z2c");
	cmd << " -";
	if (scu)
		cmd << "scu ";
	else
		cmd << "c++ ";
	
	cmd << "-mainfile " << file << " ";
	//cmd << "-color qtf";
	if (optPackMode.Get() == true)
		cmd << "-pak " << LastPackage << " ";
	if (optimize == 2)
		cmd << " -O2";
	else if (optimize == 1)
		cmd << " -O1";
	else if (optimize == 0)
		cmd << " -Od";

	/*if (libMode)
		cmd << " -lib";*/
	
	if (popMethodList.GetCursor() != -1) {
		cmd << " -bm " << popMethodList.Get(popMethodList.GetCursor(), 0);
		cmd << " -arch " << arch;
	}
	
	if (lstBldConf.GetIndex() == 0)
		cmd << " -subsystem console";
	else if (lstBldConf.GetIndex() == 1)
		cmd << " -subsystem windows";
	//if (p.x > 0)
	//	cmd << " -acp " << p.x << " " << p.y;
	
	DUMP(cmd);
	
	return cmd;
}

void ZideWindow::OnMenuBuildRun(bool newConsole) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	String file = asbAss.MainFile.GetCount() ? asbAss.MainFile : NativePath(tabs.ActiveFile());
	tabs.SaveAllIfNeeded();
	
	tabs.ClearErrors();
	editor.RefreshLayoutDeep();
		
	splBottom.Show();

	bool res = false;
	String t, tt;
	console.Set(String());
	String cmd = BuildCmd(file, true, res);
	
	BuildData* data = new BuildData();
	data->zide = this;
	data->file = file;
	data->cmd = cmd;
	data->editor = &editor;
	data->newConsole = newConsole;
	
	Thread().Run(callback1(ExecutableThreadBuild, data));
}

void ZideWindow::OnFinishedBuild(BuildData* data) {
	if (data->runOk) {
		String ename = GetFileDirectory(data->file) + GetFileTitle(data->file) + BuildMethod::ExeName("");
		
		if (!data->newConsole)
			console.Append(/*data->result + */"Running " + ename + "\n");
		/*else
			console.Append(data->result);*/
		Title("ZIDE - Executing: " + ename);
		
		sw.Reset();
		running = true;
		doCapture = true;
		
		Thread().Run(callback3(ExecutableThreadRun, this, ename, data->newConsole));
	}
	else {
		tabs.SetErrors(data->result);
	}
	
	delete data;
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

void ZideWindow::MainToolbar(Bar& bar) {
	if (settings.ToolbarInRow)
		bar.Separator();
	
	bar.Add(optPackMode, HorzLayoutZoom(100));
	bar.Gap(4);
	bar.Add(mbtEntryPoint, HorzLayoutZoom(220));
	bar.Gap(4);
	bar.Add(lstBldConf, HorzLayoutZoom(120));
	bar.Gap(4);
	bar.Add(mbtBldMode, HorzLayoutZoom(180));
	bar.Gap(4);
	
	bar.Add("Override: Debug Optimization", ZImg::Od(), THISBACK(OnToolO0))
		.Check(optimize == 0);
	bar.Add("Override: Low Optimization", ZImg::O1(),   THISBACK(OnToolO1))
		.Check(optimize == 1);
	bar.Add("Override: High Optimization", ZImg::O2(),  THISBACK(OnToolO2))
		.Check(optimize == 2);
}

void ZideWindow::OnToolO0() {
	optimize = 0;
	tlbMain.Set(THISBACK(MainToolbar));
}

void ZideWindow::OnToolO1() {
	optimize = 1;
	tlbMain.Set(THISBACK(MainToolbar));
}

void ZideWindow::OnToolO2() {
	optimize = 2;
	tlbMain.Set(THISBACK(MainToolbar));
}

void ZideWindow::DropMethodList() {
	popMethodList.PopUp(&mbtBldMode);
}

void ZideWindow::DropTypeList() {
	popTypeList.PopUp(&mbtBldMode);
}

void ZideWindow::DropArchList() {
	popArchList.PopUp(&mbtBldMode);
}

void ZideWindow::OnSelectMethod() {
	String s;
	
	if (popMethodList.GetCursor() == -1)
		for (int i = 0; i < popMethodList.GetCount(); i++)
			if (popMethodList.Get(i, 0) == method) {
				popMethodList.SetCursor(i);
				break;
			}
	
	if (popMethodList.GetCursor() == -1 && popMethodList.GetCount())
		popMethodList.SetCursor(0);
	
	if (popMethodList.GetCursor() != -1) {
		method = popMethodList.Get(popMethodList.GetCursor(), 0);
		s << method;
	}
		
	Index<String> archs;
	int index = -1;
	bool x86 = false, x64 = false;
	for (int i = 0; i < methods.GetCount(); i++) {
		if (methods[i].Name == method) {
			archs.FindAdd(methods[i].Arch);
			index = i;
		}
	}
	
	String oldArch;
	if (popArchList.GetCursor() != -1)
		oldArch = popArchList.Get(popArchList.GetCursor(), 0);
	if (oldArch.GetCount() == 0)
		oldArch = arch;
		
	popArchList.Clear();
	
	if (index!= -1) {
		for (int i = 0; i < archs.GetCount(); i++) {
			popArchList.Add(archs[i]);
			if (oldArch.GetCount() && oldArch == archs[i])
				popArchList.SetCursor(popArchList.GetCount() - 1);
		}
	}
	
	if (popArchList.GetCursor() == -1 && popArchList.GetCount())
		popArchList.SetCursor(0);
	
	if (popArchList.GetCursor() != -1) {
		if (!s.IsEmpty())
			s << " ";
		arch = popArchList.Get(popArchList.GetCursor(), 0);
		s << arch;
	}
	else
		arch = "";
		
	int c = popTypeList.GetCursor();
	if (c != -1) {
		if (!s.IsEmpty())
			s << " ";
		s << popTypeList.Get(c, 0);
		if (c == 0)
			OnToolO0();
		else if (c == 2)
			OnToolO1();
		else if (c == 1)
			OnToolO2();
	}
	
	mbtBldMode.Set(s);
}

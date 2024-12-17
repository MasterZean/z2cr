#include "ZideWindow.h"

int ZideWindow::HIGHLIGHT_Z2;

ZideWindow::ZideWindow() {
	CtrlLayout(*this, "ZIDE");
	Sizeable().Zoomable().Icon(ZImg::zide());
	
	int r = HorzLayoutZoom(100);
	int l = HorzLayoutZoom(250);
	
	bool toolbar_in_row = true;
	mnuMain.Transparent();
	
	if (toolbar_in_row) {
		int tcy = mnuMain.GetStdHeight() + 2 * Zy(2);
		
		bararea.Add(mnuMain.LeftPos(0, l).VCenterPos(mnuMain.GetStdHeight()));
		bararea.Add(lblLine.RightPos(4, r).VSizePos(2, 3));
		lblLine.AddFrame(ThinInsetFrame());
		bararea.Height(tcy);
		
		AddFrame(bararea);
	}
	
	mnuMain.Set(THISBACK(DoMainMenu));
		
	Add(splAsbCanvas);
	splAsbCanvas.Horz(asbAss, canvas);
	splAsbCanvas.SetPos(1750);
	
	canvas.Add(tabs);
	tabs.SizePos();
	
	console.Height(Zy(150));
	canvas.AddFrame(splBottom.Bottom(console, Zy(150)));
	
	console.WhenLeft = THISBACK(OnOutputSel);
	
	splBottom.Hide();
	
	tabs.WhenTabChange = THISBACK(OnTabChange);
	tabs.WhenEditorCursor = THISBACK(OnEditorCursor);
	tabs.WhenEditorChange = THISBACK(OnEditorChange);
	
	asbAss.WhenSelectSource = THISBACK(OnSelectSource);
	asbAss.WhenFileRemoved = THISBACK(OnFileRemoved);
	asbAss.WhenFileSaved = THISBACK(OnFileSaved);
	asbAss.WhenRenameFiles = callback(&tabs, &EditorManager::OnRenameFiles);
	
	WhenClose = THISBACK(OnClose);
	
	lblLine.SetAlign(ALIGN_CENTER);
	
	tabs.ShowTabs(false);
	
	OrderColors();
}

void ZideWindow::OrderColors() {
	colors.Add(HighlightSetup::INK_NORMAL);
	colors.Add(HighlightSetup::PAPER_NORMAL);
	colors.Add(HighlightSetup::INK_SELECTED);
	colors.Add(HighlightSetup::PAPER_SELECTED);
	
	colors.Add(HighlightSetup::INK_KEYWORD);
	colors.Add(HighlightSetup::INK_UPP);
	colors.Add(HighlightSetup::INK_SLOT);
	
	colors.Add(HighlightSetup::INK_CONST_STRING);
	colors.Add(HighlightSetup::INK_CONST_STRINGOP);
	
	colors.Add(HighlightSetup::INK_CONST_INT);
	colors.Add(HighlightSetup::INK_CONST_HEX);
	colors.Add(HighlightSetup::INK_CONST_OCT);
	colors.Add(HighlightSetup::INK_CONST_BIN);
	colors.Add(HighlightSetup::INK_CONST_FLOAT);
	
	colors.Add(HighlightSetup::INK_OPERATOR);
		
	colors.Add(HighlightSetup::INK_COMMENT);
	colors.Add(HighlightSetup::INK_COMMENT_WORD);
	colors.Add(HighlightSetup::PAPER_COMMENT_WORD);
	
	colors.Add(HighlightSetup::INK_ERROR);
	colors.Add(HighlightSetup::INK_PAR0);
	colors.Add(HighlightSetup::INK_PAR1);
	colors.Add(HighlightSetup::INK_PAR2);
	colors.Add(HighlightSetup::INK_PAR3);
	
	colors.Add(HighlightSetup::PAPER_BRACKET0);
	colors.Add(HighlightSetup::PAPER_BRACKET);
	colors.Add(HighlightSetup::PAPER_BLOCK1);
	colors.Add(HighlightSetup::PAPER_BLOCK2);
	colors.Add(HighlightSetup::PAPER_BLOCK3);
	colors.Add(HighlightSetup::PAPER_BLOCK4);
	
	if (IsDarkTheme())
		DarkTheme();
	else
		WhiteTheme();
	
	//settings.Theme = IsDarkTheme();
};

void ZideWindow::Serialize(Stream& s) {
	int version = 1;
	s / version;

	if (s.IsLoading()) {
		bool maximized = true;
		int width = 800;
		int height = 600;
		s % maximized % width % height;
		if (maximized)
			Maximize(maximized);
		else
			HCenterPos(width, 0).VCenterPos(height, 0);
	}
	else {
		bool maximized = IsMaximized();
		int width = GetSize().cx;
		int height = GetSize().cy;
		s % maximized % width % height;
	}
	
	s % LastPackage % RecentPackages % openNodes % ActiveFile % openDialogPreselect;
	s % settings % oShowPakPaths;
}

void ZideWindow::LoadPackage(const String& package) {
	if (package.GetCount() == 0)
		return;
	
#ifdef PLATFORM_WIN32
	String platform = "WIN32";
	String platformLib = "microsoft.windows";
#endif

			
#ifdef PLATFORM_POSIX
	String platform = "POSIX";
	String platformLib = "ieee.posix";
#endif

	LastPackage = package;
	asbAss.ClearModules();
	packages << LastPackage;
	LoadModule(LastPackage, 0);

	String pak = NativePath(CurFolder + "source/stdlib/sys.core");
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 1);
	}
	pak = NativePath(CurFolder + "source/stdlib/bind.c");
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 1);
	}
	pak = NativePath(CurFolder + "source/stdlib/" + platformLib);
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 2);
	}
	
	openNodes.Add(LastPackage);
	SetupLast();
	
	int i = RecentPackages.Find(LastPackage);
	if (i == -1)
		RecentPackages.Insert(0, LastPackage);
	else {
		RecentPackages.Remove(i);
		RecentPackages.Insert(0, LastPackage);
	}
}

void ZideWindow::LoadModule(const String& mod, int color) {
	asbAss.AddModule(NativePath(mod), color);
}

void ZideWindow::SetupLast() {
	asbAss.OpenNodes(openNodes);
	asbAss.HighlightFile(ActiveFile);
	
	tabs.SetSettings(settings);
	
	tabs.ShowTabs(tabs.GetCount());
	asbAss.SetShowPaths(oShowPakPaths);
}

void ZideWindow::OnFileRemoved(const String& file) {
	tabs.RemoveFile(file);
	mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnSelectSource() {
	ActiveFile = asbAss.GetCursorItem();
		
	tabs.Open(ActiveFile);
	mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnFileSaved(const String& file) {
	tabs.Save(file);
}

void ZideWindow::OnTabChange() {
	ActiveFile = tabs.ActiveFile();
	asbAss.HighlightFile(ActiveFile);
	
	tabs.ShowTabs(tabs.GetCount());
	//splExplore.Show(tabs.tabFiles.GetCount());
}

void ZideWindow::OnEditorChange() {
	SmartEditor* editor = tabs.GetEditor();
	if (!editor)
		return;
	
	editor->Hash++;
	//if (editThread)
	//	Thread().Run(callback3(OutlineThread, this, editor.Get(), info->Hash));
}

void ZideWindow::OnEditorCursor() {
	SmartEditor* editor = tabs.GetEditor();
	if (!editor)
		return;
	
	Point p = editor->GetColumnLine(editor->GetCursor());
	lblLine.SetText(String().Cat() << "Ln " << (p.y + 1) << ", Cl " << (p.x + 1));
}

void ZideWindow::OnClose() {
	while (Thread::GetCount()) {
		Sleep(10);
	}
	
	if (tabs.PromptSaves())
		Close();
}

String ZideWindow::Build(const String& file, bool scu, bool& res, Point p) {
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
	/*cmd << "-pak " << lastPackage << " ";
	if (optimize == 2)
		cmd << " -O2";
	else if (optimize == 1)
		cmd << " -O1";
	else if (optimize == 0)
		cmd << " -Od";

	if (libMode)
		cmd << " -lib";
	
	if (popMethodList.GetCursor() != -1)
		cmd << " -bm " << popMethodList.Get(popMethodList.GetCursor(), 0);
	
	cmd << " -arch " << arch;*/
	
	//if (p.x > 0)
	//	cmd << " -acp " << p.x << " " << p.y;
	
	DUMP(cmd);
	String t, tt;
	LocalProcess lp(cmd);

	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	res = BuildMethod::IsSuccessCode(lp.GetExitCode());

	if (res == false && tt.GetCount() == 0) {
		DUMP(tt);
		cmd = GetFileDirectory(GetExeFilePath()) + BuildMethod::ExeName("z2c");

		if (!FileExists(cmd))
			tt = "Could not find: " + cmd;
	}
	
	return tt;
}

void ZideWindow::AddOutputLine(const String& str) {
	console << str;
	console.ScrollEnd();
}

bool ZideWindow::IsVerbose() const {
	return console.VerboseBuild;
}

void ZideWindow::PutConsole(const char *s) {
	console << s << "\n";
}

void ZideWindow::PutVerbose(const char *s) {
	if(console.VerboseBuild) {
		PutConsole(s);
		console.Sync();
	}
}

void ZideWindow::OutPutEnd() {
	//console.ScrollEnd();
	console.ScrollLineUp();
	console.ScrollLineUp();
	Title("ZIDE - Execution done in " + sw.ToString() + " sec.");
	running = false;
}

void ZideWindow::OnOutputSel() {
	if (console.GetSelection().GetLength() != 0)
		return;
	
	Point p = console.GetColumnLine(console.GetCursor());
	
	for (int i = 0; i < 5; i++) {
		if (p.y >= 0 &&	GetLineOfError(p.y))
			return;
		
		p.y--;
	}
}

bool ZideWindow::GetLineOfError(int ln) {
	auto temp = GetEditor();
	if (!temp)
		return false;
	
	CodeEditor& editor = *temp;
	
	String line = console.GetUtf8Line(ln);
	
//#ifdef PLATFORM_WIN32
	int s = line.Find("(");
	int e = line.Find(")");
//#endif

/*#ifdef PLATFORM_POSIX
	int s = line.Find(":");
	int e = line.Find(": error: ");
#endif*/
	
	if (s > -1 && s < e) {
		String file = TrimLeft(line.Left(s));

		if (FileExists(file)) {
			tabs.Open(file);
			
			String rest = line.Mid(s + 1, e - s - 1);
			Vector<String> v = Split(rest, ",");
			if (v.GetCount() == 2) {
				int x = StrInt(TrimBoth(v[0])) - 1;
				int y = StrInt(TrimBoth(v[1])) - 1;
				
				editor.SetCursor(editor.GetGPos(x, y));
				editor.SetFocus();
				
				return true;
			}
			else if (v.GetCount() == 1) {
				int x = StrInt(TrimBoth(v[0])) - 1;
				
				editor.SetCursor(editor.GetGPos(x, 1));
				editor.SetFocus();
			
				return true;
			}
			
			return false;
		}
	}
	
	return false;
}

void ZideWindow::ReadHlStyles(ArrayCtrl& hlstyle) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	hlstyle.Clear();
	for(int i = 0; i < colors.GetCount(); i++) {
		const HlStyle& s = editor.GetHlStyle(colors[i]);
		hlstyle.Add(GetHlName(colors[i], editor), s.color, s.bold, s.italic, s.underline);
	}
}

const char* ZideWindow::GetHlName(int i, CodeEditor& editor) {
	if (i == HighlightSetup::INK_UPP)
		return "Class name text";
	else
		return editor.GetHlName(i);
}

bool ZideWindow::LoadMethods() {
	LoadFromXMLFile(methods, CurFolder + "buildMethods.xml");
	return methods.GetCount() != 0;
}

void ZideWindow::DetectMehods() {
	methods.Clear();
	Cout() << "No cached build method found! Trying to auto-detect...\n";
	BuildMethod::Get(methods);
	if (methods.GetCount() == 0) {
		PromptOK("Could not find any build methods. Building is dissabled!");
		canBuild = false;
	}
	StoreAsXMLFile(methods, "methods", CurFolder + "buildMethods.xml");
}

bool runProg = false;

void ProgThread(DetectWindow* detect) {
	while (runProg) {
		PostCallback(callback(detect, &DetectWindow::OnProgress));
		Sleep(100);
	}
}

void DetectThread(DetectWindow* detect, ZideWindow* zide) {
	zide->DetectMehods();
	PostCallback(callback(detect, &DetectWindow::OnDone));
}

void DetectWindow::Activate() {
	TopWindow::Activate();
	if (ZIDE) {
		runProg = true;
		prog.Run(callback1(ProgThread, this));
		Thread().Run(callback2(DetectThread, this, ZIDE));
	}
}

void DetectWindow::OnDone() {
	runProg = false;
	Close();
}
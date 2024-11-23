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
	
	tabs.WhenTabChange = THISBACK(OnTabChange);
	tabs.WhenEditorCursor = THISBACK(OnEditorCursor);
	tabs.WhenEditorChange = THISBACK(OnEditorChange);
	
	asbAss.WhenSelectSource = THISBACK(OnSelectSource);
	asbAss.WhenFileRemoved = THISBACK(OnFileRemoved);
	asbAss.WhenFileSaved = THISBACK(OnFileSaved);
	asbAss.WhenRenameFiles = callback(&tabs, &EditorManager::OnRenameFiles);
	
	WhenClose = THISBACK(OnClose);
	
	tabs.ShowTabs(false);
}

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
	
	s % LastPackage % RecentPackages % openNodes % activeFile % openDialogPreselect;
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
	asbAss.HighlightFile(activeFile);
	
	tabs.SetSettings(settings);
	
	tabs.ShowTabs(tabs.GetCount());
	asbAss.SetShowPaths(oShowPakPaths);
}

void ZideWindow::OnFileRemoved(const String& file) {
	//tabs.RemoveFile(file);
	//mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnSelectSource() {
	activeFile = asbAss.GetCursorItem();
		
	tabs.Open(activeFile);
	//mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnFileSaved(const String& file) {
	/*int i = tabs.tabFiles.FindKey(file);
	if (i != -1)
		tabs.Save(i);*/
}

void ZideWindow::OnTabChange() {
	activeFile = tabs.ActiveFile();
	asbAss.HighlightFile(activeFile);
	
	tabs.ShowTabs(tabs.GetCount());
	//splExplore.Show(tabs.tabFiles.GetCount());
}

void ZideWindow::OnEditorChange() {
	OpenFileInfo* info = tabs.GetInfo();
	if (!info)
		return;
	
	SmartEditor& editor = info->editor;
	if (!editor.IsEnabled())
		return;
	
	info->Hash++;
	//if (editThread)
	//	Thread().Run(callback3(OutlineThread, this, editor.Get(), info->Hash));
}

void ZideWindow::OnEditorCursor() {
	OpenFileInfo* info = tabs.GetInfo();
	if (!info)
		return;
	
	CodeEditor& editor = info->editor;
	if (!editor.IsEnabled())
		return;
	
	Point p = editor.GetColumnLine(editor.GetCursor());
	lblLine.SetText(String().Cat() << "Ln " << (p.y + 1) << ", Cl " << (p.x + 1));
}

void ZideWindow::OnClose() {
	while (Thread::GetCount()) {
		Sleep(10);
	}
	
	if (tabs.PromptSaves())
		Close();
}

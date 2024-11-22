#include "ZideWindow.h"

ZideWindow::ZideWindow() {
	CtrlLayout(*this, "ZIDE");
	Sizeable().Zoomable().Icon(ZImg::zide());
	
	int r = HorzLayoutZoom(100);
	int l = HorzLayoutZoom(250);
	
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
	asbAss.WhenRenameFiles = THISBACK(OnRenameFiles);
	
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
	
	s % LastPackage % recentPackages % openNodes % activeFile;
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
	
	int i = recentPackages.Find(LastPackage);
	if (i == -1)
		recentPackages.Insert(0, LastPackage);
	else {
		recentPackages.Remove(i);
		recentPackages.Insert(0, LastPackage);
	}
}

void ZideWindow::LoadModule(const String& mod, int color) {
	asbAss.AddModule(NativePath(mod), color);
}

void ZideWindow::SetupLast() {
	asbAss.OpenNodes(openNodes);
	asbAss.HighlightFile(activeFile);
	
	tabs.ShowTabs(tabs.GetCount());
}

bool ZideWindow::OnRenameFiles(const Vector<String>& files, const String& oldPath, const String& newPath) {
	/*for (int i = 0; i < files.GetCount(); i++) {
		int j = tabs.tabFiles.FindKey(files[i].ToWString());
		if (j != -1) {
			if (tabs.IsChanged(j)) {
				tabs.Save(j);
			}
		}
	}*/
	
	if (FileMove(oldPath, newPath))
		for (int i = 0; i < files.GetCount(); i++) {
			/*int j = tabs.tabFiles.FindKey(files[i].ToWString());
			if (j != -1) {
				ASSERT(files[i].StartsWith(oldPath));
				String np = newPath + files[i].Mid(oldPath.GetLength());
				WString w1 = tabs.tabFiles.GetKey(j);
				WString w2 = np.ToWString();
				int k = tabs.files.Find(w1);
				ASSERT(k != -1);
				OpenFileInfo* info = tabs.files.Detach(k);
				tabs.files.Add(w2);
				tabs.files.Set(tabs.files.GetCount() - 1, info);
				DUMP(w1);
				DUMP(w2);
				tabs.tabFiles.RenameFile(w1, w2, ZImg::zsrc);
			}*/
		}
		
	return true;
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
	//lblLine.SetText(String().Cat() << "Ln " << (p.y + 1) << ", Cl " << (p.x + 1));
}
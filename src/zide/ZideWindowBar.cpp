#include "ZideWindow.h"

void ZideWindow::DoMainMenu(Bar& bar) {
	int i = tabs.GetCursor();
	bool tab = i != -1;
	
	bar.Add("File",       THISBACK(DoMenuFile));
	
	/*if (tab) {
		bar.Add("Edit",   THISBACK(DoMenuEdit));
		bar.Add("Format", THISBACK(DoMenuFormat));
		bar.Add("Build",  THISBACK(DoMenuBuild));
	}*/
	
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

void ZideWindow::DoMenuHelp(Bar& bar) {
	//bar.Add("Export documentation", THISBACK(OnMenuHelpRebuildDocs));
	//bar.Add("Reload documentation", callback(&asbAss, &AssemblyBrowser::ReloadDocs));
	//bar.Separator();
	bar.Add("About", THISBACK(OnMenuHelpAbout));
}

void ZideWindow::OnMenuHelpAbout() {
	PromptOK("zide!!!!!!!!!!!!!!!!!!!!!!");
}

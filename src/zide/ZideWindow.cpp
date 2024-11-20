#include "ZideWindow.h"

#define IMAGECLASS ZImg
#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_source.h>

static String ExeName(const String& exe) {
#ifdef PLATFORM_WIN32
	return exe + ".exe";
#endif
#ifdef PLATFORM_POSIX
	return exe;
#endif
}

AssemblyBrowser::AssemblyBrowser() {
	AddFrame(frame.Height(2));
	Add(treModules);

	treModules.HSizePos().VSizePos();
	treModules.RenderMultiRoot().EmptyNodeIcon(CtrlImg::cross());
	treModules.NoRoot();
	treModules.NoWantFocus();
	treModules.Open(0);
}

int AssemblyBrowser::AddModule(const String& aModule, int color) {
	int i = modules.Find(aModule);
	
	if (i == -1) {
		modules.Add(aModule);
		if (color == 0)
			return AddModule(0, aModule, aModule, CtrlImg::Dir());
		else if (color == 1)
			return AddModule(0, aModule, aModule, ZImg::libDirUsed());
		else if (color == 2)
			return AddModule(0, aModule, aModule, ZImg::libDirNotUsed());
	}
	
	return i;
}

int AssemblyBrowser::AddModule(int parent, const String& path, const String& ppath, const Image& img) {
	int item = -1;
	if (path != ppath && path.Find(path) == 0)
		item = treModules.Add(parent, img, path, path.Mid(ppath.GetLength() + 1));
	else {
		String module = GetFileName(path);
		String modPath = GetFileDirectory(path);
		
		if (showPaths) {
			String v = module;
			v << " (" << modPath << ")";
			item = treModules.Add(parent, img, path, v);
		}
		else
			item = treModules.Add(parent, img, path, module);
	}

	FindFile ff;
	ff.Search(path + "/*");

	Vector<String> folders;
	Vector<String> files;
	
	while (ff) {
		if (ff.IsFolder()) {
			if (ff.GetName() != "." && ff.GetName() != "..")
				folders << ff.GetPath();
		}
		else
			files << ff.GetPath();
		
		ff.Next();
	}

	Sort(folders);
	Sort(files);
	
	for (int i = 0; i < folders.GetCount(); i++)
		AddModule(item, folders[i], path, CtrlImg::Dir());
	
	for (int i = 0; i < files.GetCount(); i++) {
		String name = GetFileName(files[i]);
		String ext = GetFileExt(name);

		if (ext == ".z2") {
			//Cache.FindAdd(files[i], LoadFile(files[i]));
			treModules.Add(item, ZImg::zsrc, files[i], name);
		}
		/*else if (name.EndsWith(".api.md")) {
			openDocs << files[i];
			AddDocFile(files[i]);
		}*/
	}

	return item;
}

ZideWindow::ZideWindow() {
	CtrlLayout(*this, "ZIDE");
	Sizeable().Zoomable().Icon(ZImg::zide());
	
	int r = HorzLayoutZoom(100);
	int l = HorzLayoutZoom(250);
	
	Add(splAsbCanvas);
	splAsbCanvas.Horz(asbAss, canvas);
	splAsbCanvas.SetPos(1750);
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
	
	s % LastPackage % recentPackages;
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
	//asbAss.ClearModules();
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
	
	//openNodes.Add(lastPackage);
	//SetupLast();
	
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

GUI_APP_MAIN {
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	
	Ctrl::SetAppName("ZIDE");
	ZideWindow& zide = *(new ZideWindow());
	
	zide.CurFolder = GetFileDirectory(GetExeFilePath());
	
	FindFile ff(NativePath(zide.CurFolder + "/" + ExeName("z2c")));
	if (ff.IsExecutable())
		zide.CompilerExe = ff.GetPath();

	if (!LoadFromFile(zide)) {
		zide.LastPackage = NativePath("c:/temp/test.pak");
	}

	zide.LoadPackage(zide.LastPackage);
	
	zide.Run();
	
	StoreToFile(zide);
	
	delete &zide;
}

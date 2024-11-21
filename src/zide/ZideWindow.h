#ifndef _zide_zide_h
#define _zide_zide_h

#include "zide.h"

#include "AssemblyBrowser.h"

class ZideWindow : public WithzideLayout<TopWindow> {
public:
	typedef ZideWindow CLASSNAME;
	
	String CurFolder;
	String CompilerExe;
	String LastPackage;
	
	ZideWindow();
	
	void Serialize(Stream& s);
	
	void LoadPackage(const String& package);
	void LoadModule(const String& mod, int color);
	
private:
	Vector<String> packages;
	Index<String> recentPackages;
	Vector<String> openNodes;
	String activeFile;
	
	Splitter splAsbCanvas;
	AssemblyBrowser asbAss;
	ParentCtrl canvas;
	
	void OnSelectSource();
	
	bool OnRenameFiles(const Vector<String>& files, const String& oldPath, const String& newPath);
	void OnFileRemoved(const String& file);
	void OnFileSaved(const String& file);
	
	void SetupLast();
};

#endif

#ifndef _zide_zide_h
#define _zide_zide_h

#include "zide.h"

#include "AssemblyBrowser.h"
#include "EditorManager.h"

class ZideWindow : public WithzideLayout<TopWindow> {
public:
	typedef ZideWindow CLASSNAME;
	
	static int HIGHLIGHT_Z2;
	
	String CurFolder;
	String CompilerExe;
	String LastPackage;
	
	Settings settings;
	
	ZideWindow();
	
	void Serialize(Stream& s);
	
	void LoadPackage(const String& package);
	void LoadModule(const String& mod, int color);
	
private:
	Vector<String> packages;
	Index<String> recentPackages;
	Vector<String> openNodes;
	String activeFile;
	EditorManager tabs;
	
	Splitter splAsbCanvas;
	AssemblyBrowser asbAss;
	ParentCtrl canvas;
	
	void OnTabChange();
	void OnSelectSource();
	void OnEditorChange();
	void OnEditorCursor();
	
	bool OnRenameFiles(const Vector<String>& files, const String& oldPath, const String& newPath);
	void OnFileRemoved(const String& file);
	void OnFileSaved(const String& file);
	
	void OnClose();
		
	void SetupLast();
};

#endif

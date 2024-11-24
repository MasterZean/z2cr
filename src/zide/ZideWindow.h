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
	Index<String> RecentPackages;
	
	Settings settings;
	
	ZideWindow();
	
	void Serialize(Stream& s);
	
	void LoadPackage(const String& package);
	void LoadModule(const String& mod, int color);
	
	SmartEditor* GetEditor() {
		return tabs.GetEditor();
	}
	
private:
	Vector<String> packages;
	Vector<String> openNodes;
	String activeFile;
	EditorManager tabs;
	String openDialogPreselect;
	bool oShowPakPaths = true;
	
	FrameTop<StaticBarArea> bararea;
	MenuBar mnuMain;
	Label lblLine;
		
	Splitter splAsbCanvas;
	AssemblyBrowser asbAss;
	ParentCtrl canvas;
	
	void OnTabChange();
	void OnSelectSource();
	void OnEditorChange();
	void OnEditorCursor();
	
	void OnFileRemoved(const String& file);
	void OnFileSaved(const String& file);
	
	void OnClose();
		
	void SetupLast();
	
	void DoMainMenu(Bar& bar);
	
	void DoMenuFile(Bar& bar);
	void OnMenuFileLoadPackage();
	void OnMenuFileLoadFile();
	void OnMenuFileSaveFile();
	void OnMenuFileSaveAll();
	void OnMenuShowPackagePaths();
	void DoMenuRecent(Bar& bar);
	void OoMenuRecent(const String& path);
	
	void DoMenuEdit(Bar& bar);
	void OnMenuEditGoTo(CodeEditor* editor);
	
	void DoMenuFormat(Bar& menu);
	
	void DoMenuHelp(Bar& bar);
	void OnMenuHelpAbout();
};

#endif

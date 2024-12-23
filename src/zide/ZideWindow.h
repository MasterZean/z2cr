#ifndef _zide_zide_h
#define _zide_zide_h

#include "zide.h"

#include "AssemblyBrowser.h"
#include "EditorManager.h"

#include <z2crlib/StopWatch.h>
#include <z2crlib/BuildMethod.h>

class Console: public LineEdit {
public:
	bool VerboseBuild = false;

	Console() {
		SetReadOnly();
	}

	Callback WhenLeft;

	virtual void LeftUp(Point p, dword flags) {
		LineEdit::LeftUp(p, flags);
		WhenLeft();
	}
	
	void ScrollLineUp() {
		sb.LineUp();
	}
	
	Console& operator<<(const String& s) {
		Append(s);
		return *this;
	}
	
	void Append(const String& s) {
		Insert(total, s);
	}
};

class ZideWindow;

class BuildData {
public:
	ZideWindow* zide = nullptr;
	CodeEditor* editor = nullptr;
	String file;
	String cmd;
	String result;
	bool newConsole = false;
	bool runOk = false;
};

class ZideWindow : public WithZideLayout<TopWindow> {
public:
	typedef ZideWindow CLASSNAME;
	
	static int HIGHLIGHT_Z2;
	
	String CurFolder;
	String CompilerExe;
	String LastPackage;
	Index<String> RecentPackages;
	String ActiveFile;
	
	Settings settings;
	
	ZideWindow();
	
	void Serialize(Stream& s);
	
	void LoadPackage(const String& package);
	void LoadModule(const String& mod, int color);
	
	SmartEditor* GetEditor() {
		return tabs.GetEditor();
	}
	
	void AddOutputLine(const String& str);
	bool IsVerbose() const;
	void PutConsole(const char *s);
	void PutVerbose(const char *s);
	void OutPutEnd(bool result);
	
	String BuildCmd(const String& file, bool scu, bool& res, Point p = Point(-1, -1));
	void OnFinishedBuild(BuildData* data);
	
	bool LoadMethods();
	void DetectMehods();
	void PopulateMehods();
	
private:
	Vector<String> packages;
	Vector<String> openNodes;
	EditorManager tabs;
	String openDialogPreselect;
	bool oShowPakPaths = true;
	Vector<BuildMethod> methods;
	String method;
	String arch;
	StopWatch sw;
	
	FrameTop<StaticBarArea> bararea;
	MenuBar mnuMain;

	Label lblLine;
		
	Splitter splAsbCanvas;
	AssemblyBrowser asbAss;
	ParentCtrl canvas;
	
	SplitterFrame splBottom;
	FrameBottom<Console> console;
	
	ToolBar tlbMain;
	MultiButton mbtEntryPoint;
	DropList lstBldConf;
	MultiButton mbtBldMode;
	
	PopUpTable popMethodList;
	PopUpTable popTypeList;
	PopUpTable popArchList;
	
	Vector<int> colors;
	int optimize = 0;
	
	bool running = false;
	bool canBuild = true;
	
	void OnOutputSel();
	bool GetLineOfError(int ln);
	void ReadHlStyles(ArrayCtrl& hlstyle);
	void OrderColors();
	const char *GetHlName(int i, CodeEditor& editor);
	
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
	void OnMenuFormatShowSettings();
	
	void DoMenuBuild(Bar& bar);
	void OnMenuBuildKill();
	void OnMenuBuildRun(bool newConsole);
	void OnMenuBuildMethods();
	void OnMenuBuildShowLog();
	
	void DoMenuHelp(Bar& bar);
	void OnMenuHelpAbout();
	
	void MainToolbar(Bar& bar);

	void OnToolO0();
	void OnToolO1();
	void OnToolO2();
	
	void DropMethodList();
	void DropTypeList();
	void DropArchList();
	
	void OnSelectMethod();
};

class DetectWindow: public WithWaitingLayout<TopWindow> {
public:
	ZideWindow* ZIDE = nullptr;
	Thread prog;
	
	DetectWindow() {
		CtrlLayout(*this, "Detecting build methods...");
		Icon(ZImg::zide());
		
		prgDetect.SetTotal(0);
		prgDetect.Set(0);
	}
	
	void OnProgress() {
		prgDetect.Set(prgDetect.Get() + 5, 0);
	}
	
	void OnDone();
	
	virtual void Activate();
};

void ExecutableThreadBuild(BuildData* data);
void ExecutableThreadRun(ZideWindow* zide, const String& file, bool newConsole);

#endif

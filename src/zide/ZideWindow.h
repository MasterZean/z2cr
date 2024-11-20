#ifndef _zide_zide_h
#define _zide_zide_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <zide/zide.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS ZImg
#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_header.h>

class AssemblyBrowser: public ParentCtrl {
public:
	AssemblyBrowser();

	int AddModule(const String& aModule, int color);
	
	void SetShowPaths(bool show);
	
private:
	FrameTop<ParentCtrl> frame;
	TreeCtrl treModules;
	
	Index<String> modules;
	
	bool showPaths = false;
	
	int AddModule(int parent, const String& path, const String& ppath, const Image& img);
};

class ZideWindow : public WithzideLayout<TopWindow> {
public:
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
	
	Splitter splAsbCanvas;
	AssemblyBrowser asbAss;
	ParentCtrl canvas;
};

#endif

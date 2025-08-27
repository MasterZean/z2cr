#ifndef _zide_AssemblyBrowser_h_
#define _zide_AssemblyBrowser_h_

#include "zide.h"

class DocEntry {
public:
	String Brief;
	WithDeepCopy<VectorMap<String, String>> Params;
	String Returns;
	String SeeAlso;
	String Code;
	String Key;
	WithDeepCopy<Index<String>> Links;
};

class AssemblyBrowser: public ParentCtrl {
public:
	typedef AssemblyBrowser CLASSNAME;
	
	Callback WhenSelectSource;
	Callback1<const String&> WhenFileRemoved;
	Callback1<const String&> WhenFileSaved;
	Gate3<const Vector<String>&, const String&, const String&> WhenRenameFiles;
	
	ArrayMap<String, DocEntry> Docs;
	
	AssemblyBrowser();

	void ClearModules();
	int AddModule(const String& aModule, int color);
	
	void SetShowPaths(bool show);
	
	void OnTreModuleRmbMenu(Bar& bar);
	
	void OnSelectSource();
	void OnRename();
	
	void OnAddFile();
	void OnAddFolder();
	void OnRenameFile();
	void OnRenameFolder();
	void OnDeleteFile();
	void OnDeleteFolder();
	
	void OnDrag();
	void OnDrop(int ii, PasteClip& pc);
	
	void OnFileNameCopy();
	
	String GetCursorItem() {
		int cursor = treModules.GetCursor();
		return cursor != -1 ? treModules.GetNode(cursor).key : "";
	}
	
	void OpenNodes(Vector<String>& openNodes);
	void HighlightFile(String& file);
	
	void ReloadDocs();
	
private:
	FrameTop<ParentCtrl> frame;
	TreeCtrl treModules;
	
	Index<String> modules;
	Vector<String> openDocs;
	
	bool showPaths = false;
	
	int AddModule(int parent, const String& path, const String& ppath, const Image& img);
	void AddDocFile(const String& path);
};

#endif

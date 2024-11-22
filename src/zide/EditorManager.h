#ifndef _zide_EditorManager_h_
#define _zide_EditorManager_h_

#include "zide.h"

class SmartEditor: public CodeEditor {
public:
	typedef SmartEditor CLASSNAME;
};

class OpenFileInfo {
public:
	SmartEditor editor;
	Index<String> classes;
	bool IsChanged;
	uint64 Hash;

	OpenFileInfo(): IsChanged(false), Hash(0) {
	}
};

class EditorManager: public ParentCtrl {
public:
	typedef EditorManager CLASSNAME;
	
	Callback WhenTabChange;
	Callback WhenEditorChange;
	Callback WhenEditorCursor;
		
	EditorManager();
	
	void Open(const String& item, bool forceReload = false);
	
	void ShowTabs(bool aShow) {
		tabFiles.Show(aShow);
	}
	
	bool IsChanged(int i);
	void SetChanged(int i, bool changed);

	int GetCount() const {
		return tabFiles.GetCount();
	}

	int GetCursor() const {
		return tabFiles.GetCursor();
	}
	
	String ActiveFile() {
		int i = GetCursor();
		if (i == -1)
			return "";
	
		return tabFiles[i].key;
	}
	
	OpenFileInfo* GetInfo() {
		int i = GetCursor();
		if (i == -1)
			return nullptr;
		
		WString file = tabFiles[i].key;
		int j = files.Find(file);
		if (j == -1)
			return nullptr;
		
		return &files[j];
	}
	
private:
	FileTabs tabFiles;
	ParentCtrl canvas;
	
	ArrayMap<WString, OpenFileInfo> files;

	void OnTabChange();
	void OnEditorChange();
	void OnEditorCursor();
	
	OpenFileInfo& GetInfo(int i);
};

#endif

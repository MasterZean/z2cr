#ifndef _zide_EditorManager_h_
#define _zide_EditorManager_h_

#include "zide.h"

class Settings {
public:
	bool   ShowTabs = false;
	bool   ShowSpaces = false;
	bool   ShowNewlines = false;
	bool   WarnSpaces = true;
	int    TabSize = 4;
	bool   IndentSpaces = false;
	bool   ShowLineNums = true;
	bool   HighlightLine = true;
	int    TabPos = AlignedFrame::TOP;
	int    TabClose = AlignedFrame::RIGHT;
	int    LinePos = 96;
	Color  LineColor = LtGray();
	int    ScopeHighlight = 2;
	int    Brackets = 1;
	bool   Thousands = true;
	int    Theme = 0;
	
	bool   ToolbarInRow = true;
	
	String Style;
	
	void Serialize(Stream& s) {
		s % ShowTabs % ShowSpaces % ShowNewlines % WarnSpaces % TabSize % IndentSpaces
		  % ShowLineNums % HighlightLine % TabPos % TabClose % LinePos % LineColor % Style
		  % ScopeHighlight % Brackets % Thousands % Theme % ToolbarInRow;
	}
};

class SmartEditor: public CodeEditor {
public:
	typedef SmartEditor CLASSNAME;
	
	int ID = 0;
	Index<String> classes;
	Index<String> openExploreNodes;
	Index<String> backupExploreNodes;
	bool IsChanged = false;
	uint64 Hash = 0;
};

class EditorManager: public ParentCtrl {
public:
	typedef EditorManager CLASSNAME;
	
	Callback WhenTabChange;
	Callback WhenEditorChange;
	Callback WhenEditorCursor;
	Callback WhenAnnotation;
		
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
	
	SmartEditor* GetEditor() {
		int i = GetCursor();
		if (i == -1)
			return nullptr;
		
		WString file = tabFiles[i].key;
		int j = files.Find(file);
		if (j == -1)
			return nullptr;
		
		files[j].ID = j;
		return &files[j];
	}
	
	static void SetSettings(CodeEditor& editor, Settings& settings, const String& syntax);
	
	void SetSettings(Settings& settings);
	void SetTabSettings(const Settings& settings);
	void SetColors(int colors);
	
	void Save(int i);
	void Save(const String& item);
	void SaveAll();
	void SaveAllIfNeeded();
	bool PromptSaves();
	void RemoveFile(const String& item);
			
	bool OnRenameFiles(const Vector<String>& fileListToRename, const String& oldPath, const String& newPath);
	
private:
	FileTabs tabFiles;
	ParentCtrl canvas;
	
	Settings settings;
	
	ArrayMap<WString, SmartEditor> files;

	void OnTabChange();
	void OnEditorChange();
	void OnEditorCursor();
	void OnAnnotation();
	
	void OnTabClose(Value val);
	bool OnConfirmClose(Value val);
	bool OnCancelClose(Value val);
	
	SmartEditor& GetInfo(int i);
};

void WhiteTheme();
void DarkTheme();

#endif

#include "EditorManager.h"

EditorManager::EditorManager() {
	AddFrame(tabFiles);
	Add(canvas);
	canvas.HSizePos().VSizePos();
	
	tabFiles.StyleDefault().Write().Variant1Crosses();
	tabFiles.WhenAction = THISBACK(OnTabChange);
	tabFiles.WhenClose = THISBACK(OnTabClose);
	tabFiles.ConfirmClose = THISBACK(OnConfirmClose);
	tabFiles.CancelClose = THISBACK(OnCancelClose);
}

bool EditorManager::OnCancelClose(Value val) {
	int count = tabFiles.GetCount();
	return count <= 1;
}

void EditorManager::Open(const String& item, bool forceReload) {
	if (!FileExists(item)) {
		ErrorOK("[pd Could not open file:&]-|[* " + DeQtf(item) + "]");
		return;
	}
	
	WString w = item.ToWString();
	
	int i = tabFiles.FindKey(item);
	if (i == -1) {
		int ii = files.Find(w);
		if (ii == -1) {
			SmartEditor& ff = files.Add(w);
			ii = files.GetCount() - 1;
			String content = LoadFile(item);
			ff.Set(content);
		}
		
		SmartEditor& editor = files[ii];
		
		
		tabFiles.AddFile(w, ZImg::zsrc());

		/*if (settings.Theme == 0)
			f.editor.WhiteTheme();
		else if (settings.Theme == 1)
			DarkTheme();*/
		// TODO:
		//f.editor.SetBarLight(settings.Theme == 0);
		
		editor.AddFrame(RightSeparatorFrame());
		String syn = EditorSyntax::GetSyntaxForFilename(item);
		editor.WhenAction = THISBACK(OnEditorChange);
		editor.WhenUpdate = THISBACK(OnEditorChange);
		editor.WhenSel = THISBACK(OnEditorCursor);
		//f.editor.WhenPopup = WhenPopup;
		editor.HiliteIfEndif(true);
		editor.CheckEdited();
		editor.HiliteScope(2);
		editor.Annotations(Zx(10));
		canvas.Add(editor);
		editor.HSizePos().VSizePos();
		WhenEditorCursor();
		editor.SetFocus();
		SetSettings(editor, settings, syn);
		WhenEditorChange();
		
		//f.editor.WhenAssistChange = THISBACK(OnAssistChange);
		//f.editor.WhenAnnotationMove = THISBACK(OnAnnotation);
	}
	else {
		WString w = item.ToWString();
		int j = files.Find(w);
		if (j != -1) {
			SmartEditor& editor = files[j];
			tabFiles.SetCursor(i);
			editor.SetFocus();
			
			if (forceReload) {
				String content = LoadFile(item);
				editor.Set(content);
			}
		}
	}
}

void EditorManager::OnTabChange() {
	WString file = tabFiles[tabFiles.GetCursor()].key;
	for (int j = 0; j < files.GetCount(); j++)
		files[j].Hide();
	
	int i = files.Find(file);
	if (i != -1)
		files[i].Show();
	
	WhenTabChange();
	WhenEditorChange();
}

void EditorManager::OnEditorChange() {
	SetChanged(tabFiles.GetCursor(), true);
	WhenEditorChange();
}

void EditorManager::OnEditorCursor() {
	WhenEditorCursor();
}

SmartEditor& EditorManager::GetInfo(int i) {
	WString w = tabFiles[i].key;

	int j = files.Find(tabFiles[i].key);
	return files[j];
}

bool EditorManager::IsChanged(int i) {
	if (i < 0 || i > tabFiles.GetCount())
		return false;

	return GetInfo(i).IsChanged;
}

void EditorManager::SetChanged(int i, bool changed) {
	if (i < 0 || i > tabFiles.GetCount())
		return;

	GetInfo(i).IsChanged = changed;
	if (changed)
		tabFiles.SetColor(i, Color(255, 127, 127));
	else
		tabFiles.SetColor(i, Null);
}

void EditorManager::SetSettings(CodeEditor& editor, Settings& settings, const String& syntax) {
	editor.ShowTabs(settings.ShowTabs);
	editor.ShowSpaces(settings.ShowSpaces);
	editor.ShowLineEndings(settings.ShowNewlines);
	editor.WarnWhiteSpace(settings.WarnSpaces);
	editor.TabSize(settings.TabSize);
	editor.IndentSpaces(settings.IndentSpaces);
	editor.LineNumbers(settings.ShowLineNums);
	editor.ShowCurrentLine(settings.HighlightLine ? HighlightSetup::GetHlStyle(HighlightSetup::SHOW_LINE).color : (Color)Null);
	editor.BorderColumn(settings.LinePos, settings.LineColor);
	editor.HiliteScope(settings.ScopeHighlight);
	editor.HiliteBracket(settings.Brackets);
	editor.ThousandsSeparator(settings.Thousands);
	editor.LoadHlStyles(settings.Style);
	
	editor.Highlight(syntax);
}

void EditorManager::SetSettings(Settings& settings) {
	tabFiles.SetAlign(settings.TabPos);
	tabFiles.Crosses(settings.TabClose >= 0, settings.TabClose);
	
	CodeEditor::LoadHlStyles(settings.Style);
	
	for (int i = 0; i < files.GetCount(); i++)
		SetSettings(files[i], settings, EditorSyntax::GetSyntaxForFilename(files.GetKey(i).ToString()));
	
	this->settings = settings;
}

void EditorManager::SetColors(int colors) {
	for (int i = 0; i < files.GetCount(); i++) {
		if (colors == 0) {
			files[i].WhiteTheme();
			// TODO:
			//files[i].SetBarLight(true);
		}
		else if (colors == 1) {
			//DarkTheme();
			// TODO:
			//files[i].SetBarLight(false);
		}
	}
}

void EditorManager::OnTabClose(Value val) {
	WString w = val;
	int j = files.Find(w);
	if (j != -1)
		files.Remove(j);
}

bool EditorManager::OnConfirmClose(Value val) {
	WString w = val;
	int i = tabFiles.FindKey(w);

	if (i == -1)
		return false;

	if (IsChanged(i)) {
		BeepQuestion();
		String fn = DeQtf(tabFiles[i].key.ToString());
		int save = PromptSaveDontSaveCancel("[ph The following file was changed since last save:&-|[* " + fn + "]&Would you like to save it?]");
		if (save == 1)
			Save(i);
		else if (save == -1)
			return false;
	}
		
	return true;
}

void EditorManager::Save(int i) {
	if (i < 0 || i > tabFiles.GetCount())
		return;

	int j = files.Find(tabFiles[i].key);
	if (j == -1)
		return;

	SaveFile(files.GetKey(j).ToString(), files[j].Get());
	SetChanged(i, false);
}

void EditorManager::SaveAll() {
	for (int i = 0; i < tabFiles.GetCount(); i++)
		Save(i);
}

void EditorManager::SaveAllIfNeeded() {
	for (int i = 0; i < tabFiles.GetCount(); i++)
		if (IsChanged(i))
			Save(i);
}

bool EditorManager::PromptSaves() {
	for (int i = 0; i < tabFiles.GetCount(); i++) {
		SmartEditor& editor = GetInfo(i);
		if (editor.IsChanged) {
			BeepQuestion();
			int save = PromptSaveDontSaveCancel("[ph The following file was changed since last save:&-|[* " + DeQtf(tabFiles[i].key.ToString()) + "]&Would you like to save it?]");
			if (save == 1)
				Save(i);
			else if (save == -1)
				return false;
		}
	}

	return true;
}

bool EditorManager::OnRenameFiles(const Vector<String>& fileListToRename, const String& oldPath, const String& newPath) {
	for (int i = 0; i < fileListToRename.GetCount(); i++) {
		int j = tabFiles.FindKey(fileListToRename[i].ToWString());
		if (j != -1) {
			if (IsChanged(j)) {
				Save(j);
			}
		}
	}
	
	if (FileMove(oldPath, newPath))
		for (int i = 0; i < fileListToRename.GetCount(); i++) {
			int j = tabFiles.FindKey(fileListToRename[i].ToWString());
			if (j != -1) {
				ASSERT(fileListToRename[i].StartsWith(oldPath));
				String np = newPath + fileListToRename[i].Mid(oldPath.GetLength());
				WString w1 = tabFiles.GetKey(j);
				WString w2 = np.ToWString();
				int k = files.Find(w1);
				ASSERT(k != -1);
				SmartEditor* editor = files.Detach(k);
				files.Add(w2);
				files.Set(files.GetCount() - 1, editor);
				DUMP(w1);
				DUMP(w2);
				tabFiles.RenameFile(w1, w2, ZImg::zsrc);
			}
		}
		
	return true;
}

void EditorManager::RemoveFile(const String& item) {
	files.RemoveKey(item.ToWString());
	int ki = tabFiles.FindKey(item);
	tabFiles.CloseForce(ki);
}

void EditorManager::Save(const String& item) {
	files.RemoveKey(item.ToWString());
	int ki = tabFiles.FindKey(item);
	tabFiles.CloseForce(ki);
}
#include "EditorManager.h"

void WhiteTheme() {
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SLOT, Magenta);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_BIN, Blue);
}

void DarkTheme() {
	HighlightSetup::SetHlStyle(HighlightSetup::INK_NORMAL, Color(255, 251, 247), true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_NORMAL, Color(51, 51, 51));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SELECTED, White);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_SELECTED, Color(0, 120, 215));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DISABLED, Color(109, 109, 109));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_READONLY, Color(91, 91, 91));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_COMMENT, Color(134, 134, 134), false, true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_COMMENT_WORD, Blue, true, true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_COMMENT_WORD, Yellow);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_STRING, Color(238, 222, 130));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_OPERATOR, Color(249, 167, 40), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_KEYWORD, Color(254, 95, 146), true, true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_UPP, Color(188, 234, 100), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SLOT, Color(188, 234, 100), true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_LNG, Color(59, 75, 78));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_ERROR, LtRed);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_PAR0, White, true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_PAR1, Color(233, 126, 117), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_PAR2, Color(61, 124, 61), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_PAR3, Color(143, 58, 143), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_INT, Color(205, 170, 249), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_FLOAT, Color(0xE2, 0x7F, 0xC8), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_HEX, Color(0xE2, 0x7F, 0xC8), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_OCT, Color(0xE2, 0x7F, 0xC8), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_CONST_STRINGOP, Color(205, 170, 249));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BRACKET0, Color(127, 127, 127), true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BRACKET, Color(197, 127, 127), true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BLOCK1, Color(35, 35, 35));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BLOCK2, Color(44, 31, 65));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BLOCK3, Color(18, 55, 2));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_BLOCK4, Color(78, 78, 3));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_MACRO, White, true, true);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_MACRO, Color(71, 71, 71));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_IFDEF, Color(73, 73, 73));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_IFDEF, Color(170, 170, 170));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_UPPER, Color(28, 255, 0));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SQLBASE, Color(113, 255, 255), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SQLFUNC, Color(113, 255, 255), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_SQLBOOL, Color(113, 255, 255), true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_UPPMACROS, Color(255, 127, 255));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_UPPLOGS, Green);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DIFF_FILE_INFO, Black, true);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DIFF_HEADER, Color(28, 127, 200));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DIFF_ADDED, Color(28, 42, 255));
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DIFF_REMOVED, LtRed);
	HighlightSetup::SetHlStyle(HighlightSetup::INK_DIFF_COMMENT, Color(80, 255, 80));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_SELWORD, Yellow);
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_ERROR, Color(226, 42, 0));
	HighlightSetup::SetHlStyle(HighlightSetup::PAPER_WARNING, Color(255, 255, 205));
	HighlightSetup::SetHlStyle(HighlightSetup::SHOW_LINE, Color(67, 67, 67));
	HighlightSetup::SetHlStyle(HighlightSetup::WHITESPACE, Color(126, 186, 234));
	HighlightSetup::SetHlStyle(HighlightSetup::WARN_WHITESPACE, Color(191, 126, 126));
}

int AdjustForTabs(const String& text, int col, int tabSize) {
	int pos = 1;
	
	for (int i = 0; i < text.GetLength(); i++) {
		if (text[i] == '\t') {
			int newpos = (pos + tabSize - 1) / tabSize * tabSize + 1;
			col -= newpos - pos - 1;
			pos = newpos;
		}
		else
			pos++;
	}
	
	return col;
}

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
		editor.WhenAnnotationMove = THISBACK(OnAnnotation);
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
	if (i != -1) {
		files[i].openExploreNodes = std::move(files[i].backupExploreNodes);
		files[i].Show();
	}
	
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

void EditorManager::OnAnnotation() {
	WhenAnnotation();
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
	SetTabSettings(settings);
	
	CodeEditor::LoadHlStyles(settings.Style);
	
	for (int i = 0; i < files.GetCount(); i++)
		SetSettings(files[i], settings, EditorSyntax::GetSyntaxForFilename(files.GetKey(i).ToString()));
	
	this->settings = settings;
}

void EditorManager::SetTabSettings(const Settings& settings) {
	tabFiles.SetAlign(settings.TabPos);
	tabFiles.Crosses(settings.TabClose >= 0, settings.TabClose);
}

void EditorManager::SetColors(int colors) {
	for (int i = 0; i < files.GetCount(); i++) {
		if (colors == 0) {
			files[i].WhiteTheme(false);
			WhiteTheme();
		}
		else if (colors == 1) {
			files[i].DarkTheme(false);
			DarkTheme();
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
	int i = tabFiles.FindKey(item);
	
	int j = files.Find(item.ToWString());
	if (j == -1)
		return;

	SaveFile(files.GetKey(j).ToString(), files[j].Get());
	SetChanged(i, false);
}

void EditorManager::SetErrors(const String errText) {
	for (int i = 0; i < files.GetCount(); i++) {
		files[i].errorList.Clear();
		files[i].Errors(Vector<Point>());
	}
	
	//int errors = 0;
	Vector<String> lines = Split(errText, '\n');
	//Vector<Point> errorList;
	for (int i = 0; i < lines.GetCount(); i++) {
		String s = TrimBoth(lines[i]);
		
		int ii = s.Find('(');
		if (ii != -1) {
			int jj = s.Find(')', ii);
			if (ii < jj) {
				String path = s.Mid(0, ii);
				
				int fi = files.Find(path.ToWString());
				
				if (fi != -1) {
					String ll = s.Mid(ii + 1, jj - ii - 1);
					Vector<String> s2 = Split(ll, ',');
					if (s2.GetCount() == 2) {
						int line = StrInt(s2[0]) - 1;
						int col = StrInt(s2[1]) - 1;
						
						String text = files[fi].GetUtf8Line(line);
						col = AdjustForTabs(text, col, settings.TabSize);
						
						files[fi].errorList.Add(Point(col, line));
						//errors++;
					}
				}
			}
		}
	}
	
	for (int i = 0; i < files.GetCount(); i++)
		if (files[i].errorList.GetCount()) {
			files[i].Errors(std::move(files[i].errorList));
			files[i].RefreshLayoutDeep();
		}
}

void EditorManager::ClearErrors() {
	for (int i = 0; i < files.GetCount(); i++)
		files[i].Errors(Vector<Point>());
}



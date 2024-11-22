#include "EditorManager.h"

EditorManager::EditorManager() {
	AddFrame(tabFiles);
	Add(canvas);
	canvas.HSizePos().VSizePos();
	
	tabFiles.StyleDefault().Write().Variant1Crosses();
	tabFiles.WhenAction = THISBACK(OnTabChange);
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
			OpenFileInfo& ff = files.Add(w);
			ii = files.GetCount() - 1;
			String content = LoadFile(item);
			ff.editor.Set(content);
		}
		
		OpenFileInfo& f = files[ii];
		
		
		tabFiles.AddFile(w, ZImg::zsrc());

		/*if (settings.Theme == 0)
			f.editor.WhiteTheme();
		else if (settings.Theme == 1)
			DarkTheme();*/
		// TODO:
		//f.editor.SetBarLight(settings.Theme == 0);
		
		f.editor.AddFrame(RightSeparatorFrame());
		//String syn = EditorSyntax::GetSyntaxForFilename(item);
		f.editor.WhenAction = THISBACK(OnEditorChange);
		f.editor.WhenSel = THISBACK(OnEditorCursor);
		//f.editor.WhenPopup = WhenPopup;
		f.editor.HiliteIfEndif(true);
		f.editor.CheckEdited();
		f.editor.HiliteScope(2);
		f.editor.Annotations(Zx(10));
		canvas.Add(f.editor);
		f.editor.HSizePos().VSizePos();
		WhenEditorCursor();
		f.editor.SetFocus();
		//SetSettings(f.editor, settings, syn);
		WhenEditorChange();
		
		//f.editor.WhenAssistChange = THISBACK(OnAssistChange);
		//f.editor.WhenAnnotationMove = THISBACK(OnAnnotation);
	}
	else {
		WString w = item.ToWString();
		int j = files.Find(w);
		if (j != -1) {
			OpenFileInfo& f = files[j];
			tabFiles.SetCursor(i);
			f.editor.SetFocus();
			
			if (forceReload) {
				String content = LoadFile(item);
				f.editor.Set(content);
			}
		}
	}
}

void EditorManager::OnTabChange() {
	WString file = tabFiles[tabFiles.GetCursor()].key;
	for (int j = 0; j < files.GetCount(); j++)
		files[j].editor.Hide();
	
	int i = files.Find(file);
	if (i != -1)
		files[i].editor.Show();
	
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

OpenFileInfo& EditorManager::GetInfo(int i) {
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
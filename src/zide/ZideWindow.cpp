#include "ZideWindow.h"
#include "ItemDisplay.h"

#include <z2crlib/Assembly.h>
#include <z2crlib/ZSource.h>
#include <z2crlib/ZScanner.h>

int ZideWindow::HIGHLIGHT_Z2;
extern bool doCapture;

ZideWindow::ZideWindow() {
	CtrlLayout(*this, "ZIDE");
	Sizeable().Zoomable().Icon(ZImg::zide());
	
	int r = HorzLayoutZoom(100);
	int l = HorzLayoutZoom(250);
	
	mnuMain.Transparent();
	
	if (settings.ToolbarInRow) {
		tlbMain.SetFrame(NullFrame());
		int tcy = tlbMain.GetStdHeight() + 2 * Zy(2);
		
		bararea.Add(mnuMain.LeftPos(0, l).VCenterPos(mnuMain.GetStdHeight()));
		bararea.Add(tlbMain.HSizePos(l, r).VCenterPos(tcy));
		bararea.Add(lblLine.RightPos(4, r).VSizePos(2, 3));
		lblLine.AddFrame(ThinInsetFrame());
		bararea.Height(max(mnuMain.GetStdHeight(), tcy));
		AddFrame(bararea);
		tlbMain.Transparent();
		AddFrame(TopSeparatorFrame());
	}
	
	mnuMain.Set(THISBACK(DoMainMenu));
	tlbMain.Set(THISBACK(MainToolbar));
		
	Add(splAsbCanvas);
	splAsbCanvas.Horz(asbAss, canvas);
	splAsbCanvas.SetPos(1750);
	
	canvas.Add(tabs);
	tabs.SizePos();
	
	console.Height(Zy(150));
	canvas.AddFrame(splBottom.Bottom(console, Zy(150)));
	
	console.WhenLeft = THISBACK(OnOutputSel);
	
	splBottom.Hide();
	
	CtrlLayout(explore);
	explore.lstItems.SetDisplay(Single<ItemDisplay>());
	explore.lstItems.RenderMultiRoot();
	explore.lstItems.NoRoot();
	explore.lstItems.WhenAction = THISBACK(OnExplorerClick);
	//explore.lstItems.Set(0, "aa", RawToValue(ZItem()));
	explore.lstItems.NoWantFocus();
	//explore.lstItems.WhenBar = THISBACK(OnExplorerMenu);
	explore.AddFrame(LeftSeparatorFrame());
	
	tabs.AddFrame(splExplore.Left(explore, Zx(200)));
	
	tabs.WhenTabChange = THISBACK(OnTabChange);
	tabs.WhenEditorCursor = THISBACK(OnEditorCursor);
	tabs.WhenEditorChange = THISBACK(OnEditorChange);
	
	asbAss.WhenSelectSource = THISBACK(OnSelectSource);
	asbAss.WhenFileRemoved = THISBACK(OnFileRemoved);
	asbAss.WhenFileSaved = THISBACK(OnFileSaved);
	asbAss.WhenRenameFiles = callback(&tabs, &EditorManager::OnRenameFiles);
	
	WhenClose = THISBACK(OnClose);
	
	lblLine.SetAlign(ALIGN_CENTER);
	
	tabs.ShowTabs(false);
	
	mbtEntryPoint.NoWantFocus();
	mbtEntryPoint.Tip("Current entry point");
	mbtEntryPoint.Set("* File in editor");
	mbtEntryPoint.AddButton().Tip("Set entry point").SetImage(ZImg::dots) <<= Callback()/*THISBACK(DropTypeList)*/;
	
	lstBldConf.Add("Console");
	lstBldConf.Add("Windows");
	lstBldConf.Tip("Select build configuration");
	lstBldConf.SetIndex(0);
	lstBldConf.NoDropFocus();
	lstBldConf.NoWantFocus();
	
	mbtBldMode.NoWantFocus();
	mbtBldMode.Tip("Build mode");
	mbtBldMode.AddButton().Tip("Build method").Left() <<= THISBACK(DropMethodList);
	mbtBldMode.AddButton().Tip("Build type") <<= THISBACK(DropTypeList);
	mbtBldMode.AddButton().Tip("Build architecture") <<= THISBACK(DropArchList);
	
	popMethodList.Normal();
	popMethodList.WhenSelect = THISBACK(OnSelectMethod);
	
	popTypeList.Normal();
	popTypeList.WhenSelect = THISBACK(OnSelectMethod);
	popTypeList.Add("Debug");
	popTypeList.Add("Speed");
	popTypeList.Add("Size");
	popTypeList.SetCursor(1);
	
	popArchList.Normal();
	popArchList.WhenSelect = THISBACK(OnSelectMethod);
	
	OrderColors();
}

void ZideWindow::OrderColors() {
	colors.Add(HighlightSetup::INK_NORMAL);
	colors.Add(HighlightSetup::PAPER_NORMAL);
	colors.Add(HighlightSetup::INK_SELECTED);
	colors.Add(HighlightSetup::PAPER_SELECTED);
	
	colors.Add(HighlightSetup::INK_KEYWORD);
	colors.Add(HighlightSetup::INK_UPP);
	colors.Add(HighlightSetup::INK_SLOT);
	
	colors.Add(HighlightSetup::INK_CONST_STRING);
	colors.Add(HighlightSetup::INK_CONST_STRINGOP);
	
	colors.Add(HighlightSetup::INK_CONST_INT);
	colors.Add(HighlightSetup::INK_CONST_HEX);
	colors.Add(HighlightSetup::INK_CONST_OCT);
	colors.Add(HighlightSetup::INK_CONST_BIN);
	colors.Add(HighlightSetup::INK_CONST_FLOAT);
	
	colors.Add(HighlightSetup::INK_OPERATOR);
		
	colors.Add(HighlightSetup::INK_COMMENT);
	colors.Add(HighlightSetup::INK_COMMENT_WORD);
	colors.Add(HighlightSetup::PAPER_COMMENT_WORD);
	
	colors.Add(HighlightSetup::INK_ERROR);
	colors.Add(HighlightSetup::INK_PAR0);
	colors.Add(HighlightSetup::INK_PAR1);
	colors.Add(HighlightSetup::INK_PAR2);
	colors.Add(HighlightSetup::INK_PAR3);
	
	colors.Add(HighlightSetup::PAPER_BRACKET0);
	colors.Add(HighlightSetup::PAPER_BRACKET);
	colors.Add(HighlightSetup::PAPER_BLOCK1);
	colors.Add(HighlightSetup::PAPER_BLOCK2);
	colors.Add(HighlightSetup::PAPER_BLOCK3);
	colors.Add(HighlightSetup::PAPER_BLOCK4);
	
	if (IsDarkTheme())
		DarkTheme();
	else
		WhiteTheme();
	
	//settings.Theme = IsDarkTheme();
};

void ZideWindow::Serialize(Stream& s) {
	int version = 1;
	s / version;

	if (s.IsLoading()) {
		bool maximized = true;
		int width = 1000;
		int height = 600;
		s % maximized % width % height;
		if (maximized)
			Maximize(maximized);
		else
			HCenterPos(width, 0).VCenterPos(height, 0);
		
		int subs = 0;
		s % subs;
		
		int split;
		s % split;
		splAsbCanvas.SetPos(split);
		s % split;
		splExplore.SetSize(split);
		s % split;
		splBottom.SetSize(split);
	}
	else {
		bool maximized = IsMaximized();
		int width = GetSize().cx;
		int height = GetSize().cy;
		s % maximized % width % height;
		int subs = lstBldConf.GetIndex();
		s % subs;
		
		int split = splAsbCanvas.GetPos();
		s % split;
		split = splExplore.GetSize();
		s % split;
		split = splBottom.GetSize();
		s % split;
		
		if (subs >= 0 & subs < lstBldConf.GetCount())
			lstBldConf.SetIndex(subs);
	}
	
	s % LastPackage % RecentPackages % openNodes % ActiveFile % openDialogPreselect;
	s % settings % method % arch % oShowPakPaths;
}

void ZideWindow::LoadPackage(const String& package) {
	if (package.GetCount() == 0)
		return;
	
#ifdef PLATFORM_WIN32
	String platform = "WIN32";
	String platformLib = "microsoft.windows";
#endif

			
#ifdef PLATFORM_POSIX
	String platform = "POSIX";
	String platformLib = "ieee.posix";
#endif

	LastPackage = package;
	asbAss.ClearModules();
	packages << LastPackage;
	LoadModule(LastPackage, 0);

	String pak = NativePath(CurFolder + "source/stdlib/sys.core");
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 1);
	}
	pak = NativePath(CurFolder + "source/stdlib/bind.c");
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 1);
	}
	pak = NativePath(CurFolder + "source/stdlib/" + platformLib);
	if (DirectoryExists(pak)) {
		packages << pak;
		LoadModule(pak, 2);
	}
	
	openNodes.Add(LastPackage);
	SetupLast();
	
	int i = RecentPackages.Find(LastPackage);
	if (i == -1)
		RecentPackages.Insert(0, LastPackage);
	else {
		RecentPackages.Remove(i);
		RecentPackages.Insert(0, LastPackage);
	}
}

void ZideWindow::LoadModule(const String& mod, int color) {
	asbAss.AddModule(NativePath(mod), color);
}

void ZideWindow::SetupLast() {
	asbAss.OpenNodes(openNodes);
	asbAss.HighlightFile(ActiveFile);
	
	tabs.SetSettings(settings);
	
	tabs.ShowTabs(tabs.GetCount());
	splExplore.Show(tabs.GetCount());
	asbAss.SetShowPaths(oShowPakPaths);
	
	int ii = popMethodList.Find(method);
	if (ii != -1) {
		popMethodList.SetCursor(ii);
		OnSelectMethod();
	}
}

void ZideWindow::OnFileRemoved(const String& file) {
	tabs.RemoveFile(file);
	mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnSelectSource() {
	ActiveFile = asbAss.GetCursorItem();
		
	tabs.Open(ActiveFile);
	mnuMain.Set(THISBACK(DoMainMenu));
}

void ZideWindow::OnFileSaved(const String& file) {
	tabs.Save(file);
}

void ZideWindow::OnTabChange() {
	ActiveFile = tabs.ActiveFile();
	asbAss.HighlightFile(ActiveFile);
	
	tabs.ShowTabs(tabs.GetCount());
	splExplore.Show(tabs.GetCount());
}

void ZideWindow::OnEditorChange() {
	SmartEditor* editor = tabs.GetEditor();
	if (!editor)
		return;
	
	editor->Hash++;
	if (!editThread)
		return;
	//	Thread().Run(callback3(OutlineThread, this, editor.Get(), info->Hash));
	
	Assembly ass;
	ZPackage pak = ZPackage(ass, "temp", LastPackage);
	ZSource source = ZSource(pak);
	source.LoadVirtual(editor->Get());
	
#ifdef PLATFORM_WIN32
	PlatformType platform = PlatformType::WINDOWS;
#else
	PlatformType platform = PlatformType::UNIX;
#endif

	ZScanner scaner = ZScanner(source, platform);
	scaner.Scan();
	
	int cur = explore.lstItems.GetCursor();
	Point pt = explore.lstItems.GetScroll();
	
	explore.lstItems.Clear();
	explore.lstItems.Set(0, "aa");
	//explore.lstItems.Set(0, "aa", RawToValue(ZItem()));
	
	for (int i = 0; i < scaner.EntityContent.GetCount(); i++) {
		ZEntity& ent = *scaner.EntityContent[i];
		
		if (ent.Type == EntityType::Class || ent.Type == EntityType::Namespace) {
			String s;
			ZNamespace& cls = (ZNamespace&)ent;
			
			ZItem clsItem;
			
			if (cls.IsClass) {
				clsItem.Kind = ZItem::itClass;
				s = cls.Name;
				if (cls.Namespace().ProperName.GetLength())
					s << " (" << cls.Namespace().ProperName << ")";
				clsItem.Name = s;
				clsItem.Namespace = cls.Namespace().Name + cls.Name;
			}
			else {
				clsItem.Kind = ZItem::itNamespace;
				clsItem.Name = cls.ProperName;
			}
			
			//clsItem.Kind = /*cls.Scan.IsEnum ? ZItem::itEnum : */ZItem::itClass;
			clsItem.Pos = cls.DefPos.P;
			
			int node = explore.lstItems.Add(0, Image(), cls.Name, RawToValue(clsItem));
			
			for (int j = 0; j < cls.PreVariables.GetCount(); j++) {
				ZVariable& f = cls.PreVariables[j];
				
				ZItem item;
				item.Kind = f.IsConst ? ZItem::itConst: ZItem::itVar;
				item.Name = f.Name;
				item.Pos = f.DefPos.P;
				item.Access = f.Access;
				
				CParser::Pos pos = f.DefPos.Pos;
				const char *ch = pos.ptr + f.Name.GetCount();
				s = "";
								
				while ((*ch >= ' ') && (*ch != '{' && *ch != ';'/* && *ch != '='*/)) {
					s << *ch;
					ch++;
				}
				item.Sig = s;
				
				explore.lstItems.Add(node, Image(), item.Name, RawToValue(item));
				//editor.words.Add(item);
			}
			
			for (int j = 0; j < cls.PreConstructors.GetCount(); j++) {
				ZItem item;
				
				ZFunction& f = cls.PreConstructors[j];
				
				item.Name = f.Name;
				if (f.IsConstructor == 2)
					item.Kind = ZItem::itNamed;
				else
					item.Kind = ZItem::itThis;
				
				s = "{";
				CParser::Pos pos = f.ParamPos.Pos;
				const char *ch = pos.ptr;
				if (*ch == '(')
					ch++;
				while ((*ch >= ' ') && (*ch != ')')) {
					s << *ch;
					ch++;
				}
				s << "}";
				item.Sig = s;
				item.Pos = f.DefPos.P;
				item.Access = f.Access;
				explore.lstItems.Add(node, Image(), item.Name, RawToValue(item));
			}
			
			for (int j = 0; j < cls.PreFunctions.GetCount(); j++) {
				ZFunction& f = cls.PreFunctions[j];
				//if (!over.IsGenerated) {
				
				ZItem item;
				
				item.Name = f.Name;
				item.Access = f.Access;

				CParser::Pos pos = f.ParamPos.Pos;
				const char *ch = pos.ptr;
				s = "";
				if (*ch == '(') {
					s = "(";
					ch++;
					while ((*ch >= ' ') && (*ch != ')')) {
						s << *ch;
						ch++;
					}
					s << ")";
					ch++;
				}
				
				while ((*ch >= ' ') && (*ch != '{' && *ch != ';' && *ch != '=')) {
					s << *ch;
					ch++;
				}
				item.Sig = TrimBoth(s);
				item.Pos = f.DefPos.P;
				if (f.IsProperty)
					item.Kind = f.IsGetter ? ZItem::itGet : ZItem::itSet;
				else
					item.Kind = f.IsFunction ? ZItem::itFunc : ZItem::itDef;
				item.Static = f.IsStatic;
				explore.lstItems.Add(node, Image(), item.Name, RawToValue(item));
			}
			
			explore.lstItems.Open(node);
		}
	}
}

void ZideWindow::OnEditorCursor() {
	SmartEditor* editor = tabs.GetEditor();
	if (!editor)
		return;
	
	Point p = editor->GetColumnLine(editor->GetCursor());
	lblLine.SetText(String().Cat() << "Ln " << (p.y + 1) << ", Cl " << (p.x + 1));
}

void ZideWindow::OnClose() {
	if (running) {
		//GuiLock __;
		OnMenuBuildKill();
	}
	
	int count  = 0;
	int threads = Thread::GetCount();
	while (threads > 0 && count < 2000) {
		Sleep(10);
		count++;
	}
	
	if (tabs.PromptSaves())
		Close();
}

void ZideWindow::AddOutputLine(const String& str) {
	console << str;
	console.ScrollEnd();
}

bool ZideWindow::IsVerbose() const {
	return console.VerboseBuild;
}

void ZideWindow::PutConsole(const char *s) {
	console << s << "\n";
}

void ZideWindow::PutVerbose(const char *s) {
	if(console.VerboseBuild) {
		PutConsole(s);
		console.Sync();
	}
}

void ZideWindow::OutPutEnd(bool result) {
	//console.ScrollLineUp();
	//console.ScrollLineUp();
	Title("ZIDE - Execution done in " + sw.ToString() + " sec.");
	running = false;
	doCapture = false;
}

void ZideWindow::OnOutputSel() {
	if (console.GetSelection().GetLength() != 0)
		return;
	
	Point p = console.GetColumnLine(console.GetCursor());
	
	for (int i = 0; i < 5; i++) {
		if (p.y >= 0 &&	GetLineOfError(p.y))
			return;
		
		p.y--;
	}
}

bool ZideWindow::GetLineOfError(int ln) {
	auto temp = GetEditor();
	if (!temp)
		return false;
	
	CodeEditor& editor = *temp;
	
	String line = console.GetUtf8Line(ln);
	
//#ifdef PLATFORM_WIN32
	int s = line.Find("(");
	int e = line.Find(")");
//#endif

/*#ifdef PLATFORM_POSIX
	int s = line.Find(":");
	int e = line.Find(": error: ");
#endif*/
	
	if (s > -1 && s < e) {
		String file = TrimLeft(line.Left(s));

		if (FileExists(file)) {
			tabs.Open(file);
			
			temp = GetEditor();
			if (!temp)
				return false;
			
			CodeEditor& newEditor = *temp;
			
			String rest = line.Mid(s + 1, e - s - 1);
			Vector<String> v = Split(rest, ",");
			if (v.GetCount() == 2) {
				int x = StrInt(TrimBoth(v[0])) - 1;
				int y = StrInt(TrimBoth(v[1])) - 1;
				
				newEditor.SetCursor(newEditor.GetGPos(x, y));
				newEditor.SetFocus();
				
				return true;
			}
			else if (v.GetCount() == 1) {
				int x = StrInt(TrimBoth(v[0])) - 1;
				
				newEditor.SetCursor(newEditor.GetGPos(x, 1));
				newEditor.SetFocus();
			
				return true;
			}
			
			return false;
		}
	}
	
	return false;
}

void ZideWindow::ReadHlStyles(ArrayCtrl& hlstyle) {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	hlstyle.Clear();
	for(int i = 0; i < colors.GetCount(); i++) {
		const HlStyle& s = editor.GetHlStyle(colors[i]);
		hlstyle.Add(GetHlName(colors[i], editor), s.color, s.bold, s.italic, s.underline);
	}
}

const char* ZideWindow::GetHlName(int i, CodeEditor& editor) {
	if (i == HighlightSetup::INK_UPP)
		return "Class name text";
	else
		return editor.GetHlName(i);
}

bool ZideWindow::LoadMethods() {
	LoadFromXMLFile(methods, CurFolder + "buildMethods.xml");
	return methods.GetCount() != 0;
}

void ZideWindow::DetectMehods() {
	methods.Clear();
	Cout() << "No cached build method found! Trying to auto-detect...\n";
	BuildMethod::Get(methods);
	if (methods.GetCount() == 0) {
		PromptOK("Could not find any build methods. Building is dissabled!");
		canBuild = false;
	}
	StoreAsXMLFile(methods, "methods", CurFolder + "buildMethods.xml");
}

void ZideWindow::PopulateMehods() {
	Index<String> met;
	for (int i = 0; i < methods.GetCount(); i++)
		met.FindAdd(methods[i].Name);
	
	popMethodList.Clear();
	for (int i = 0; i < met.GetCount(); i++)
		popMethodList.Add(met[i]);
}

void ZideWindow::OnExplorerClick() {
	//if (pauseExplorer)
	//	return;
	
	int i = explore.lstItems.GetCursor();
	if (i == -1)
		return;
	
	auto temp = GetEditor();
	if (!temp)
		return;
	
	CodeEditor& editor = *temp;
	
	Point p = ValueTo<ZItem>(explore.lstItems.GetValue(i)).Pos;
	editThread = false;
	editor.SetCursor(editor.GetGPos(p.x - 1, p.y - 1));
	editor.SetFocus();
	editThread = true;
}

void ZideWindow::OnExplorerMenu(Bar& bar) {
	/*int i = explore.lstItems.GetCursor();
	if (i == -1)
		return;

	ZItem zi = ValueTo<ZItem>(explore.lstItems.GetValue(i));
	if (zi.Kind == ZItem::itClass || zi.Kind == ZItem::itEnum)
		bar.Add("Generate documentation template",  THISBACK(OnGenerateDocTemp));*/
}

bool runProg = false;

void ProgThread(DetectWindow* detect) {
	while (runProg) {
		PostCallback(callback(detect, &DetectWindow::OnProgress));
		Sleep(100);
	}
}

void DetectThread(DetectWindow* detect, ZideWindow* zide) {
	zide->DetectMehods();
	PostCallback(callback(detect, &DetectWindow::OnDone));
}

void DetectWindow::Activate() {
	TopWindow::Activate();
	if (ZIDE) {
		runProg = true;
		prog.Run(callback1(ProgThread, this));
		Thread().Run(callback2(DetectThread, this, ZIDE));
	}
}

void DetectWindow::OnDone() {
	runProg = false;
	Close();
}
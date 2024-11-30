#include "ZideWindow.h"

#define IMAGECLASS ZImg
#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_source.h>

static const char *z2_keywords[] = {
	"alias",
	"break",
	"case",
	"catch",
	"const",
	"continue",
	"class",
	"default",
	"def",
	"do",
	"enum",
	"else",
	"for",
	"finally",
	"foreach",
	"func",
	"goto",
	"get",
	"if",
	"in",
	"move",
	"new",
	"namespace",
	"override",
	"private",
	"protected",
	"public",
	"property",
	"ref",
	"return",
	"static",
	"set",
	"switch",
	"this",
	"try",
	"throw",
	"using",
	"virtual",
	"val",
	"while",
	"with",
	NULL
};
	
static const char *empty[] = {
	NULL
};

void CreateZSyntax(One<EditorSyntax>& e, int kind) {
	CSyntax& s = e.Create<CSyntax>();
	s.SetHighlight(kind);
}

GUI_APP_MAIN {
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);

	// register new syntax
	CSyntax::InitKeywords();
	ZideWindow::HIGHLIGHT_Z2 = CSyntax::RegisterNewHighlightType();
	CSyntax::LoadSyntax(ZideWindow::HIGHLIGHT_Z2, z2_keywords, empty);
	
	EditorSyntax::Register("z2", callback1(CreateZSyntax, ZideWindow::HIGHLIGHT_Z2), "*.z2", "Z2	Source Files");
	
	// main window
	Ctrl::SetAppName("ZIDE");
	ZideWindow& zide = *(new ZideWindow());
	
	zide.CurFolder = GetFileDirectory(GetExeFilePath());
	
	FindFile ff(NativePath(zide.CurFolder + "/" + BuildMethod::ExeName("z2c")));
	if (ff.IsExecutable())
		zide.CompilerExe = ff.GetPath();

	if (!LoadFromFile(zide)) {
		String pakPath = NativePath(zide.CurFolder + "source/ut/org.z2legacy.ut");
		zide.LastPackage = pakPath;
		zide.RecentPackages.Add(zide.LastPackage);
		
		String helloPath = NativePath(pakPath + "/HelloWorld.z2");
		if (FileExists(helloPath))
			zide.ActiveFile = helloPath;
	}

	zide.LoadPackage(zide.LastPackage);
	
	zide.Run();
	
	StoreToFile(zide);
	
	delete &zide;
}
#include "ZideWindow.h"

#define IMAGECLASS ZImg
#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_source.h>

static String ExeName(const String& exe) {
#ifdef PLATFORM_WIN32
	return exe + ".exe";
#endif
#ifdef PLATFORM_POSIX
	return exe;
#endif
}

GUI_APP_MAIN {
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	
	Ctrl::SetAppName("ZIDE");
	ZideWindow& zide = *(new ZideWindow());
	
	zide.CurFolder = GetFileDirectory(GetExeFilePath());
	
	FindFile ff(NativePath(zide.CurFolder + "/" + ExeName("z2c")));
	if (ff.IsExecutable())
		zide.CompilerExe = ff.GetPath();

	if (!LoadFromFile(zide)) {
		zide.LastPackage = NativePath("c:/temp/test.pak");
	}

	zide.LoadPackage(zide.LastPackage);
	
	zide.Run();
	
	StoreToFile(zide);
	
	delete &zide;
}
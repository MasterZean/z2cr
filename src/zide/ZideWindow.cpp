#include "ZideWindow.h"

#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_source.h>

ZideWindow::ZideWindow() {
	CtrlLayout(*this, "ZIDE");
}

GUI_APP_MAIN {
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	
	Ctrl::SetAppName("ZIDE");
	ZideWindow& zide = *(new ZideWindow());
	
	zide.Run();
	
	delete &zide;
}

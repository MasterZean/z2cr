#ifndef _zide_zide_h
#define _zide_zide_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <zide/zide.lay>
#include <CtrlCore/lay.h>

#define IMAGEFILE <zide/zide.iml>
#include <Draw/iml_header.h>

class ZideWindow : public WithzideLayout<TopWindow> {
public:
	ZideWindow();
};

#endif

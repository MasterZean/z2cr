#ifndef _zide_ItemDisplay_h_
#define _zide_ItemDisplay_h_

#include "zide.h"

#include <z2crlib/common.h>

using namespace Upp;

class ItemDisplay: public Display {
public:
	int DoPaint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
	
	virtual Size GetStdSize(const Value& q) const {
		NilDraw w;
		return Size(DoPaint(w, Rect(0, 0, INT_MAX, INT_MAX), q, Null, Null, 0), StdFont().GetCy());
	}
	
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const {
		DoPaint(w, r, q, ink, paper, style);
	}
};

class ZItem: Moveable<ZItem> {
public:
	enum Type {
		itClass,
		itNamespace,
		itEnum,
		itAlias,
		itFunc,
		itDef,
		itThis,
		itNamed,
		itConst,
		itVar,
		itDest,
		itGet,
		itSet,
		itGetSet
	};
	
	String Name;
	String Sig;
	String Namespace;
	AccessType Access = AccessType::Public;
	bool Static = false;
	Type Kind;
	Point Pos = Point(0, 0);
};

#endif

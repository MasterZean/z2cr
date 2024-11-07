#include "z2cr.h"

bool ER::PrintPath;

void ER::Error(const ZSource& src, const Point& p, const String& text) {
	//ASSERT(0);
	if (PrintPath)
		throw ZException(String().Cat() << src.Package().Path << src.Path << "(" << p.x << ", " << p.y << ")", text);
	else
		throw ZException(String().Cat() << GetFileName(src.Path) << "(" << p.x << ", " << p.y << ")", text);
}

void ER::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}

ZException ER::Duplicate(const ZSourcePos& p, const String& aText) {
	return ZException(p.ToString(), aText);
}

ZException ER::Duplicate(const String& name, const ZSourcePos& cur, const ZSourcePos& prev) {
	String err;
		
	err << "duplicate symbol: " << name << ", ";
	err << "previous occurrence at:\n";
	err << "\t\t" << prev.ToString() << "\n";
		
	return Duplicate(cur, err);
}



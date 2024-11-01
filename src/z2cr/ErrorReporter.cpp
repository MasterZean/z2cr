#include "z2cr.h"

bool ErrorReporter::PrintPath;

void ErrorReporter::Error(const ZSource& src, const Point& p, const String& text) {
	//ASSERT(0);
	if (PrintPath)
		throw ZException(String().Cat() << src.Package().Path << src.Path << "(" << p.x << ", " << p.y << ")", text);
	else
		throw ZException(String().Cat() << GetFileName(src.Path) << "(" << p.x << ", " << p.y << ")", text);
}

void ErrorReporter::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}

void ErrorReporter::Duplicate(const ZSourcePos& p, const String& aText) {
	throw ZException(p.ToString(), aText);
}


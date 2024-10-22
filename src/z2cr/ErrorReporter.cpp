#include "z2cr.h"

bool ErrorReporter::PrintPath;

void ErrorReporter::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}

void ErrorReporter::Duplicate(const ZSourcePos& p, const String& aText) {
	throw ZException(p.ToString(), aText);
}


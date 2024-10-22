#include "z2cr.h"

bool ErrorReporter::PrintPath;

void ErrorReporter::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}
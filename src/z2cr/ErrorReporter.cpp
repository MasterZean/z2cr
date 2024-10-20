#include "z2cr.h"

void ErrorReporter::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}
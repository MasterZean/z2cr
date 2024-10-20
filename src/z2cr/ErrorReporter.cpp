#include "z2cr.h"

void ErrorReporter::CantOpenFile(const String& aPath) {
	Cout() << "Can't open file: " << aPath << "\n";
}
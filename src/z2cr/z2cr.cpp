#include "z2cr.h"

CONSOLE_APP_MAIN {
	String path = "c:\\temp\\test.z2";
	
	Assembly ass;
	
	ZSource source(ass);
	if (!source.LoadFile(path)) {
		ErrorReporter::CantOpenFile(path);
	}
	
	try {
		Scanner scanner(source, true);
		scanner.Scan();
	}
	catch (ZException e) {
		Cout() << e.Path << ": " << e.Error << "\n";
	}
}

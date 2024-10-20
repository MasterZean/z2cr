#ifndef _z2cr_ErrorReporter_h_
#define _z2cr_ErrorReporter_h_

class ErrorReporter {
public:
	static bool PrintPath;
	
	static void CantOpenFile(const String& aPath);
};

#endif

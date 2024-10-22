#ifndef _z2cr_ErrorReporter_h_
#define _z2cr_ErrorReporter_h_

class ZSourcePos;

class ErrorReporter {
public:
	static bool PrintPath;
	
	static void CantOpenFile(const String& aPath);
	
	static void Duplicate(const ZSourcePos& p, const String& aText);
};

#endif

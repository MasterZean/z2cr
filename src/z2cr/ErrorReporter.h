#ifndef _z2cr_ErrorReporter_h_
#define _z2cr_ErrorReporter_h_

class ZSource;
class ZSourcePos;

class ErrorReporter {
public:
	static bool PrintPath;
	
	static void Error(const ZSource& src, const Point& p, const String& text);
	
	static void CantOpenFile(const String& aPath);
	
	static ZException Duplicate(const ZSourcePos& p, const String& aText);
	static ZException Duplicate(const String& name, const ZSourcePos& cur, const ZSourcePos& prev);
	
	static void ErrIncompatOp(const ZSource& src, const Point& p, const String& op, const String& text, const String& text2) {
		Error(src, p, "Can't apply operator '" + op + "' on types: \n\t\t'\f" + text + "\f' and \n\t\t'\f" + text2 + "\f'");
	}
};

#endif

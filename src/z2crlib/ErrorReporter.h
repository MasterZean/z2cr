#ifndef _z2cr_ErrorReporter_h_
#define _z2cr_ErrorReporter_h_

#include <z2crlib/ZException.h>

class ZSource;
class ZSourcePos;

class ER {
public:
	static bool PrintPath;
	
	static bool  NoColor;
	
	static String White;
	static String DkGray;
	static String Gray;
	static String Red;
	static String Cyan;
	static String Blue;
	static String Green;
	static String Yellow;
	static String Magenta;
	
	static void Error(const ZSource& src, const Point& p, const String& text);
	
	static void CantOpenFile(const String& aPath);
	
	static ZException Duplicate(const ZSourcePos& p, const String& aText);
	static ZException Duplicate(const String& name, const ZSourcePos& cur, const ZSourcePos& prev);
	
	static ZException ErrNestedNamespace(const ZSourcePos& p);
	static ZException ErrNestedClasses(const ZSourcePos& p);
	static ZException ErrUsingInClass(const ZSourcePos& p);
	static ZException ErrDeclOutsideNamespace(const ZSourcePos& p);
	static ZException ErrCantBeStatic(const ZSourcePos& p, const String& type);
	static ZException ErrNamespaceInClass(const ZSourcePos& p);
	
	static void ErrIncompatOp(const ZSource& src, const Point& p, const String& op, const String& text, const String& text2) {
		Error(src, p, "Can't apply operator '" + op + "' on types: \n\t\t'\f" + text + "\f' and \n\t\t'\f" + text2 + "\f'");
	}
	
	static void PrettyPrint(const String& error, Stream& stream, bool color);
	
	static String StripColor(const String& error) {
		StringStream ss;
		PrettyPrint(error, ss, false);
		return ss;
	}
};

#endif

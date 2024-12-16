#ifndef _z2cr_ErrorReporter_h_
#define _z2cr_ErrorReporter_h_

#include <z2crlib/entities.h>
#include <z2crlib/ZException.h>

class ZSource;
class ZSourcePos;

class ER {
public:
	static bool PrintPath;
	
	static ErrorColorType ErrorColor;
	
	static String White;
	static String DkGray;
	static String Gray;
	static String Red;
	static String Cyan;
	static String Blue;
	static String Green;
	static String Yellow;
	static String Magenta;
	
	static String ToColor(const ZNamespace& ns, bool qt = false);
	
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
	
	static void ErrCArrayWrongArgumentNo(const ZSource& source, const Point& p, const ZClass& cls, int argNo);
	
	static void ErrClassTemplateWrongArgumentNo(const ZSource& source, const Point& p, const ZClass& cls, int target, int argNo);
	
	static void CallError(const ZSource& source, const Point& p, Assembly& ass, ZNamespace& owner, ZMethodBundle* def, Vector<Node*>& params, int limit, bool cons = false);
	
	static void ErrIncompatOp(const ZSource& src, const Point& p, const String& op, const String& text, const String& text2) {
		Error(src, p, "Can't apply operator '" + op + "' on types: \n\t\t'\f" + text + "\f' and \n\t\t'\f" + text2 + "\f'");
	}
	
	static void PrettyPrint(const String& error, Stream& stream, ErrorColorType color);
	
	static String StripColor(const String& error) {
		StringStream ss;
		PrettyPrint(error, ss, ErrorColorType::None);
		return ss;
	}
	
	static void ErrNotCompileTimeInst(const ZSource& source, const Point& p) {
		Error(source, p, "Only compile time constants can be used for specilization.");
	}
	
	static void ErrItemCountNegative(const ZSource& source, const Point& p, const String& text) {
		Error(source, p, "\f'" + text + "'\f" + " can only be instantiated with a greater than 0 item count.");
	}
	
	static void ErrItemCountNotInteger(const ZSource& source, const Point& p, const String& text) {
		Error(source, p, "\f'" + text + "'\f" + " can only be instantiated with a integer item count.");
	}
};

#endif

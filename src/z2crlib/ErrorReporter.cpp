#include "z2cr.h"

bool ER::PrintPath;

bool ER::NoColor = false;
String ER::White = "${white}";
String ER::Gray = "${gray}";
String ER::DkGray = "${dkgray}";
String ER::Red = "${red}";
String ER::Cyan = "${cyan}";
String ER::Blue = "${blue}";
String ER::Green = "${green}";
String ER::Yellow = "${yellow}";
String ER::Magenta = "${magenta}";

String ER::ToColor(const ZNamespace& ns, bool qt ) {
	StringStream ss;
	
	ss << ER::White;
	if (qt)
		ss << "'";
	if (ns.IsClass)
		ss << ER::Magenta << "class " << ER::White << ns.Namespace().Name << ER::Cyan << ns.Name;
	else
		ss << ER::Magenta << "namespace " << ER::White << ns.ProperName;
	ss << ER::White;
	if (qt)
		ss << "'";
	
	return ss;
}


void ER::Error(const ZSource& src, const Point& p, const String& text) {
	//ASSERT(0);
	if (PrintPath)
		throw ZException(String().Cat() << src.Package().Path << src.Path << "(" << p.x << ", " << p.y << ")", text);
	else
		throw ZException(String().Cat() << GetFileName(src.Path) << "(" << p.x << ", " << p.y << ")", text);
}

void ER::CantOpenFile(const String& aPath) {
	throw ZException(String().Cat() << "Can't open file: " << aPath);
}

ZException ER::Duplicate(const ZSourcePos& p, const String& aText) {
	return ZException(p.ToString(), aText);
}

ZException ER::Duplicate(const String& name, const ZSourcePos& cur, const ZSourcePos& prev) {
	String err;
		
	err << "duplicate symbol: " << name << ", ";
	err << "previous occurrence at:\n";
	err << "\t\t" << prev.ToString() << "\n";
		
	return Duplicate(cur, err);
}

ZException ER::ErrNestedNamespace(const ZSourcePos& p) {
	return ZException(p.ToString(), "nested namespaces are not supported");
}

ZException ER::ErrDeclOutsideNamespace(const ZSourcePos& p) {
	return ZException(p.ToString(), "declaration encountered outside of a namespace");
}

ZException ER::ErrNestedClasses(const ZSourcePos& p) {
	return ZException(p.ToString(), "nested classes are not supported");
}

ZException ER::ErrUsingInClass(const ZSourcePos& p) {
	return ZException(p.ToString(), "using statement inside a class not supported");
}

ZException ER::ErrCantBeStatic(const ZSourcePos& p, const String& type) {
	return ZException(p.ToString(), type + " declaration can't be static");
}

ZException ER::ErrNamespaceInClass(const ZSourcePos& p) {
	return ZException(p.ToString(), "namespace declaration can't be part of a class");
}

void ER::ErrCArrayWrongArgumentNo(const ZSource& source, const Point& p, const ZClass& cls, int argNo) {
	String err = "template " + ToColor(cls, true) + " must be instanced with 1 or 2 arguments";
				err << ", " << IntStr(argNo) << " provided.";
				
	Error(source, p, err);
}

void ER::ErrClassTemplateWrongArgumentNo(const ZSource& source, const Point& p, const ZClass& cls, int target, int argNo) {
	String err = "template " + ToColor(cls, true) + "' must be instanced with " + IntStr(target) + " argument";
	if (target != 1)
		err << "s";
	err << ", " << IntStr(argNo) << " provided.";
	
	Error(source, p, err);
}

void ER::CallError(const ZSource& source, const Point& p, Assembly& ass, ZNamespace& owner, ZMethodBundle* def, Vector<Node*>& params, int limit, bool cons) {
	String s;

	if (cons) {
		s << ER::ToColor(owner) << ": does not have a constructor\n\t\t";
	}
	else {
		s << ER::ToColor(owner) << ": does not have an overload\n\t\t";
		if (limit == 2)
			s << "func ";
		else if (limit == 1)
			s << "def  ";
		
		if (def)
			s << ER::Green << def->Name << ER::White;
		else
			s << owner.Name;
	}
	
	if (cons)
		s << "{";
	else
		s << "(";
	
	for (int k = 0; k < params.GetCount(); k++) {
		//if (params[k]->IsConst)
		//	s << "const ";
		s << ass.TypeToColor(params[k]->Tt);
		if (k < params.GetCount() - 1)
			s << ", ";
	}
	if (cons)
		s << "}\n";
	else
		s << ")\n";
	
	if (def && def->IsTemplate)
		s << ER::ToColor(owner) << ": has an incompatible template '" << def->Name << "'";
	else if (def) {
		s << "\texisting overloads\n";
		for (int i = 0; i < def->Functions.GetCount(); i++) {
			ZFunction& ol = *def->Functions[i];
			
			if (ol.IsDeleted)
				continue;
			
			/*if (ol.IsConstructor == 1)
				s << "\t\t" << "{" << ol.FuncSig() << "}\n";
			else if (ol.IsConstructor == 2)
				s << "\t\t" << ol.Name << "{" << ol.FuncSig() << "}\n";
			else {
				s << "\t\t";
				if (ol.IsFunction)
					s << "func ";
				else
					s << "def  ";
				s << ol.Name << "(" << ol.FuncSig() << ")\n";
			}*/
			s << "\t\t" << ol.ColorSig() << "\n";
		}
	}
	
	Error(source, p, s);
}

void ZException::PrettyPrint(Stream& stream) {
#ifdef PLATFORM_WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
	WORD                        m_currentConsoleAttr;
	CONSOLE_SCREEN_BUFFER_INFO   csbi;

	//retrieve and save the current attributes
	if(GetConsoleScreenBufferInfo(hConsole, &csbi))
	    m_currentConsoleAttr = csbi.wAttributes;

	int cWhite = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
	int cGray = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	int cRed = FOREGROUND_RED | FOREGROUND_INTENSITY;
	int cCyan = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	int cBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	int cMagenta = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;
	int cGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	int cYellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
	
#else

	int hConsole = 0;
	int m_currentConsoleAttr;
	
	int cWhite = 0;
	int cGray = 0;
	int cRed = 0;
	int cCyan = 0;
	int cBlue = 0;
	int cMagenta = 0;
	int cYellow = 0;
	
#endif

	if (Path.GetCount()) {
		if (!ER::NoColor)
			SetConsoleTextAttribute(hConsole, cGray);
		stream << Path << ": ";
	}
	
	ER::PrettyPrint(Error, stream, !ER::NoColor);
}

void ER::PrettyPrint(const String& error, Stream& stream, bool color) {
#ifdef PLATFORM_WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
	WORD                        m_currentConsoleAttr;
	CONSOLE_SCREEN_BUFFER_INFO   csbi;

	//retrieve and save the current attributes
	if(GetConsoleScreenBufferInfo(hConsole, &csbi))
	    m_currentConsoleAttr = csbi.wAttributes;

	int cWhite = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
	int cGray = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	int cDkGray = FOREGROUND_INTENSITY;
	int cRed = FOREGROUND_RED | FOREGROUND_INTENSITY;
	int cCyan = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	int cBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
	int cMagenta = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;
	int cGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
	int cYellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
	
#else

	int hConsole = 0;
	int m_currentConsoleAttr;
	
	int cWhite = 0;
	int cGray = 0;
	int cDkGray = 0;
	int cRed = 0;
	int cCyan = 0;
	int cBlue = 0;
	int cMagenta = 0;
	int cYellow = 0;
	
#endif
	
	String input = error;
	
	int index = input.Find("${");
		
	if (index == -1) {
		if (color)
			SetConsoleTextAttribute(hConsole, cWhite);
		stream << input;
		if (color)
			SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
		return;
	}
	
	const char* colstr[] = { "${white}", "${gray}", "${red}", "${cyan}", "${blue}", "${green}", "${yellow}", "${magenta}", "${dkgray}"};
	int         colval[] = { cWhite,     cGray,     cRed,     cCyan,     cBlue,     cGreen,     cYellow,     cMagenta,     cDkGray };
	
	if (color)
		SetConsoleTextAttribute(hConsole, cWhite);
	
	while (index != -1) {
		String pre = input.Mid(0, index);
		stream << pre;
		String post = input.Mid(index);
		
		for (int i = 0; i < __countof(colstr); i++) {
			if (post.StartsWith(colstr[i])) {
				post = post.Mid(strlen(colstr[i]));
				
				if (color)
					SetConsoleTextAttribute(hConsole, colval[i]);
				
				int index2 = post.Find("${");
				
				if (index2 == -1) {
					stream << post;
					index = index2;
				}
				else {
					String mid = post.Mid(0, index2);
					stream << mid;
					
					input = post.Mid(index2);
					index = input.Find("${");
				}
			}
		}
	}
	
	if (color)
		SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
}

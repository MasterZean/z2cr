#include "z2cr.h"

bool ER::PrintPath;

ErrorColorType ER::ErrorColor = ErrorColorType::None;
String ER::White = "${white}";
String ER::Gray = "${gray}";
String ER::DkGray = "${dkgray}";
String ER::Red = "${red}";
String ER::Cyan = "${cyan}";
String ER::Blue = "${blue}";
String ER::Green = "${green}";
String ER::Yellow = "${yellow}";
String ER::Magenta = "${magenta}";

#define CSI "\x1b["

String ER::ToColor(const ZNamespace& ns, bool qt) {
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
		throw ZException(String().Cat() << src.Path << "(" << p.x << ", " << p.y << ")", text);
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

ZException ER::ErrMethodDeclarationExpected(const ZSourcePos& p) {
	return ZException(p.ToString(), "method declaration expected");
}

ZException ER::ErrConstructorDeclarationExpected(const ZSourcePos& p) {
	return ZException(p.ToString(), "constructor declaration expected");
}

ZException ER::ErrCantOverrideDefaultBehavior(const ZSourcePos& p) {
	return ZException(p.ToString(), "can't override default behaviour for build-in system types");
}

void ER::ErrCArrayWrongArgumentNo(const ZSource& source, const Point& p, const ZClass& cls, int argNo) {
	String err = "template " + ToColor(cls, true) + " must be instanced with 1 or 2 arguments";
				err << ", " << IntStr(argNo) << " provided.";
				
	Error(source, p, err);
}

void ER::CantInstantiateTemplate(const ZSourcePos& p, ZClass& cls) {
	String err = "generic " + ToColor(cls, true) + " can't be instantiated without specialization";
	Error(*p.Source, p.P, err);
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
		if (def) {
			if (cons == 0)
				s << ER::Green;
			else
				s << ER::Magenta;
			s << def->Name << ER::White;
		}
		else
			s << owner.Name;
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

String DeQtfLf(const char *s) {
	StringBuffer r;
	while(*s) {
		if((byte)*s > ' ' && !IsDigit(*s) && !IsAlpha(*s) && (byte)*s < 128)
			r.Cat('`');
		if((byte)*s >= ' ')
			r.Cat(*s);
		else
		if(*s == '\n')
			r.Cat('&');
		else
		if(*s == '\t')
			r.Cat("-|");
		s++;
	}
	return String(r);
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
		if (ER::ErrorColor == ErrorColorType::Win32) {
			SetConsoleTextAttribute(hConsole, cGray);
			stream << Path << ": ";
		}
		else if (ER::ErrorColor == ErrorColorType::Ansi)
			stream << CSI << "37m";
		else if (ER::ErrorColor == ErrorColorType::Qtf) {
			stream << "\1[@1 ";
			stream << DeQtfLf(Path) << ": ";
			stream << "]";
		}
		else
			stream << Path << ": ";
	}
	
	ER::PrettyPrint(Error, stream, ER::ErrorColor);
}

void ER::PrettyPrint(const String& error, Stream& stream, ErrorColorType color) {
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
		if (ER::ErrorColor == ErrorColorType::Win32)
			SetConsoleTextAttribute(hConsole, cWhite);
		stream << input;
		if (ER::ErrorColor == ErrorColorType::Win32)
			SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
		return;
	}
	
	const char* colstr[] = { "${white}", "${gray}", "${red}", "${cyan}", "${blue}", "${green}", "${yellow}", "${magenta}", "${dkgray}"};
	int         colval[] = { cWhite,     cGray,     cRed,     cCyan,     cBlue,     cGreen,     cYellow,     cMagenta,     cDkGray };
	const char* ansist[] = { "97m",      "37m",     "91m",    "96m",     "94m",     "92m",      "93m",      "95m",         "37m" };
	const char* qtfpre[] = { "[@2",      "[@1",     "[@R",    "[@8",     "[@B",     "[@G",      "[@9",      "[@M",         "[@K" };
	
	if (ER::ErrorColor == ErrorColorType::Win32)
		SetConsoleTextAttribute(hConsole, cWhite);
	
	while (index != -1) {
		String pre = input.Mid(0, index);
		stream << pre;
		String post = input.Mid(index);
		
		for (int i = 0; i < __countof(colstr); i++) {
			if (post.StartsWith(colstr[i])) {
				post = post.Mid(strlen(colstr[i]));
				
				if (ER::ErrorColor == ErrorColorType::Win32)
					SetConsoleTextAttribute(hConsole, colval[i]);
				else if (ER::ErrorColor == ErrorColorType::Ansi)
					stream << CSI << ansist[i];
				else if (ER::ErrorColor == ErrorColorType::Qtf)
					stream << qtfpre[i] << " ";
				
				int index2 = post.Find("${");
				
				if (index2 == -1) {
					if (ER::ErrorColor == ErrorColorType::Qtf)
						stream << DeQtfLf(post);
					else
						stream << post;
					index = index2;
				}
				else {
					String mid = post.Mid(0, index2);
					if (ER::ErrorColor == ErrorColorType::Qtf)
						stream << DeQtfLf(mid);
					else
						stream << mid;
					
					input = post.Mid(index2);
					index = input.Find("${");
				}
				
				if (ER::ErrorColor == ErrorColorType::Qtf)
					stream << "]";
			}
		}
	}
	
	if (ER::ErrorColor == ErrorColorType::Win32)
		SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
}

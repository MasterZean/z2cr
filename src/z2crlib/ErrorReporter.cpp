#include "z2cr.h"

bool ER::PrintPath;

bool ER::NoColor = false;
String ER::White = "${white}";
String ER::Gray = "${gray}";
String ER::Red = "${red}";
String ER::Cyan = "${cyan}";
String ER::Blue = "${blue}";
String ER::Green = "${green}";
String ER::Yellow = "${yellow}";

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
	int cBlue = FOREGROUND_BLUE | FOREGROUND_GREEN;
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
	int cYellow = 0;
	
#endif

	if (Path.GetCount()) {
		if (!ER::NoColor)
			SetConsoleTextAttribute(hConsole, cGray);
		stream << Path << ": ";
	}
	
	String input = Error;
	
	int index = input.Find("${");
		
	if (index == -1) {
		if (!ER::NoColor)
			SetConsoleTextAttribute(hConsole, cWhite);
		stream << input;
		if (!ER::NoColor)
			SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
		return;
	}
	
	const char* colstr[] = { "${white}", "${gray}", "${red}", "${cyan}", "${blue}", "${green}", "${yellow}"};
	int         colval[] = { cWhite,     cGray,     cRed,     cCyan,     cBlue,     cGreen,     cYellow };
	
	if (!ER::NoColor)
		SetConsoleTextAttribute(hConsole, cWhite);
	
	while (index != -1) {
		String pre = input.Mid(0, index);
		stream << pre;
		String post = input.Mid(index);
		
		for (int i = 0; i < __countof(colstr); i++) {
			if (post.StartsWith(colstr[i])) {
				post = post.Mid(strlen(colstr[i]));
				
				if (!ER::NoColor)
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
	
	if (!ER::NoColor)
		SetConsoleTextAttribute(hConsole, m_currentConsoleAttr);
}

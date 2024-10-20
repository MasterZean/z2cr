#include "z2cr.h"

extern char SingleOp[24];
extern char DoubleOpCh1[9];
extern char DoubleOpCh2[9];

char SingleOp[24] = { '+', '-', '*', '/', '=', ';', '(', ')', '.', '<', '>', '&', ',', '%', '|', '^', ':', '!', '[', ']', '@', '~', '?', '#' };
char DoubleOpCh1[9]  = { '<', '>', '=', '!', '<', '>', ':', '+', '-' };
char DoubleOpCh2[9]  = { '<', '>', '=', '=', '=', '=', ':', '+', '-' };

bool ZParser::IsZId() {
	if (term[0] == 'a' && IsId0("alias"))
		return false;
	else if (term[0] == 'b' && IsId0("break"))
		return false;
	else if (term[0] == 'c' && IsId0("case"))
		return false;
	else if (term[0] == 'c' && IsId0("catch"))
		return false;
	else if (term[0] == 'c' && IsId0("const"))
		return false;
	else if (term[0] == 'c' && IsId0("continue"))
		return false;
	else if (term[0] == 'c' && IsId0("class"))
		return false;
	else if (term[0] == 'd' && IsId0("default"))
		return false;
	else if (term[0] == 'd' && IsId0("def"))
		return false;
	else if (term[0] == 'd' && IsId0("do"))
		return false;
	else if (term[0] == 'e' && IsId0("enum"))
		return false;
	else if (term[0] == 'e' && IsId0("else"))
		return false;
	else if (term[0] == 'f' && IsId0("for"))
		return false;
	else if (term[0] == 'f' && IsId0("finally"))
		return false;
	else if (term[0] == 'f' && IsId0("foreach"))
		return false;
	else if (term[0] == 'f' && IsId0("func"))
		return false;
	else if (term[0] == 'g' && IsId0("goto"))
		return false;
	else if (term[0] == 'g' && IsId0("get"))
		return false;
	else if (term[0] == 'i' && IsId0("if"))
		return false;
	else if (term[0] == 'i' && IsId0("in"))
		return false;
	else if (term[0] == 'm' && IsId0("move"))
		return false;
	else if (term[0] == 'n' && IsId0("new"))
		return false;
	else if (term[0] == 'n' && IsId0("namespace"))
		return false;
	else if (term[0] == 'o' && IsId0("override"))
		return false;
	else if (term[0] == 'p' && IsId0("private"))
		return false;
	else if (term[0] == 'p' && IsId0("protected"))
		return false;
	else if (term[0] == 'p' && IsId0("public"))
		return false;
	else if (term[0] == 'p' && IsId0("property"))
		return false;
	else if (term[0] == 'r' && IsId0("ref"))
		return false;
	else if (term[0] == 'r' && IsId0("return"))
		return false;
	else if (term[0] == 's' && IsId0("static"))
		return false;
	else if (term[0] == 's' && IsId0("set"))
		return false;
	else if (term[0] == 's' && IsId0("switch"))
		return false;
	else if (term[0] == 't' && IsId0("this"))
		return false;
	else if (term[0] == 't' && IsId0("try"))
		return false;
	else if (term[0] == 't' && IsId0("throw"))
		return false;
	else if (term[0] == 'u' && IsId0("using"))
		return false;
	else if (term[0] == 'v' && IsId0("virtual"))
		return false;
	else if (term[0] == 'v' && IsId0("val"))
		return false;
	else if (term[0] == 'w' && IsId0("while"))
		return false;
	else if (term[0] == 'w' && IsId0("with"))
		return false;
	
	return IsId();
}

String ZParser::ExpectId() {
	if (IsId())
		return ReadId();
	else {
		Point p = GetPoint();
		Error(p, "identifier expected, " + Identify() + " found");

		return "";
	}
}

String ZParser::ExpectZId() {
	if (IsZId())
		return ReadId();
	else {
		Point p = GetPoint();
		Error(p, "identifier expected, " + Identify() + " found");

		return "";
	}
}

String ZParser::ExpectId(const String& id) {
	if (IsId(id))
		return ReadId();
	else {
		Point p = GetPoint();
		Error(p, "'" + id + "' expected, " + Identify() + " found");

		return "";
	}
}

void ZParser::Expect(char ch) {
	if (!Char(ch)) {
		Point p = GetPoint();
		Error(p, "'" + (String().Cat() << ch) + "' expected, " + Identify() + " found");
	}
}

void ZParser::Expect(char ch, char ch2) {
	if (!Char2(ch, ch2)) {
		Point p = GetPoint();
		Error(p, "'" + (String().Cat() << ch << ch2) + "' expected, " + Identify() + " found");
	}
}

void ZParser::ExpectEndStat() {
	Expect(';');
}

void ZParser::Error(const Point& p, const String& text) {
	throw ZException(String().Cat() << source.Path() << "(" << p.x << ", " << p.y << ")" << Mode, text);
	//Cout() << source.Path() << "(" << p.x << ", " << p.y << "): " << text << "\n";
}

String ZParser::Identify() {
	if (IsId("true"))
		return "boolean constant 'true'";
	else if (IsId("false"))
		return "boolean constant 'false'";
	else if (IsInt())
		return "integer constant '" + IntStr(ReadInt()) + "'";
	else if (IsEof())
		return "end-of-file";
	else if (IsZId())
		return "identifier '" + ReadId() + "'";
	else if (IsId())
		return "keyword '" + ReadId() + "'";
	else if (IsString())
		return "string constant";
	else if (term[0] == '\n')
		return "end of statement";
	else {
		for (int i = 0; i < 9; i++)
			if (IsChar2(DoubleOpCh1[i], DoubleOpCh2[i])) {
				char c[3] = "  ";
				c[0] = DoubleOpCh1[i];
				c[1] = DoubleOpCh2[i];
			    return c;
			}
		for (int i = 0; i < 24; i++)
			if (IsChar(SingleOp[i])) {
				char c[2] = " ";
				c[0] = SingleOp[i];
			    return c;
			}
			
		return "unexpected token";
	}
}
#include "z2cr.h"

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
	//else if (term[0] == 'g' && IsId0("get"))
	//	return false;
	else if (term[0] == 'i' && IsId0("if"))
		return false;
	//else if (term[0] == 'i' && IsId0("in"))
	//	return false;
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
	//else if (term[0] == 's' && IsId0("set"))
	//	return false;
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
	if (IsId()) {
		if (!IsZId()) {
			Point p = GetPoint();
			Error(p, "identifier expected, " + Identify() + " found");
		}
		else if (IsId("true") || IsId("false")) {
			Point p = GetPoint();
			Error(p, "identifier expected, " + Identify() + " found");
		}
		return ReadId();
	}
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
	throw ZException(String().Cat() << source.Path << "(" << p.x << ", " << p.y << ")" << Mode, text);
	//Cout() << source.Path() << "(" << p.x << ", " << p.y << "): " << text << "\n";
}

void ZParser::Warning(const Point& p, const String& text) {
	Cout() << source.Path << "(" << p.x << ", " << p.y << ")" << Mode;
	Cout() << text << "\n";
}

String ZParser::Identify() {
	if (IsId("true"))
		return "boolean constant 'true'";
	else if (IsId("false"))
		return "boolean constant 'false'";
	else if (IsInt()) {
		int64 oInt;
		double oDub;
		
		int base = 10;
		int type = ReadInt64(oInt, oDub, base);
		
		if (type == ZParser::ntDouble || type == ZParser::ntFloat)
			return "floating point constant '" + DblStr(oDub) + "'";
		else
			return "integer constant '" + IntStr64(oInt) + "'";
	}
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
	else if (term[0] == '{')
		return "block starter '{'";
	else if (term[0] == '}')
		return "block finisher '}'";
	else if (term[0] == '\'')
		return "character constant";
	else {
		for (int i = 0; i < 9; i++)
			if (IsChar2(DoubleOpCh1[i], DoubleOpCh2[i])) {
				char c[3] = "  ";
				c[0] = DoubleOpCh1[i];
				c[1] = DoubleOpCh2[i];
			    return "'" + String(c) + "'";
			}
		for (int i = 0; i < 24; i++)
			if (IsChar(SingleOp[i])) {
				char c[2] = " ";
				c[0] = SingleOp[i];
			    return "'" + String(c) + "'";
			}
			
		return "unexpected token";
	}
}

ZParser::NumberType ZParser::ReadInt64(int64& oInt, double& oDub, int& base) {
	Point p = GetPoint();
	int sign = Sgn();
	
	if (*term == '0') {
		term++;
		if (*term == 'x' || *term == 'X') {
			term++;
			oInt = ReadNumber64Core(p, 16);
			base = 16;
			
			return ReadI(p, sign, oInt);
		}
		else if (*term == 'o' || *term == 'O') {
			term++;
			oInt = ReadNumber64Core(p, 8);
			base = 8;
			
			return ReadI(p, sign, oInt);
		}
		else if (*term == 'b' || *term == 'B') {
			term++;
			oInt = ReadNumber64Core(p, 2);
			base = 2;
			
			return ReadI(p, sign, oInt);
		}
		else if (IsAlNum(*term) && (*term != 'u' && *term != 's' && *term != 'l' && *term != 'p'))
			Error(p, "invalid numeric literal");
		else {
			if (*term == '.' && IsDigit(*(term + 1))) {
				oDub = 0;
				base = 10;
				
				return ReadF(p, sign, oDub);
			}
			else {
				oInt = 0;
				base = 10;
				if (IsDigit(*term))
					oInt = ReadNumber64Core(p, 10);
				
				return ReadI(p, sign, oInt);
			}
		}
		
		ASSERT(0);
		
		return ntInvalid;
	}
	else {
		base = 10;
		oInt = ReadNumber64Core(p, 10);
		
		if (*term == '.' && IsDigit(*(term + 1))) {
			oDub = (double)oInt;
			return ReadF(p, sign, oDub);
		}
		else
			return ReadI(p, sign, oInt);
	}
}

uint64 ZParser::ReadNumber64Core(Point& p, int base) {
	uint64 n = 0;
	int q = ctoi(*term);
	bool expect = false;
	
	if(q < 0 || q >= base)
		Error(p, "invalid numeric literal");
	
	for(;;) {
		if (*term == '\'') {
			term++;
			continue;
		}
		
		int c = ctoi(*term);
		if(c < 0 || c >= base)
			break;
		
		uint64 n1 = n;
		n = base * n + c;
		
		if(n1 > n)
			Error(p, "integer constant is too big");
		
		if ((int64)n < 0 && (int64)n != -9223372036854775808ll) {
			expect = true;
		}
		
		term++;
	}

	if (expect && term[0] != 'u' && term[2] != 'l')
		Error(p, "'QWord' literal constants require a 'ul' sufix");
		
	return n;
}

ZParser::NumberType ZParser::ReadI(Point& p, int sign, int64& oInt) {
	int64 i = oInt;
	if(sign > 0 ? i > INT64_MAX : i > (uint64)INT64_MAX + 1)
		Error(p, "integer constant is too big");

	bool l = false;
	bool u = false;
	bool ps = false;
	
	NumberType nt = ntInt;
	
	if (*term == 'l') {
		term++;
		l = true;
		if (IsAlNum(*term))
			Error(p, "invalid numeric literal");
		nt = ntLong;
	}
	else if (*term == 'u') {
		term++;
		u = true;
		nt = ntDWord;
		if (*term == 'l') {
			term++;
			l = true;
			if (IsAlNum(*term))
				Error(p, "invalid numeric literal");
			nt = ntQWord;
		}
		else if (IsAlNum(*term))
			Error(p, "invalid numeric literal");
	}
	else if (*term == 's') {
		term++;
		if (*term == 'l') {
			term++;
			l = true;
			if (IsAlNum(*term))
				Error(p, "invalid numeric literal");
			nt = ntLong;
		}
		else if (IsAlNum(*term))
			Error(p, "invalid numeric literal");
	}
	else if (*term == 'p') {
		term++;
		ps = true;
		nt = ntPtrSize;
	}
	if (u) {
		if (sign == -1)
			Error(p, "unsinged integer constants can't have a leading '-'");
		if (i > 4294967295 && !l)
			Error(p, "'QWord' literal constants require a 'l' sufix");
	}
	else {
		i = sign * i;
		if ((i < -2147483648LL || i > 2147483647) && !l)
			Error(p, "'Long' literal constants require a 'l' sufix");
	}
	
	oInt = i;
	Spaces();
	
	return nt;
}


ZParser::NumberType ZParser::ReadF(Point& p, int sign, double& oDub) {
	term++;
	
	double nf = oDub;
	double q = 1;
	
	while(IsDigit(*term)) {
		q = q / 10;
		nf += q * (*term++ - '0');
	}

	if(Char('e') || Char('E')) {
		int exp = ReadInt();
		long double ddd = pow(10.0, exp);
		nf *= ddd;
	}
	
	bool f = Char('f') || Char('F');
	nf = sign * nf;
	
	if(!IsFin(nf))
		Error(p, "floating point constant is too big");
	
	oDub = nf;
	Spaces();
	
	return f ? ntFloat : ntDouble;
}

uint32 ZParser::ReadChar() {
	if (term[0] != '\'')
		return -1;
	
	if (term[1] == '\\') {
		term += 2;
		uint32 c = -1;
		if (*term == 't') {
			c = '\t';
			term++;
		}
		else if (*term == 'n') {
			c = '\n';
			term++;
		}
		else if (*term == 'r') {
			c = '\r';
			term++;
		}
		else if (*term == 'a') {
			c = '\a';
			term++;
		}
		else if (*term == 'b') {
			c = '\b';
			term++;
		}
		else if (*term == 'f') {
			c = '\f';
			term++;
		}
		else if (*term == 'v') {
			c = '\v';
			term++;
		}
		else if (*term == '\'') {
			c = '\'';
			term++;
		}
		else if (*term == '0') {
			c = '\0';
			term++;
		}
		else if (*term == '\\') {
			c = '\\';
			term++;
		}
		else if (*term == 'u') {
			term++;
			c = 0;
			for(int i = 0; i < 6; i++) {
				uint32 cc = ctoi(*term);
				if(cc < 0 || cc >= 16)
					return -1;
				c = 16 * c + cc;
				term++;
			}
		}
		else
			return -1;
		
		if (*term != '\'')
			return -1;
		
		term += 1;
		Spaces();
		
		return c;
	}
	else {
		term += 1;
		uint32 c = (byte)*term;
		if (c == 0)
			return -1;
		if (c < 0x80)
			term += 1;
		else if (c < 0xC0)
			return -1;
		else if (c < 0xE0) {
			c = ((c - 0xC0) << 6) + (byte)term[1] - 0x80;
			term += 2;
		}
		else if (c < 0xF0) {
			c = ((c - 0xE0) << 12) + (((byte)term[1] - 0x80) << 6) + (byte)term[2] - 0x80;
			term += 3;
		}
		else if (c < 0xF5) {
			c = ((c - 0xE0) << 18) + (((byte)term[1] - 0x80) << 12) + (((byte)term[2] - 0x80) << 6) + (byte)term[3] - 0x80;
			term += 4;
		}
		
		if (*term != '\'')
			return -1;
		
		term += 1;
		Spaces();
		
		return c;
	}
}

bool ZParser::EatIf() {
	if (term[0] == '#' && term[1] == 'i' && term[2] == 'f') {
		term += 3;
		Spaces();
		return true;
	}
	return false;
}

bool ZParser::EatElse() {
	if (term[0] == '#' && term[1] == 'e' && term[2] == 'l' && term[3] == 's' && term[4] == 'e') {
		term += 5;
		Spaces();
		return true;
	}
	return false;
}

bool ZParser::EatEndIf() {
	if (term[0] == '#' && term[1] == 'e' && term[2] == 'n' && term[3] == 'd' && term[4] == 'i'  && term[5] == 'f') {
		term += 6;
		Spaces();
		return true;
	}
	return false;
}

bool ZParser::IsElse() {
	if (term[0] == '#' && term[1] == 'e' && term[2] == 'l' && term[3] == 's' && term[4] == 'e')
		return true;
	
	return false;
}

bool ZParser::IsEndIf() {
	if (term[0] == '#' && term[1] == 'e' && term[2] == 'n' && term[3] == 'd' && term[4] == 'i'  && term[5] == 'f')
		return true;

	return false;
}

void ZParser::SkipBlock() {
	while (true) {
		if (term[0] == '#' && term[1] == 'e' && term[2] == 'l' && term[3] == 's' && term[4] == 'e')
			return;
		else if (term[0] == '#' && term[1] == 'e' && term[2] == 'n' && term[3] == 'd' && term[4] == 'i'  && term[5] == 'f')
			return;
		else if (term[0] == 0)
			return;
		else if (term[0] == '\n') {
			line++;
			term++;
		}
		else
			term++;
	}
	
	Spaces();
}
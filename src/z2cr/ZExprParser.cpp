#include "z2cr.h"

extern String opss[];

Node* ZExprParser::Parse() {
	Node* left = ParseAtom();
	
	CParser::Pos backupPoint;
	Node* exp = ParseBin(0, left, backupPoint);
	
	return exp;
}

Node* ZExprParser::ParseBin(int prec, Node* left, CParser::Pos& backupPoint) {
	while (true) {
		int op, tempop;
		bool opc = false, tempopc;
		int p = GetPriority(op, opc);
		if (p < prec)
			return left;
		
		Point opp = parser.GetPoint();
		
		Node* right = nullptr;
		
		parser.GetChar();
		if (opc == true)
			parser.GetChar();
		parser.Spaces();
	
		right = ParseAtom();
		
		ASSERT(right->Tt.Class);
		int nextp = GetPriority(tempop, tempopc);
		if (p < nextp) {
			backupPoint = parser.GetPos();
			right = ParseBin(p + 1, right, backupPoint);
		}
		ASSERT(right->Tt.Class);
		
		if ((op == 3 || op == 4) && right->IsZero(ass))
			parser.Error(opp, "second operand of division is 0 or equivalent");
		
		Node* r = irg.op(left, right, OpNode::Type(op), opp);
		if (r == nullptr)
			IncompatOp(parser.Source(), opp, opss[op], left, right);
		ASSERT(r->Tt.Class);
		left = r;
	}
}

Node* ZExprParser::ParseAtom() {
	Point opp = parser.GetPoint();

	Node* exp = nullptr;
	
	if (parser.IsInt())
		exp = ParseNumeric();
	else if (parser.IsCharConst()) {
		uint32 ch = parser.ReadChar();
		if (ch == -1)
			parser.Error(opp, "illegal '\fChar\f' literal cosntant");
		exp = irg.const_char(ch);
	}
	else if (parser.Id("true"))
		exp = irg.const_bool(true);
	else if (parser.Id("false"))
		exp = irg.const_bool(false);
	else if (parser.Id("void"))
		exp = irg.const_void();
	else if (parser.Id("null"))
		exp = irg.const_null();
	else if (parser.IsId()) {
		exp = ParseId();
	}
	else if (parser.Char2(':', ':')) {
		exp = ParseNamespace();
	}
	else {
		parser.Error(opp, "expression expected, " + parser.Identify() + " found");
		return nullptr;
	}
	
	ASSERT(exp);
	
	return exp;
}

Node* ZExprParser::ParseId() {
	if (Section == nullptr || Section->Using.GetCount() == 0)
		return ParseNamespace();
	
	Point opp = parser.GetPoint();
	String s;
	if (parser.Char('@'))
		s = "@" + parser.ExpectId();
	else
		s = parser.ExpectId();
	
	Node* member = nullptr;
	for (int i = 0; i < Section->Using.GetCount(); i++) {
		ZNamespace& ns = *Section->Using[i];
		
		Node* res = ParseMember(ns, s, opp);
		
		if (member)
			ErrorReporter::Error(parser.Source(), opp, s + ": ambigous symbol");
		
		member = res;
	}
	
	if (member)
		return member;
	else
		return ParseNamespace(s, opp);
}

Node* ZExprParser::ParseNamespace() {
	Point opp = parser.GetPoint();
	auto s = parser.ReadId();
	
	return ParseNamespace(s, opp);
}

Node* ZExprParser::ParseNamespace(const String& s, Point opp) {
	ZNamespaceItem* ns = &ass.NsLookup;
	
	auto total = s;
	
	int index = ns->Names.Find(s);
		
	if (index == -1) {
		// TODO: refactor
		Assembly dass;
		ZPackage& dpak = dass.AddPackage("main", "");
		ZSource& dsrc = dpak.AddSource("test", false);
		dsrc.LoadVirtual(s);
		ZParser dummy(dsrc);
		
		if (!dummy.IsZId())
			parser.Error(opp, "unexpected keyword: " + s);
		else
			parser.Error(opp, "unknown identifier: " + s);
	}
	ns = &ns->Names[index];
	
	opp = parser.GetPoint();
	
	if (!parser.IsChar('.'))
		parser.Error(opp, "namespace element must be folowed by '.', " + parser.Identify() + " found");

	while (parser.Char('.')) {
		String name;
		
		if (parser.Char('@')) {
			// only namespace member
			name = "@" + parser.ExpectId();
			
			ASSERT(ns->Namespace);
			return ParseMember(*ns->Namespace, name, opp);
		}
		else {
			name = parser.ExpectId();
			
			index = ns->Names.Find(name);
			
			if (index != -1) {
				// still a namespace, ask for more '.'
				ns = &ns->Names[index];
				opp = parser.GetPoint();
				if (/*!ns->Namespace &&*/ !parser.IsChar('.'))
					parser.Error(opp, "namespace element must be folowed by '.', " + parser.Identify() + " found");
			}
			else {
				// a namespace child
				return ParseMember(*ns->Namespace, name, opp);
			}
		}
		
		total << "." << name;
	}
		
	//TODO: fix
	return irg.const_void();
}

Node *ZExprParser::ParseMember(ZNamespace& ns, const String& aName, const Point& opp) {
	int index = ns.Methods.Find(aName);
	
	if (index != -1) {
		parser.Expect('(');
		parser.Expect(')');
	
		return irg.mem_def(*ns.Methods[index].Functions[0], nullptr);
	}
	
	index = ns.Variables.Find(aName);
	if (index != -1) {
		return irg.mem_var(ns.Variables[index]);
	}
	
	if (index == -1)
		parser.Error(opp, "namespace '" + ns.Name + "' does not have a member called: '" + aName + "'");
	
	return nullptr;
}

Node* ZExprParser::ParseNumeric() {
	Node* exp = nullptr;
	
	int64 oInt;
	double oDub;
	
	int base = 10;
	int type = parser.ReadInt64(oInt, oDub, base);

	if (type == ZParser::ntInt)
		exp = irg.const_i(oInt, nullptr, base);
	else if (type == ZParser::ntDWord)
		exp = irg.const_u(oInt, nullptr, base);
	else if (type == ZParser::ntLong) {
		exp = irg.const_i(oInt);
		exp->Tt = ass.CLong->Tt;
	}
	else if (type == ZParser::ntQWord) {
		exp = irg.const_u(oInt);
		exp->Tt = ass.CQWord->Tt;
	}
	else if (type == ZParser::ntDouble)
		exp = irg.const_r64(oDub);
	else if (type == ZParser::ntFloat)
		exp = irg.const_r32(oDub);
	else if (type == ZParser::ntPtrSize) {
		exp = irg.const_i(oInt);
		exp->Tt = ass.CPtrSize->Tt;
	}
	else
		ASSERT_(0, "Error in parse int");

	return exp;
}

int ZExprParser::GetPriority(int& op, bool& opc) {
	Point p = Point(-1, -1);

	if (parser.IsChar3('<', '<', '='))
		return -1;
	if (parser.IsChar3('>', '>', '='))
		return -1;
	if (parser.IsChar2('+', '='))
		return -1;
	if (parser.IsChar2('-', '='))
		return -1;
	if (parser.IsChar2('<', '-'))
		return -1;
	if (parser.IsChar2('*', '='))
		return -1;
	if (parser.IsChar2('/', '='))
		return -1;
	if (parser.IsChar2('%', '='))
		return -1;
	if (parser.IsChar2('&', '='))
		return -1;
	if (parser.IsChar2('^', '='))
		return -1;
	if (parser.IsChar2('|', '='))
		return -1;
	else if (parser.IsChar2('<', '<'))
		p = Point( 5, 210 - 20);
	else if (parser.IsChar2('>', '>'))
		p = Point( 6, 210 - 20);
	else if (parser.IsChar2('<', '='))
		p = Point( 8, 210 - 30);
	else if (parser.IsChar2('>', '='))
		p = Point(10, 210 - 30);
	else if (parser.IsChar2('=', '='))
		p = Point(11, 210 - 40);
	else if (parser.IsChar2('!', '='))
		p = Point(12, 210 - 40);
	else if (parser.IsChar2('&', '&'))
		p = Point(16, 210 - 80);
	else if (parser.IsChar2('|', '|'))
		p = Point(17, 210 - 90);

	if (p.y != -1)
		opc = true;
	else
		p = OPS[parser.PeekChar()];

	op = p.x;
	return p.y;
}
	
void ZExprParser::Initialize() {
	if (initialized == true)
		return;
	
	for (int i = 0; i < 255; i++)
		OPS[i] = Point(-1, -1);
	OPS['*'] = Point( 2, 210 -  0);
	OPS['/'] = Point( 3, 210 -  0);
	OPS['%'] = Point( 4, 210 -  0);
	OPS['+'] = Point( 0, 210 - 10);
	OPS['-'] = Point( 1, 210 - 10);
	OPS['<'] = Point( 7, 210 - 30);
	OPS['>'] = Point( 9, 210 - 30);
	OPS['&'] = Point(13, 210 - 50);
	OPS['^'] = Point(14, 210 - 60);
	OPS['|'] = Point(15, 210 - 70);
	
	Zero(OPCONT);
	OPCONT['('] = true;
	OPCONT['{'] = true;
	OPCONT['.'] = true;
	OPCONT['*'] = true;
	OPCONT['+'] = true;
	OPCONT['-'] = true;
	OPCONT['<'] = true;
	OPCONT['['] = true;
	
	initialized = true;
}

Point ZExprParser::OPS[256];
bool ZExprParser::OPCONT[256];
bool ZExprParser::initialized = false;




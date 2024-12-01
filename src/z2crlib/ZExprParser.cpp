#include "z2cr.h"
#include "OverloadResolver.h"

extern String opss[];

String CLS_STR = "class";

Node* ZExprParser::Parse() {
	Point p = parser.GetPoint();
	Node* left = ParseAtom();
	
	CParser::Pos backupPoint;
	Node* exp = ParseBin(0, left, backupPoint);
	
	if (parser.Char('?')) {
		if (exp->Tt.Class != ass.CBool)
			parser.Error(p, "ternary operator '?': first operand must have type " + ass.ToQtColor(ass.CBool));

		left = Parse();
		parser.Expect(':');
		Node* right = Parse();

		// TODO
		if (left->Tt.Class != right->Tt.Class)//!TypesEqualDeep(ass, &left->Tt, &right->Tt))
			parser.Error(p, "ternary operator '?': second and third operand must have the same type, but they are " +
					ass.ToQtColor(left) + " and " + ass.ToQtColor(right));

		exp = irg.opTern(exp, left, right);
	}
	
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
			parser.Error(opp, "illegal " + ass.ToQtColor(ass.CChar) + " literal constant");
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
	else if (parser.IsString())
		exp = irg.const_str(ass.AddStringConst(parser.ReadString()));
	else if (parser.Char2(':', ':')) {
		exp = ParseNamespace();
	}
	else if (parser.Char2('+', '+')) {
		Point p = parser.GetPoint();
		exp = ParseAtom();
		if (!exp->IsLValue())
			parser.Error(p, "expression is not a l-value, can't apply operator prefix '++'");
		Node* op = irg.inc(exp, true);
		exp = op;
		
		ASSERT(exp->Tt.Class);
		return exp;
	}
	else if (parser.Char2('-', '-')) {
		Point p = parser.GetPoint();
		exp = ParseAtom();
		if (!exp->IsLValue())
			parser.Error(p, "expression is not a l-value, can't apply operator prefix '--'");
		Node* op = irg.dec(exp, true);
		exp = op;
		
		ASSERT(exp->Tt.Class);
		return exp;
	}
	else if (parser.Char('-')) {
		Node* node = ParseAtom();
		exp = irg.minus(node);
		if (exp == nullptr)
			parser.Error(opp, "can't apply unary '-' ('@minus') on type " + ass.ToQtColor(node));
	}
	else if (parser.Char('+')) {
		Node* node = ParseAtom();
		exp = irg.plus(node);
		if (exp == nullptr)
			parser.Error(opp, "can't apply unary '+' ('@plus') on type " + ass.ToQtColor(node));
	}
	else if (parser.Char('!')) {
		Node* node = ParseAtom();
		exp = irg.op_not(node);
		if (exp == nullptr)
			parser.Error(opp, "can't apply unary '!' ('@not') on type " + ass.ToQtColor(node));
	}
	else if (parser.Char('~')) {
		Node* node = ParseAtom();
		exp = irg.bitnot(node);
		if (exp == nullptr)
			parser.Error(opp, "can't apply unary '~' ('@bitnot') on type " + ass.ToQtColor(node));
	}
	else if (parser.Char('(')) {
		Node* node = Parse();
		parser.Expect(')');
		exp = irg.list(node);
	}
	else {
		parser.Error(opp, "expression expected, " + parser.Identify() + " found");
		return nullptr;
	}
	
	ASSERT(exp);
	
	while (OPCONT[parser.PeekChar()]) {
		Point p = parser.GetPoint();
		
		if (parser.Char('(')) {
			// TODO: fix
			parser.Error(p, "syntax error");
		}
		else if (parser.Char('{')) {
			if (exp->IsLiteral && exp->Tt.Class == ass.CClass) {
				ZClass& cobj = ass.Classes[(int)exp->IntVal];
				
				Vector<Node*> params;
				getParams(params, '}');
				
				Node* temp = Temporary(ass, irg, cobj, params/*, p*/);
				exp = temp;
			}
		}
		else if (parser.Char('.')) {
			exp = ParseDot(exp);
		}
		else if (parser.Char2('+', '+')) {
			if (!exp->IsLValue())
				parser.Error(p, "expression is not a l-value, can't apply operator postfix '++'");
			Node* op = irg.inc(exp);
			exp = op;
			ASSERT(exp->Tt.Class);
		}
		else if (parser.Char2('-', '-')) {
			if (!exp->IsLValue())
				parser.Error(p, "expression is not a l-value, can't apply operator postfix '--'");
			Node* op = irg.dec(exp);
			exp = op;
			ASSERT(exp->Tt.Class);
		}
		else
			break;
		
		ASSERT(exp);
		ASSERT(exp->Tt.Class);
	}
	
	return exp;
}

Node* ZExprParser::ParseId() {
	Point opp = parser.GetPoint();
	String s;
	if (parser.Char('@'))
		s = "@" + parser.ExpectId();
	else
		s = parser.ExpectId();
	
//	if (s == "Test")
//		s == "Int";
	
	if (Function) {
		for (int j = 0; j < Function->Params.GetCount(); j++) {
			if (Function->Params[j].Name == s) {
				auto node = irg.mem_var(Function->Params[j]);
				node->IsLocal = true;
				return node;
			}
		}
		
		for (int j = 0; j < Function->Blocks.GetCount(); j++) {
			ZBlock& b = Function->Blocks[j];
			for (int k = 0; k < b.Locals.GetCount(); k++)
				if (b.Locals[k]->Name == s) {
					auto node = irg.mem_var(*b.Locals[k]);
					node->IsLocal = true;
					return node;
				}
		}
	}
	
	if (Class != nullptr) {
		Node* node = ParseMember(*Class, s, opp, false);
		
		if (node)
			return node;
	}
	
	if (Section != nullptr && Namespace != nullptr && Section->Using.GetCount() == 0) {
		Node* node = ParseMember(*Namespace, s, opp, true);
		
		if (node == nullptr) {
			int index = parser.Source().ShortNameLookup.Find(s);
			
			if (index != -1)
				node = irg.const_class(*parser.Source().ShortNameLookup[index]);
		}
		
		if (node)
			return node;
	}
	
	int count = 0;
	for (int i = 0; i < Section->Using.GetCount(); i++) {
		ZNamespace& ns = *Section->Using[i];
		if (ns.HasMember(s))
			count++;
	}
	
	if (count > 1) {
		String err = s + ": ambigous symbol: can be found in multiple namespaces:\n";
		for (int i = 0; i < Section->Using.GetCount(); i++) {
			ZNamespace& ns = *Section->Using[i];
			if (ns.HasMember(s))
				err << "\t\t" << ns.Name.Mid(0, ns.Name.GetCount() - 1) << "\n";
		}
		parser.Error(opp, err);
	}
	
	Node* member = nullptr;
	
	if (count > 0) {
		for (int i = 0; i < Section->Using.GetCount(); i++) {
			ZNamespace& ns = *Section->Using[i];
			
			if (member == nullptr)
				member = ParseMember(ns, s, opp, true);
		}
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
			Node* node = ParseMember(*ns->Namespace, name, opp, true);
			if (!node)
				parser.Error(opp, "namespace '" + ns->Namespace->Name + "' does not have a member called: '" + name + "'");
			return node;
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
				ASSERT(ns->Namespace);
				int clsIndex = ns->Namespace->Classes.Find(name);
				
				if (clsIndex != -1) {
					return irg.const_class(*ns->Namespace->Classes[clsIndex]);
				}
				else {
					Node* node = ParseMember(*ns->Namespace, name, opp, true);
					
					if (!node)
						parser.Error(opp, "namespace '" + ns->Namespace->Name + "' does not have a member called: '" + name + "'");
					return node;
				}
			}
		}
		
		total << "." << name;
	}
		
	return nullptr;
}

Node* ZExprParser::ParseMember(ZNamespace& ns, const String& aName, const Point& opp, bool onlyStatic, Node* object) {
	int index = ns.Methods.Find(aName);
	
	if (index != -1) {
		parser.Expect('(');
		
		Vector<Node*> params;
		getParams(params);
		
		bool ambig = false;
		ZFunction* f = GetBase(&ns.Methods[index], nullptr, params, 1, false, ambig);
		
		if (!f)
			ER::CallError(parser.Source(), opp, ass, ns, &ns.Methods[index], params, 0/*ol->IsCons*/);
		
		if (ambig)
			parser.Error(opp, ER::Green + aName + ": ambigous symbol");
		
		TestAccess(*f, opp);
		
		if (onlyStatic && !f->IsStatic)
			parser.Error(opp, ER::Green + aName + ER::White + ": is not a static member");
		if (!onlyStatic && f->IsStatic)
			parser.Error(opp, ER::Green + aName + ER::White + ": is a static member");
	 
		ParamsNode* node = irg.callfunc(*f, object);
		node->Params = std::move(params);
		return node;
	}
	
	index = ns.Variables.Find(aName);
	if (index != -1) {
		ZVariable& f = *ns.Variables[index];
		
		TestAccess(f, opp);
		if (onlyStatic && !f.IsStatic)
			parser.Error(opp, ER::Green + aName + ER::White + ": is not a static member");
		if (!onlyStatic && f.IsStatic)
			parser.Error(opp, ER::Green + aName + ER::White + ": is a static member");
		
		return irg.mem_var(f);
	}
	
	return nullptr;
}

Node *ZExprParser::ParseDot(Node *exp) {
	Point p = parser.GetPoint();
	String s;
	
	if (parser.Char('@'))
		s = "@" + parser.ExpectId();
	else
		s = parser.ExpectId();
	
	// case .class
	if (s == CLS_STR ) {
		if (exp->Tt.Class == ass.CClass) {
			if (!exp->IsLiteral)
				parser.Warning(p, "chained .${magenta}class${white} statements detected. "
					"Expression is equivalent to '${cyan}Class${white}.${magenta}class${white}' which is '${cyan}Class${white}'");
			if (exp->IntVal == 1)
				parser.Warning(p, "'${cyan}Class${white}.${magenta}class${white}' is '${cyan}Class${white}'. Is this intentional?");
			
			exp = irg.const_class(ass.Classes[(int)exp->IntVal], nullptr);
			exp->IsLiteral = false;

			ASSERT(exp->Tt.Class);
			return exp;
		}
		else {
			exp = irg.const_class(*exp->Tt.Class, exp);
			exp->IsLiteral = false;
			
			ASSERT(exp->Tt.Class);
			return exp;
		}
	}
	
	// static members
	if (exp->Tt.Class == ass.CClass) {
		if (exp->IsLiteral) {
			ZClass& cs = ass.Classes[(int)exp->IntVal];
			
			Node* node = ParseMember(cs, s, p, true);
			if (!node)
				parser.Error(p, "${magenta}class${white} '" + ER::Cyan + cs.Name + ER::White + "' does not have a member called: '" + ER::Green + s + ER::White + "'");
			return node;
		}
	}
	// instance members
	else {
		ZClass& cs = *exp->Tt.Class;
		Node* node = ParseMember(cs, s, p, false, exp);
		if (!node)
			parser.Error(p, "${magenta}class${white} '" + ER::Cyan + cs.Name + ER::White + "' does not have a member called: '" + ER::Green + s + ER::White + "'");
		return node;
	}
	
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

ZClass* ZExprParser::ParseType(Assembly& ass, ZParser& parser) {
	auto tt = parser.GetFullPos();
	String shtype = parser.ExpectId();
	String type = shtype;
	
	while (parser.Char('.'))
		type << "." << parser.ExpectId();
	
	ZClass* cls = nullptr;
	if (type.GetCount() == shtype.GetCount()) {
		// short name
		auto search = parser.Source().ShortNameLookup.FindPtr(shtype);
		if (search)
			cls = *search;
		
		if (cls == nullptr)
			ER::Error(parser.Source(), tt.P, "unknown identifier: " + type);
	}
	else {
		// full namespace
		cls = ass.Classes.FindPtr(type);
		
		if (cls == nullptr)
			ER::Error(parser.Source(), tt.P, "unknown namespace reference: " + type);
	}
	
	return cls;
}

ZFunction* ZExprParser::GetBase(ZMethodBundle* def, ZClass* spec, Vector<Node*>& params, int limit, bool conv, bool& ambig) {
	ZFunctionResolver res(ass);
	ZFunction* one = res.Resolve(*def, params, limit, spec, conv);
	ambig = res.IsAmbig();
	//score = res.Score();
	
	return one;
}

void ZExprParser::getParams(Vector<Node*>& params, char end) {
	while (!parser.IsChar(end)) {
		params.Add(Parse());
		
		if (parser.IsChar(end))
		    break;
		if (parser.IsChar(','))
			parser.Char(',');
	}
	
	parser.Expect(end);
}

Node* ZExprParser::Temporary(Assembly& ass, IR& irg, ZClass& cls, const Vector<Node*>& params/*, const Point& p*/) {
	Node* dr = nullptr;
	
	if (&cls == ass.CFloat) {
		if (params.GetCount() == 0)
			return irg.const_r32(0);
		else {
			dr = params[0];
			if (dr->IsRef)
				dr = irg.deref(dr);
			return irg.cast(dr, &cls.Tt);
		}
	}
	else if (&cls == ass.CDouble) {
		if (params.GetCount() == 0)
			return irg.const_r64(0);
		else {
			dr = params[0];
			if (dr->IsRef)
				dr = irg.deref(dr);
			return irg.cast(dr, &cls.Tt);
		}
	}
	else if (ass.IsNumeric(cls)) {
		if (params.GetCount() == 0) {
			Node *exp = irg.const_i(0);
			exp->Tt = cls.Tt;
			return exp;
		}
		else {
			dr = params[0];
			if (dr->IsRef)
				dr = irg.deref(dr);
			return irg.cast(dr, &cls.Tt);
		}
	}
	else {
		return irg.mem_temp(cls, nullptr);
	}
	/*else if (&cls == ass.CVoid)
		parser.Error(p, "'\fVoid\f' class can't be instantiated, use 'void' instead");
	else if (&cls == ass.CNull)
		parser.Error(p, "'\fNull\f' class can't be instantiated, use 'null' instead");
	else if (&cls == ass.CCls)
		parser.Error(p, "'\fClass\f' class can't be instantiated");
	else if (&cls == ass.CDef)
		parser.Error(p, "'\fDef\f' class can't be instantiated");*/
	
	return nullptr;
}

void ZExprParser::TestAccess(ZEntity& f, const Point& opp) {
	if (f.InClass == false) {
		if (f.Access == AccessType::Protected && &f.Namespace() != Namespace)
			parser.Error(opp, "can't access protected member:\n\t\t" + f.ColorSig() + "        " + "[" + f.OwnerSig() + "]");
		if (f.Access == AccessType::Private && &f.Namespace() != Namespace && &parser.Source() != f.DefPos.Source)
			parser.Error(opp, "can't access private member:\n\t\t" + f.ColorSig() + "        " + "[" + f.OwnerSig() + "]");
	}
	else {
		if (f.Access == AccessType::Protected && &f.Owner() != Class)
			parser.Error(opp, "can't access protected member:\n\t\t" + f.ColorSig() + "        " + "[" + f.OwnerSig() + "]");
		if (f.Access == AccessType::Private)
			parser.Error(opp, "can't access private member:\n\t\t" + f.ColorSig() + "        " + "[" + f.OwnerSig() + "]");
	}
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




#include "z2cr.h"
#include "OverloadResolver.h"

extern String opss[];
extern String strops[];

String CLS_STR = "class";
String THIS_STR = "this";

Node* ZExprParser::Parse(bool secondOnlyAttempt) {
	lastValid = nullptr;
	
	Point p = parser.GetPoint();
	Node* left = ParseAtom();
	ASSERT(left->Tt.Class);
	
	if (secondOnlyAttempt == false) {
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
						ass.ToQtColor(&left->Tt) + " and " + ass.ToQtColor(&right->Tt));
	
			exp = irg.opTern(exp, left, right);
		}
		
		return exp;
	}
	else {
		lastValid = left;
		
		CParser::Pos backupPoint = parser.GetPos();
		
		try {
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
							ass.ToQtColor(&left->Tt) + " and " + ass.ToQtColor(&right->Tt));
		
				exp = irg.opTern(exp, left, right);
			}
			
			return exp;
		}
		catch (...) {
			parser.SetPos(backupPoint);
			
			return lastValid;
		}
	}
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
			r = ResolveOpOverload(left, right, op, opp);
		
		lastValid = r;
		backupPoint = parser.GetPos();
		
		ASSERT(r->Tt.Class);
		left = r;
	}
}

Node* ZExprParser::ResolveOpOverload(Node* left, Node* right, int op, const Point& opp) {
	Node* r = GetOpOverloadStatic(left, right, op, opp);
	
	if (r == nullptr)
		r = GetOpOverloadStatic(right, left, op, opp);
	else
		return r;
	if (r == nullptr)
		r = GetOpOverload(left, right, op, opp);
	else
		return r;
	if (r == nullptr)
		r = GetOpOverload(right, left, op, opp);
	else
		return r;
	
	if (r == nullptr)
		ER::ErrIncompatOp(parser.Source(), opp, opss[op], ass.ToQtColor(&left->Tt), ass.ToQtColor(&right->Tt));
			
	return r;
}

Node* ZExprParser::GetOpOverload(Node* left, Node* right, int op, const Point& opp) {
	int index = left->Tt.Class->Methods.Find(strops[op]);
	
	if (index == -1)
		return nullptr;
	
	ZMethodBundle& method = left->Tt.Class->Methods[index];
	Vector<Node*> params;
	params.Add(right);
	
	bool ambig = false;
	ZFunction* f = GetBase(&method, nullptr, params, 1, false, ambig);
	
	if (!f)
		return nullptr;//ER::CallError(parser.Source(), opp, ass, *left->Tt.Class, &method, params, 0/*ol->IsCons*/);
	
	if (ambig)
		parser.Error(opp, ER::Green + strops[op] + ": ambigous symbol");

	TestAccess(*f, opp);
	
	ParamsNode* call = irg.callfunc(*f, left);
	call->Params = std::move(params);
	f->Owner().SetInUse();
	f->SetInUse();
	
	if (Function)
		Function->Dependencies.FindAdd(f);
	
	if (f->InClass && f->ShouldEvaluate())
		comp.CompileFunc(*f);
	
	return call;
}

Node* ZExprParser::GetOpOverloadStatic(Node* left, Node* right, int op, const Point& opp) {
	int index = left->Tt.Class->Methods.Find(strops[op]);
	
	if (index == -1)
		return nullptr;
	
	ZMethodBundle& method = left->Tt.Class->Methods[index];
	Vector<Node*> params;
	params.Add(left);
	params.Add(right);
	
	bool ambig = false;
	ZFunction* f = GetBase(&method, nullptr, params, 1, false, ambig);
	
	if (!f)
		return nullptr;
		
	if (ambig)
		parser.Error(opp, ER::Green + strops[op] + ": ambigous symbol");

	TestAccess(*f, opp);
	
	ParamsNode* call = irg.callfunc(*f, left);
	call->Params = std::move(params);
	f->Owner().SetInUse();
	f->SetInUse();
	
	if (Function)
		Function->Dependencies.FindAdd(f);
	
	if (f->InClass && f->ShouldEvaluate())
		comp.CompileFunc(*f);
	
	return call;
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
	else if (parser.Id("def")) {
		if (Function) {
			parser.Expect('.');
			Point p2 = parser.GetPoint();
			if (parser.IsId()) {
				String z = parser.ReadId();
				if (z == "class")
					return irg.const_class(*ass.CDef, nullptr);
				else if (z == "Name")
					return irg.const_str(ass.AddStringConst(Function->Name));
				else if (z == "Params") {
					parser.Expect('.');
					Point p3 = parser.GetPoint();
					String pp = parser.ReadId();
					if (pp == "Length")
						return irg.const_i(Function->Params.GetCount());
					else
						parser.Error(p3, "undefined member called '" + z + "'");
				}
				else
					parser.Error(p2, ER::ToColor(*ass.CDef, false) + " does not have a meber called '" + z + "'");
			}
			else if (parser.Char('@')) {
				String z = "@" + parser.ReadId();
				
				if (z == "@HasTrait") {
					parser.Expect('(');
					String trait = parser.ReadString();
					parser.Expect(')');
					
					int ti = FindIndex(Function->Trait.Traits, trait);
					
					return irg.const_bool(ti != -1);
				}
				else
					parser.Error(p2, ER::ToColor(*ass.CDef, false) + " does not have a meber called '" + z + "'");
			}
		}
		else {
			parser.Error(opp, "'def' encountered outside of method");
		}
	}
	else if (parser.Id("this")) {
		exp = irg.mem_this(*(ZClass*)Class);
	}
	else if (parser.Id("move")) {
		if (parser.Char('?'))
			;
		exp = Parse();
	}
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
		auto pp = parser.GetFullPos();
		Point p = parser.GetPoint();
		
		if (parser.Char('(')) {
			// TODO: fix
			parser.Error(p, "syntax error");
		}
		else if (parser.Char('{')) {
			if (exp->IsLiteral && exp->Tt.Class == ass.CClass) {
				ZClass& cobj = ass.Classes[(int)exp->IntVal];
				
				if (&cobj != ass.CPtr && cobj.IsTemplate)
					ER::CantInstantiateTemplate(pp, cobj);
				
				Vector<Node*> params;
				getParams(params, '}');
				
				if (cobj.FromTemplate)
					comp.Push(pp, cobj);
				
				Node* temp = Temporary(cobj, params, pp);
				
				if (cobj.FromTemplate)
					comp.Pop();
				
				if (cobj.TBase == ass.CRaw)
					temp->Tt.Param = exp->Tt.Param;
				exp = temp;
				
				exp->Tt.Class->SetInUse();
			}
		}
		else if (parser.IsChar('<') && !parser.IsChar2('<', '<') && exp->NT == NodeType::Const && exp->Tt.Class == ass.CClass && exp->IsLiteral) {
			exp = ParseAtomClassInst(exp);
		}
		else if (parser.Char('.')) {
			exp = ParseDot(exp);
		}
		else if (parser.Char('[')) {
			if (exp->Tt.Class == ass.CClass) {
				ZClass& vecClass = ass.Classes[(int)exp->IntVal];
				ASSERT(0);
			}
			else {
				Node* index = Parse();
				parser.Expect(']');
				
				if (/*exp->Tt.Class == ass.CPtr || */exp->Tt.Class->TBase == ass.CRaw || exp->Tt.Class->TBase == ass.CSlice
						|| (exp->Tt.Class->Super && exp->Tt.Class->Super->TBase == ass.CSlice)) {
					Node* temp = irg.mem_index(exp, index);
					if (temp == nullptr)
						parser.Error(p, "expression of type '" + ass.TypeToColor(exp->Tt) + "' does not have a '["
							+ ass.TypeToColor(index->Tt) + "]' operator defined");
					
					exp = temp;
				}
				else {
					parser.Error(p, "expression of type '" + ass.TypeToColor(exp->Tt) + "' does not have a '["
							+ ass.TypeToColor(index->Tt) + "]' operator defined");
				}
			}
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

Node* ZExprParser::ParseAtomClassInst(Node* exp) {
	auto posPreTemp = parser.GetFullPos();
			
	parser.Char('<');
	
	ZClass& mainClass = ass.Classes[(int)exp->IntVal];
	
	Vector<Node*> nodes;
	
	while (true) {
		Point posSub = parser.GetPoint();
		Node* sub = Parse(true);
		
		nodes << sub;
		
		if (!sub->IsCT)
			ER::ErrNotCompileTimeInst(parser.Source(), posSub);
		
		if (parser.Char(',')) {
		}
		else if (parser.IsChar('>'))
			break;
	}
		
	parser.Expect('>');
	
	if (mainClass.TBase == ass.CRaw) {
		if (nodes.GetCount() < 1 || nodes.GetCount() > 2)
			ER::ErrCArrayWrongArgumentNo12(parser.Source(), posPreTemp.P, mainClass, nodes.GetCount());
	}
	else {
		int target = mainClass.Scan.TName.GetCount();
	
		if (ass.IsPtr(mainClass.Tt))
			target = 1;
		
		if (mainClass.Tt.Class == ass.CRaw)
			target = 2;
			
		if (nodes.GetCount() != target)
			ER::ErrClassTemplateWrongArgumentNo(parser.Source(), posPreTemp.P, mainClass, target, nodes.GetCount());
	}
	
	return ParseSpec(mainClass, exp, nodes, posPreTemp);
}

Node* ZExprParser::ParseSpec(ZClass& mainClass, Node* exp, Vector<Node*>& nodes, const ZSourcePos& p) {
	ZClass& ownClass = ass.Classes[(int)exp->IntVal];
	
	// TODO: is needed?
	/*if (ass.IsPtr(ownClass.Tt)) {
		//if (!exp->IsRef && !exp->LValue)
		//	parser.Error(p, "can't take adress of this object");
		
		Node* ptr = irg.mem_ptr(exp, 0);
		ptr->IsLiteral = true;
		ptr->IsConst = true;
		ptr->IsCT = true;
		
		return irg.cast(ptr, &ass.Classes[(int)nodes[0]->IntVal].Pt);
	}*/
	
	int rawSize = -1;
	
	if (&ownClass == ass.CRaw) {
		if (nodes.GetCount() == 2) {
			Point ppp = parser.GetPoint();
			Node* nn = nodes[1];
			
			if (!nn->IsCT)
				ER::ErrNotCompileTimeInst(parser.Source(), ppp);
			
			if (!ass.IsInteger(nn->Tt)) {
				ZClass& sub = ass.Classes[(int)nodes[0]->IntVal];
				ZClass& inst = comp.ResolveInstance(ownClass, sub, p, true);
				ER::ErrItemCountNotInteger(parser.Source(), ppp, ass.ToQtColor(&inst.Tt));
			}
			
			if (ass.IsSignedInt(nn->Tt) && nn->IntVal <= 0) {
				ZClass& sub = ass.Classes[(int)nodes[0]->IntVal];
				ZClass& inst = comp.ResolveInstance(ownClass, sub, p, true);
				ER::ErrItemCountNegative(parser.Source(), ppp, ass.ToQtColor(&inst.Tt));
			}
			
			rawSize = (int)nn->IntVal;
		}
	}
	
	ZClass& sub = ass.Classes[(int)nodes[0]->IntVal];
	ZClass& inst = comp.ResolveInstance(ownClass, sub, p, true);
	
	exp->IntVal = inst.Index;
	exp->Tt.Param = rawSize;

	return exp;
}

Node* ZExprParser::ParseId() {
	auto opp = parser.GetFullPos();
	String s;
	if (parser.Char('@')) {
		String sub = parser.ExpectId();
		
		if (sub == "size") {
			if (Class) {
				Node* size = irg.intrinsic(irg.mem_this(*Class), 1);
				size->SetType(ass.CPtrSize->Tt);
				return size;
			}
			else
				;// TODO: fix
		}
		
		s = "@" + sub;
	}
	else
		s = parser.ExpectId();
	
	if (s == "f")
		s == "f";
	
	if (Function) {
		for (int j = 0; j < Function->Params.GetCount(); j++) {
			if (Function->Params[j].Name == s) {
				auto node = irg.mem_var(Function->Params[j], nullptr, true);
				return node;
			}
		}
		
		for (int j = 0; j < Function->Blocks.GetCount(); j++) {
			ZBlock& b = Function->Blocks[j];
			for (int k = 0; k < b.Locals.GetCount(); k++)
				if (b.Locals[k]->Name == s) {
					auto node = irg.mem_var(*b.Locals[k], nullptr, true);
					return node;
				}
		}
	}
	
	if (Class != nullptr) {
		if (Class->FromTemplate && Class->TBase && Class->TBase->Scan.TName[0] == s)
			return irg.const_class(*Class->T);
		
		Node* node = ParseMember(*Class, s, opp, false);
		
		if (node)
			return node;
		else if (Class->Super) {
			node = ParseMember(*Class->Super, s, opp, false);
			
			if (node)
				return node;
		}
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
		parser.Error(opp.P, err);
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
	auto opp = parser.GetFullPos();
	auto s = parser.ReadId();
	
	if (parser.PeekChar() != '.') {
		int index = parser.Source().ShortNameLookup.Find(s);
		
		if (index != -1)
			return irg.const_class(*parser.Source().ShortNameLookup[index]);
	}
	
	return ParseNamespace(s, opp);
}

Node* ZExprParser::ParseNamespace(const String& s, const ZSourcePos& opp) {
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
			parser.Error(opp.P, "unexpected keyword: " + s);
		else
			parser.Error(opp.P, "unknown identifier: " + s);
	}
	ns = &ns->Names[index];
	
	ZSourcePos opp2 = parser.GetFullPos();
	
	if (!parser.IsChar('.'))
		parser.Error(opp2.P, "namespace element must be folowed by '.', " + parser.Identify() + " found");

	while (parser.Char('.')) {
		String name;
		
		if (parser.Char('@')) {
			// only namespace member
			name = "@" + parser.ExpectId();
			
			ASSERT(ns->Namespace);
			Node* node = ParseMember(*ns->Namespace, name, opp2, true);
			if (!node)
				parser.Error(opp2.P, "namespace '" + ns->Namespace->Name + "' does not have a member called: '" + name + "'");
			
			return node;
		}
		else {
			auto opp3 = parser.GetFullPos();
			name = parser.ExpectId();
			
			index = ns->Names.Find(name);
			
			if (index != -1) {
				// still a namespace, ask for more '.'
				ns = &ns->Names[index];
				opp3 = parser.GetFullPos();
				if (/*!ns->Namespace &&*/ !parser.IsChar('.'))
					parser.Error(opp3.P, "namespace element must be folowed by '.', " + parser.Identify() + " found");
			}
			else {
				// a namespace child
				if (ns->Namespace == nullptr)
					parser.Error(opp3.P, "namespace element '" + name + " 'is not part of parent namespace");
				
				int clsIndex = ns->Namespace->Classes.Find(name);
				
				if (clsIndex != -1) {
					return irg.const_class(*ns->Namespace->Classes[clsIndex]);
				}
				else {
					Node* node = ParseMember(*ns->Namespace, name, opp3, true);
					
					if (!node)
						parser.Error(opp3.P, "namespace '" + ns->Namespace->Name + "' does not have a member called: '" + name + "'");
					
					return node;
				}
			}
		}
		
		total << "." << name;
	}
		
	return nullptr;
}

Node* ZExprParser::ParseMember(ZNamespace& ns, const String& aName, const ZSourcePos& opp, bool onlyStatic, Node* object) {
	int index = ns.Methods.Find(aName);
	
	if (aName == "f")
		aName == "f";
	
	if (index != -1) {
		ZMethodBundle& method = ns.Methods[index];
		Vector<Node*> params;
		
		if (method.IsProperty == false) {
			if (!method.IsConstructor) {
				// TODO: unsafe
				parser.Expect('(');
				getParams(params);
			}
			else {
				parser.Expect('{');
				getParams(params, '}');
			}
		}
		
		bool ambig = false;
		ZFunction* f = GetBase(&method, nullptr, params, 1, false, ambig);
		
		if (!f)
			ER::CallError(parser.Source(), opp.P, ass, ns, &method, params, 0/*ol->IsCons*/);
		
		if (ambig)
			parser.Error(opp.P, ER::Green + aName + ": ambigous symbol");
		
		TestAccess(*f, opp.P);
		
		if (onlyStatic && !f->IsStatic && f->IsConstructor == 0)
			parser.Error(opp.P, ER::Green + aName + ER::White + ": is not a static member");
		if (!onlyStatic && f->IsStatic) {
			if (Class && Class != &f->Owner())
				parser.Error(opp.P, ER::Green + aName + ER::White + ": is a static member");
		}
		// TODO: fix unsafe
		if (allowUnsafe == false && f->IsUnsafe)
			parser.Error(opp.P, ER::Green + aName + ER::White + ": is unsafe, can only be called in unsafe context");
	 
		if (f->InClass && f->ShouldEvaluate())
			comp.CompileFunc(*f);

		Node* node = nullptr;
		
		if (f->IsConstructor == 0) {
			ParamsNode* call = irg.callfunc(*f, object);
			call->Params = std::move(params);
			f->Owner().SetInUse();
			f->SetInUse();
			
			node = call;
		}
		else if (f->IsConstructor == 2) {
			TempNode* temp = irg.mem_temp(f->Class(), f);
			temp->Params = std::move(params);
		
			f->Owner().SetInUse();
			f->SetInUse();
			
			node = temp;
		}
		else {
			Node* temp = Temporary(f->Class(), params, opp);
			temp->Tt.Class->SetInUse();
			f->Owner().SetInUse();
			f->SetInUse();
			((TempNode*)temp)->Constructor = f;
			
			node = temp;
		}
		
		if (Function)
			Function->Dependencies.FindAdd(f);
		
		if (Function && Function->ShouldEvaluate())
			comp.CompileFunc(*Function);
		
		if (f->IsProperty) {
			ASSERT(f->Bundle);
			if(f->Bundle->PropSetter)
				node->IsEffLValue = true;
		}
		
		return node;
	}
	
	index = ns.Variables.Find(aName);
	if (index != -1) {
		ZVariable& f = *ns.Variables[index];
		
		TestAccess(f, opp.P);
		if (onlyStatic && !f.IsStatic)
			parser.Error(opp.P, ER::Green + aName + ER::White + ": is not a static member");
		if (!onlyStatic && f.IsStatic) {
			if (Class && Class != &f.Owner())
				parser.Error(opp.P, ER::Green + aName + ER::White + ": is a static member");
		}
	 
		if (!f.IsEvaluated) {
			ZCompilerContext zcon;
			zcon.Class = Class;
			zcon.Func = Function;
			zcon.TargetVar = &f;
			comp.CompileVar(f, zcon);
		}
		
		f.Owner().SetInUse();
		f.InUse = true;
		
		if (Function)
			Function->Dependencies.FindAdd(&f);
		
		return irg.mem_var(f, object, false);
	}
	
	return nullptr;
}

Node *ZExprParser::ParseDot(Node *exp) {
	auto p = parser.GetFullPos();
	String s;
	
	if (s == "f")
		s == "f";
	
	if (parser.Char('@')) {
		String sub = parser.ExpectId();
		if (sub == "size") {
			/*if (exp->Tt.Class == ass.CClass) {
				Node* size = irg.intrinsic(irg.const_class(ass.Classes[(int)exp->IntVal]), 1);
				size->SetType(ass.CPtrSize->Tt);
				return size;
			}
			else {*/
				Node* size = irg.intrinsic(exp, 1);
				size->SetType(ass.CPtrSize->Tt);
				return size;
			//}
		}
		s = "@" + sub;
	}
	else if (parser.Id("class")) {
		if (parser.Char('.')) {
			parser.ExpectId("HasTrait");
			parser.Expect('(');
			parser.ReadString('"');
			parser.Expect(')');
			return irg.const_bool(false);
		}
		else
			return irg.const_class(ass.Classes[(int)exp->IntVal]);
	}
	else if (parser.Char('~')) {
		parser.Expect('{');
		parser.Expect('}');
		
		return irg.destruct(exp);
	}
	else if (parser.Char('!')) {
		parser.Expect('{');
		if (parser.Char('}')) {
			Vector<Node*> params;
			return irg.attr(exp, Temporary(*exp->Tt.Class, params, p));
		}
		else {
			Node* cons = Parse();
			parser.Expect('}');
			
			return irg.attr(exp, cons);
		}
	}
	else
		s = parser.ExpectId();
	
	// case .class
	if (s == CLS_STR ) {
		if (exp->Tt.Class == ass.CClass) {
			if (!exp->IsLiteral)
				parser.Warning(p.P, "chained .${magenta}class${white} statements detected. "
					"Expression is equivalent to '${cyan}Class${white}.${magenta}class${white}' which is '${cyan}Class${white}'");
			if (exp->IntVal == 1)
				parser.Warning(p.P, "'${cyan}Class${white}.${magenta}class${white}' is '${cyan}Class${white}'. Is this intentional?");
			
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
			
			/*if (cs.IsEvaluated == false) {
				cs.IsEvaluated = true;
				comp.PreCompileVars(cs);
			}*/
			
			Node* node = ParseMember(cs, s, p, true);
			if (!node)
				parser.Error(p.P, "${magenta}class${white} '" + ER::Cyan + cs.Name + ER::White + "' does not have a member called: '" + ER::Green + s + ER::White + "'");
			
			return node;
		}
	}
	// instance members
	else {
		ZClass& cs = *exp->Tt.Class;
		Node* node = ParseMember(cs, s, p, false, exp);
		
		if (!node && cs.Super)
			node = ParseMember(*cs.Super, s, p, false, exp);
		
		if (!node) {
			if (cs.TBase == ass.CRaw) {
				if (s == "Length" || s == "Capacity")
					return irg.const_i(exp->Tt.Param, ass.CPtrSize);
			}
			parser.Error(p.P, "${magenta}class${white} '" + ER::Cyan + cs.Name + ER::White + "' does not have a member called: '" + ER::Green + s + ER::White + "'");
		}
		
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

ObjectInfo ZExprParser::ParseType(ZCompiler& comp, ZParser& parser, bool reqArrayQual, bool isParam, ZNamespace* aclass, ZNamespace* context, ZFunction* afunc) {
	Assembly& ass = comp.Ass();
	
	ObjectInfo ti;
	ti.IsRef = false;
	ti.IsConst = false;
	
	auto tt = parser.GetFullPos();
	String shtype = parser.ExpectId();
	String type = shtype;
	
	while (parser.Char('.'))
		type << "." << parser.ExpectId();
	
	ZClass* cls = nullptr;
	if (type.GetCount() == shtype.GetCount()) {
		if (context && context->IsClass) {
			ZClass& acls = (ZClass&)*context;
			if (acls.FromTemplate && acls.TBase && acls.TBase->Scan.TName[0] == type)
				cls = acls.T;
		}
		
		if (cls == nullptr) {
			// short name
			auto search = parser.Source().ShortNameLookup.FindPtr(shtype);
			if (search)
				cls = *search;
			
			if (cls == nullptr)
				ER::Error(parser.Source(), tt.P, "unknown identifier: " + type);
		}
	}
	else {
		// full namespace
		cls = ass.Classes.FindPtr(type);
		
		if (cls == nullptr)
			ER::Error(parser.Source(), tt.P, "unknown namespace reference: " + type);
	}
	
		
	//ti.Tt = cls->Tt;
	
	if (cls == ass.CPtr) {
		parser.Expect('<');
		
		if (parser.IsId("const"))
			parser.ReadId();
		ObjectInfo sub = ParseType(comp, parser, reqArrayQual, isParam, aclass, context);
		
		parser.Expect('>');

		if (ass.InvalidPtrClass(sub.Tt.Class))
			parser.Error(tt.P, "can't have a pointer to class " + ass.ToQtColor(&sub));
				
		ti.Tt = sub.Tt.Class->Pt;
		
		/*{
			ObjectType* tt = &ti.Tt;
			
			while (tt) {
				DUMP(tt->Class);
				DUMP(tt->Class->Name);
				tt = tt->Next;
			}
			DUMP("====");
		}
		
		if (ti.Tt.Class == ass.CPtr && ti.Tt.Next->Class == ass.CPtr) {
			auto t = ass.CPtr->Temps.Add();
			t = sub.Tt;
			ti.Tt = (ass.GetPtr(&t));
		}
		
		{
			ObjectType* tt = &ti.Tt;
			
			while (tt) {
				DUMP(tt->Class);
				DUMP(tt->Class->Name);
				tt = tt->Next;
			}
			DUMP("====");
		}*/
		
		
		return ti;
	}
	
	if (parser.Char('<')) {
		if (!cls->IsTemplate)
			parser.Error(tt.P, " class " + ass.ToQtColor(cls) + " is not a template");
		
		ObjectInfo sub = ParseType(comp, parser, reqArrayQual, isParam, aclass, context);
		
		Node* node = nullptr;
		
		if (cls == ass.CRaw && isParam) {
			if (parser.IsChar(','))
				parser.Error(tt.P, " method overloading not permited based on " + ass.ToQtColor(cls) + " size");
		}
		else if (cls == ass.CRaw && reqArrayQual && !parser.IsChar(','))
			ER::ErrCArrayWrongArgumentNo2(parser.Source(), tt.P, *cls);
			//parser.Expect(',');
		
		if (parser.Char(',')) {
			// TODO: fix
			ZNamespace ns(ass);
			ns.Sections.Add();
			ZVariable dummy(ns);
			dummy.Section = &ns.Sections[0];
			ZExprParser ep(dummy, aclass, afunc, comp, parser, comp.IRG());
			node = ep.Parse(true);
		}
		
		if (!parser.Char('>'))
			ER::ErrCArrayWrongArgumentNo2(parser.Source(), tt.P, *cls);
		//parser.Expect('>');
		
		cls = &comp.ResolveInstance(*cls, *sub.Tt.Class, tt, true);
		ti.Tt = cls->Tt;
		if (node) {
			ti.Tt.Param = node->IntVal;
		}
		auto temp = new ObjectType(sub.Tt);
		cls->Temps.Add(temp);
		ti.Tt.Next = temp;
		
		return ti;
	}
	
	ti.Tt = cls->Tt;
	
	return ti;
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

Node* ZExprParser::Temporary(ZClass& cls, Vector<Node*>& params, const ZSourcePos& pos) {
	if (&cls == ass.CPtr) {
		/*if (InVarMode)
			parser.Error(p, "inline member initialization can't take addresses");
		if (InConstMode)
			parser.Error(p, "addresses are not constants");

		if (params.GetCount() != 1 || params[0]->NT != NodeType::Memory)
			parser.Error(p, "local variable expected to take its address");
		*/
		

		if (params.GetCount() == 0)
			return irg.const_null();
		else if (params.GetCount() == 1) {
			if (ass.InvalidPtrClass(params[0]->Tt.Class)) {
				ER::Error(*pos.Source, pos.P, "can't have a pointer to class " + ass.ToQtColor(&params[0]->Tt));
				return nullptr;
			}
			return irg.mem_ptr(params[0], false);
		}
		else
			return nullptr;
	}
	
	if (cls.TBase == ass.CPtr) {
		Node* ptr = irg.mem_ptr(params[0], true);
		ptr->SetType(cls.T->Pt);
		return ptr;
	}
	
	Node* dr = nullptr;
	
	ZFunction* f = FindConstructor(cls, params, pos);
		
	if (f != nullptr) {
		//f = FindConstructor(cls, params, pos.P);
		TempNode* node = irg.mem_temp(cls, f);
		node->Params = std::move(params);
		
		if (f && f->ShouldEvaluate())
			comp.CompileFunc(*f);
		
		return node;
	}
	else if (&cls == ass.CFloat) {
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
		ZFunction* fc = nullptr;
		
		if (!cls.CoreSimple/* && cls.T && cls.T->CoreSimple == false*/) {
			fc = FindConstructor(cls, params, pos);
			if (f == nullptr) {
				if (cls.TBase == ass.CRaw)
					fc = FindConstructor(*cls.T, params, pos);
				
				if (fc == nullptr) {
					if (cls.T && cls.T->TBase == ass.CRaw)
						fc = FindConstructor(*cls.T->T, params, pos);
				}
			}
		}
		
		TempNode* node = irg.mem_temp(cls, fc);
		node->Params = std::move(params);
		
		if (fc && fc->ShouldEvaluate())
			comp.CompileFunc(*f);
		
		return node;
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

ZFunction* ZExprParser::FindConstructor(ZClass& cls, Vector<Node*>& params, const ZSourcePos& pos) {
	int index = cls.Methods.Find(THIS_STR);
			
	if (index != -1) {
		bool ambig = false;
		ZFunction* f = GetBase(&cls.Methods[index], nullptr, params, 1, false, ambig);
		
		if (!f) {
			//ER::CallError(parser.Source(), pos, ass, cls, &cls.Methods[index], params, 0, 1);
			return nullptr;
		}
		
		if (ambig) {
			//parser.Error(pos, ER::Green + cls.Name + ": ambigous symbol");
			return nullptr;
		}
		
		//if (cls.FromTemplate)
		//	comp.Push(pos, cls);
		
		TestAccess(*f, pos.P);
		
		//if (cls.FromTemplate)
		//	comp.Pop();
		
		f->SetInUse();
		
		if (f->InClass && f->ShouldEvaluate())
			comp.CompileFunc(*f);
		
		return f;
	}
	
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
		// TODO:fix ihneritence
		if (f.Access == AccessType::Protected && (&f.Owner() != Class && &f.Owner() != Class->Super))
			parser.Error(opp, "can't access protected member:\n\t\t" + f.ColorSig() + "        " + "[" + f.OwnerSig() + "]");
		if (f.Access == AccessType::Private && &f.Owner() != Class)
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




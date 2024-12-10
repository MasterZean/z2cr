#include <z2crlib/ZTranspiler.h>

extern String opss[];

bool CanAccess(AccessType access, int accessFlags) {
	if (accessFlags == -1)
		return true;
	
	if (access == AccessType::Public && ((accessFlags & 0b1) == 0))
		return false;
	if (access == AccessType::Protected && ((accessFlags & 0b10) == 0))
		return false;
	if (access == AccessType::Private && ((accessFlags & 0b100) == 0))
		return false;
	
	return true;
}

void ZTranspiler::WriteIntro() {
	NL();
	cs << "#include \"cppcode.h\"";
	EL();
	
	NL();
	EL();
}

void ZTranspiler::NsIntro(ZNamespace& ns) {
	if (CppVersion >= 2017)
		cs << ns.BackName<< " {";
	else
		cs << ns.BackNameLegacy;
}

void ZTranspiler::NsOutro(ZNamespace& ns) {
	if (CppVersion >= 2017)
		cs << "}";
	else
		cs << ns.LegacySufix;
}

void ZTranspiler::WriteClassForward() {
	int count = 0;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		for (int j = 0; j < ns.Classes.GetCount(); j++) {
			ZClass& cls = *ns.Classes[j];
					
			NL();
			cs << "namespace ";
			NsIntro(ns);
			cs << " ";
			cs << "class " << cls.Name << ";";
			cs << " ";
			NsOutro(ns);
			
			EL();
			
			count++;
		}
	}
	
	if (count)
		EL();
}

void ZTranspiler::WriteOutro() {
	if (comp.MainFunction == nullptr)
		return;
	
	NL();
	cs << "int main() {";
	EL();
	
	indent++;
	
	if (comp.MainFunction->InClass == false) {
		NL();
		cs << comp.MainFunction->Namespace().BackName << "::" << comp.MainFunction->BackName << "();";
		EL();
	}
	else {
		NL();
		cs << "auto temp = new " << comp.MainFunction->Namespace().BackName << "::" << comp.MainFunction->Owner().BackName << "()";
		ES();
		
		NL();
		cs << "temp->" << comp.MainFunction->BackName << "()";
		ES();
		
		NL();
		cs << "delete temp";
		ES();
	}
	
	NL();
	cs << "return 0;";
	EL();
	
	indent--;
	
	NL();
	cs << "}";
	EL();
}

void ZTranspiler::WriteCBinds(const Vector<ZFunction *>& CBinds){
}

void ZTranspiler::TranspileDeclarations(ZNamespace& ns, int accessFlags, bool classes) {
	BeginNamespace(ns);
	TranspileNamespaceDecl(ns, accessFlags, false);
	EndNamespace();
	
	if (!classes)
		return;
	
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		TranspileClassDeclMaster(cls, accessFlags);
	}
	
	if (TranspileMemberDeclFunc(ns, accessFlags, true, 0))
		EL();
	
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		
		if (TranspileMemberDeclFunc(cls, accessFlags, true, 0))
			EL();
	}
}

bool ZTranspiler::TranspileClassDeclMaster(ZNamespace& cls, int accessFlags) {
	if (!CanAccess(cls.Access, accessFlags))
		return false;
	
	if (cls.IsDefined)
		return false;
	
	cls.IsDefined = true;
	
	for (int j = 0; j < cls.DependsOn.GetCount(); j++)
		TranspileClassDeclMaster(*cls.DependsOn[j], accessFlags);
	
	BeginNamespace(cls.Namespace());
	BeginClass(cls);
	TranspileClassDecl(cls, -1);
	EndClass();
	EndNamespace();
	
	return true;
}

void ZTranspiler::TranspileNamespaceDecl(ZNamespace& ns, int accessFlags, bool doBinds) {
	int vc = TranspileMemberDeclVar(ns, accessFlags);
	int fc = TranspileMemberDeclFunc(ns, accessFlags, doBinds, vc);
}

void ZTranspiler::TranspileClassDecl(ZNamespace& ns, int accessFlags) {
	if (ns.Variables.GetCount() + ns.Methods.GetCount() == 0)
		return;
	
	int vc = TranspileMemberDeclVar(ns, 0b1);
	vc += TranspileMemberDeclVar(ns, 0b10);
	vc += TranspileMemberDeclVar(ns, 0b100);
	
	int fc = TranspileMemberDeclFunc(ns, 0b1, false, vc);
	fc += TranspileMemberDeclFunc(ns, 0b10, false, vc);
	fc += TranspileMemberDeclFunc(ns, 0b100, false, vc);
}

void ZTranspiler::WriteType(ObjectType* tt) {
	if (tt->Class == ass.CPtr) {
		WriteType(tt->Next);
		cs << "*";
	}
	else if (tt->Class->CoreSimple)
		cs << tt->Class->BackName;
	else {
		cs << tt->Class->Namespace().BackName << "::";
		cs << tt->Class->BackName;
	}
}

int ZTranspiler::TranspileMemberDeclVar(ZNamespace& ns, int accessFlags) {
	bool first = true;
	
	int count = 0;
	
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		
		if (!CanAccess(v.Access, accessFlags))
			continue;

		ASSERT(v.I.Tt.Class);
		ASSERT(v.Value);
		
		NewMember();
				
		count++;
		
		if (first) {
			if (v.InClass == true) {
				WriteClassAccess(v.Access);
			}
			first = false;
		}
		
		NL();
		
		if (v.InClass) {
			if (v.IsStatic && v.IsConst)
				cs << "static const ";
			else if (v.IsStatic)
				cs << "static ";
		}
		else {
			cs << "extern ";
			if (v.IsConst)
				cs << "const ";
		}
		
		if (v.Name == "p11")
			v.Name == "p11";

		WriteType(&v.I.Tt);
		cs << " " << v.Name;
		
		if (v.InClass && !v.IsStatic) {
			cs << " = ";
			Walk(v.Value);
		}
		
		ES();
	}
	
	return count;
}

int ZTranspiler::TranspileMemberDeclFunc(ZNamespace& ns, int accessFlags, bool doBinds, int vc) {
	bool first = true;
	
	int count = 0;
	
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		for (int j = 0; j < ns.Methods[i].Functions.GetCount(); j++) {
			ZFunction& f = *ns.Methods[i].Functions[j];
			
			if (!CanAccess(f.Access, accessFlags))
				continue;
			
			bool bindc = f.Trait.Flags & ZTrait::BINDC;
			
			if (doBinds == false && bindc == true)
				continue;
			if (doBinds == true && bindc == false)
				continue;
			
			NewMember();
			count++;
			
			if (first) {
				if (f.InClass == false) {
					if (vc)
						EL();
				}
				else if (doBinds == false) {
					WriteClassAccess(f.Access);
				}
				first = false;
			}
			
			NL();
			if (bindc)
				cs << "extern \"C\" ";
			else if (f.Trait.Flags & ZTrait::BINDCPP)
				cs << "extern ";
			WriteFunctionDef(f);
			ES();
		}
	}
	
	return count;
}

void ZTranspiler::TranspileDefinitions(ZNamespace& ns, bool vars, bool fDecl, bool wrap) {
	if (vars) {
		TranspileValDefintons(ns);
		
		for (int i = 0; i < ns.Classes.GetCount(); i++) {
			ZClass& cls = *ns.Classes[i];
			TranspileValDefintons(cls);
		}
	}
	
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			if (f.IsExternBind())
				continue;
			
			NL();
			
			if (fDecl)
				WriteFunctionDecl(f);
			WriteFunctionBody(f, wrap);
		}
	}
	
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		
		for (int j = 0; j < cls.Methods.GetCount(); j++) {
			ZMethodBundle& d = cls.Methods[j];
			
			for (int j = 0; j < d.Functions.GetCount(); j++) {
				ZFunction& f = *d.Functions[j];
				
				if (f.IsExternBind())
					continue;
				
				NL();
				
				if (fDecl)
					WriteFunctionDecl(f);
				WriteFunctionBody(f, wrap);
			}
		}
	}
}

void ZTranspiler::TranspileValDefintons(ZNamespace& ns, bool trail) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.I.Tt.Class);
		ASSERT(v.Value);
		
		if (v.InClass && !v.IsStatic)
			continue;
		
		if (v.IsConst)
			cs << "const ";
		WriteType(&v.I.Tt);
		cs << " ";
		if (!v.InClass)
			cs << v.Owner().BackName << "::" << v.Name << " = ";
		else
			cs << v.Namespace().BackName << "::" << v.Owner().BackName << "::" << v.Name << " = ";
		Walk(v.Value);
		ES();
	}
				
	if (trail && ns.Variables.GetCount())
		EL();
}

void ZTranspiler::WriteFunctionDef(ZFunction& f) {
	if (f.IsConstructor) {
		cs << f.Owner().BackName;
		WriteFunctionParams(f);
		return;
	}
	else if (f.Trait.Flags & ZTrait::BINDC)
		;
	else if (f.InClass == false && f.Access == AccessType::Private)
		cs << "static ";
	else if (f.InClass == true && f.IsStatic)
		cs << "static ";
	
	WriteType(&f.Return.Tt);
	
	cs << " " << f.BackName;
	WriteFunctionParams(f);
	
	if (f.InClass == true && f.IsFunction && !f.IsStatic)
		cs << " const";
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	if (f.IsConstructor) {
		cs << f.Namespace().BackName << "::";
		cs << f.Owner().Name << "::";
		cs << f.Owner().Name;
		WriteFunctionParams(f);
		return;
	}
	
	WriteType(&f.Return.Tt);
	
	cs << " " << f.Namespace().BackName << "::";
	if (f.InClass)
		cs << f.Owner().Name << "::";
	cs << f.BackName;
	WriteFunctionParams(f);
	
	if (f.InClass && f.IsFunction && !f.IsStatic)
		cs << " const";
}

void ZTranspiler::WriteFunctionParams(ZFunction& f) {
	cs << "(";
	
	for (int i = 0; i < f.Params.GetCount(); i++) {
		ZVariable& var = f.Params[i];
		
		if (i > 0)
			cs << ", ";
		
		ZClass& pclass = *var.I.Tt.Class->ParamType;
		
		if (var.I.Tt.Class == ass.CPtr)
			WriteType(&var.I.Tt);
		else if (pclass.CoreSimple)
			cs << pclass.BackName;
		else {
			if (f.Trait.Flags & ZTrait::BINDC)
				cs << pclass.Namespace().BackName << "::" << pclass.BackName;
			else
				cs << "const "<< pclass.Namespace().BackName << "::" << pclass.BackName << "&";
		}
		cs << " " << var.Name;
	}
	
	cs << ")";
}

void ZTranspiler::WriteFunctionBody(ZFunction& f, bool wrap) {
	if (wrap) {
		cs << " {";
		EL();
	}
	
	indent++;
	
	String params;
	
	/*if (PrintDebug) {
		NL();
		cs << "printf(\"enter: %s::%s(%s)\\n\", ";
		for (int i = 0; i < f.Params.GetCount(); i++) {
			if (i > 0)
				params << ", ";
			params << f.Ass().ClassToString(&f.Params[i].I);
		}
		cs << "\"" << f.Namespace().BackName << "\"" << ", " << "\"" << f.BackName << "\""  << ", " << "\"" << params << "\"" ;
		cs << ")";
		ES();
	}*/
	
	WalkChildren(&f.Nodes);
		
	/*if (PrintDebug) {
		NL();
		cs << "printf(\"exit: %s::%s(%s)\\n\", ";
		cs << "\"" << f.Namespace().BackName << "\"" << ", " << "\"" << f.BackName << "\""  << ", " << "\"" << params << "\"" ;
		cs << ")";
		ES();
	}*/
	
	indent--;
	
	if (wrap) {
		NL();
		cs << "}";
		EL();
		
		NL();
		EL();
	}
}

void ZTranspiler::WriteClassAccess(AccessType access) {
	if (access == AccessType::Public) {
		indent--;
		NL();
		cs << "public:";
		EL();
		indent++;
	}
	if (access == AccessType::Protected)  {
		indent--;
		NL();
		cs << "protected:";
		EL();
		indent++;
	}
	if (access == AccessType::Private) {
		indent--;
		NL();
		cs << "private:";
		EL();
		indent++;
	}
}

void ZTranspiler::WalkNode(Node* node) {
	Walk(node);
}

void ZTranspiler::Walk(Node* node) {
	ASSERT_(node, "Null node");
	
	if (node->NT == NodeType::Const)
		Proc(*(ConstNode*)node, cs);
	else if (node->NT == NodeType::BinaryOp)
		Proc(*(OpNode*)node);
	else if (node->NT == NodeType::UnaryOp)
		Proc(*(UnaryOpNode*)node);
	else if (node->NT == NodeType::Memory)
		Proc(*(MemNode*)node);
	else if (node->NT == NodeType::Cast)
		Proc(*(CastNode*)node);
	else if (node->NT == NodeType::Temporary)
		Proc(*(TempNode*)node);
	else if (node->NT == NodeType::Def)
		Proc(*(DefNode*)node);
	else if (node->NT == NodeType::List)
		Proc(*(ListNode*)node);
	/*else if (node->NT == NodeType::Construct)
		Proc((ConstructNode*)node);*/
	else if (node->NT == NodeType::Ptr)
		Proc(*(PtrNode*)node);
	/*else if (node->NT == NodeType::Index)
		Proc((IndexNode*)node);
	else if (node->NT == NodeType::SizeOf)
		Proc((SizeOfNode*)node);
	else if (node->NT == NodeType::Property)
		Proc((PropertyNode*)node);
	else if (node->NT == NodeType::Deref)
		Proc((DerefNode*)node);*/
	else if (node->NT == NodeType::Intrinsic)
		Proc(*(IntrinsicNode*)node);
	else if (node->NT == NodeType::Return)
		Proc(*(ReturnNode*)node);
	else if (node->NT == NodeType::Local)
		Proc(*(LocalNode*)node);
	/*else if (node->NT == NodeType::Alloc)
		Proc((AllocNode*)node);
	else if (node->NT == NodeType::Array)
		Proc((RawArrayNode*)node);
	else if (node->NT == NodeType::Using)
		Proc((UsingNode*)node);*/
	else if (node->NT == NodeType::Block)
		Proc(*(BlockNode*)node);
	else if (node->NT == NodeType::If)
		Proc(*(IfNode*)node);
	else if (node->NT == NodeType::While)
		Proc(*(WhileNode*)node);
	else if (node->NT == NodeType::DoWhile)
		Proc(*(DoWhileNode*)node);
	else if (node->NT == NodeType::ForLoop)
		Proc(*(ForLoopNode*)node);
	else if (node->NT == NodeType::LoopControl)
		Proc(*(LoopControlNode*)node);
	else
		ASSERT_(0, "Invalid node");
}

void ZTranspiler::WalkChildren(Node* node) {
	Node* child = node->First;
	
	while (child) {
		NL();
		WalkNode(child);
		if (child->NT != NodeType::Block && child->NT != NodeType::If && child->NT != NodeType::While && child->NT != NodeType::ForLoop)
			ES();
		
		child = child->Next;
	}
}

void ZTranspiler::Proc(ConstNode& node, Stream& stream) {
	if (node.Tt.Class == ass.CQWord) {
		if (IsNull(node.IntVal))
			stream << "9223372036854775808ull";
		else
			stream << Format64((uint64)node.IntVal) << "ull";
	}
	else if (node.Tt.Class == ass.CLong) {
		if (IsNull(node.IntVal))
			stream << "-9223372036854775808ll";
		else
			stream << IntStr64(node.IntVal) << "ll";
	}
	else if (node.Tt.Class == ass.CDWord || node.Tt.Class == ass.CWord || node.Tt.Class == ass.CByte) {
		if (node.Base == 16)
			stream << "0x" << Format64Hex((uint32)node.IntVal) << 'u';
		else
			stream << Format64((uint32)node.IntVal) << 'u';
	}
	else if (node.Tt.Class == ass.CPtrSize) {
		if (node.Base == 16)
			stream << "0x" << Format64Hex(node.IntVal) << "u";
		else
			stream << Format64(node.IntVal) << "u";
	}
	else if (node.Tt.Class == ass.CInt || node.Tt.Class == ass.CSmall || node.Tt.Class == ass.CShort) {
		if (node.Base == 16) {
			if (node.IntVal == -2147483648ll)
				stream << "(int32)" << "0x" << ToUpper(Format64Hex(node.IntVal)) << "ll";
			else
				stream << "0x" << ToUpper(Format64Hex(node.IntVal));
		} else {
			if (node.IntVal == -2147483648ll)
				stream << "(int32)" << IntStr64(node.IntVal) << "ll";
			else
				stream << IntStr64(node.IntVal);
		}
	}
	else if (node.Tt.Class == ass.CFloat) {
		if (node.DblVal != node.DblVal)
			stream << "((float)(1e+300 * 1e+300) * 0.0f)";
		else if (node.DblVal == (float)(1e+300 * 1e+300))
			stream << "((float)(1e+300 * 1e+300))";
		else if (node.DblVal == -(float)(1e+300 * 1e+300))
			stream << "(-(float)(1e+300 * 1e+300))";
		else {
			if (IsNull(node.DblVal))
				stream << "-1.79769e+308" << "\f";
			else {
				String s = FormatDouble(node.DblVal);
				stream << s;
				if (s.Find('.') == -1 && s.Find('e') == -1)
				    stream << ".0";
				stream << 'f';
			}
		}
	}
	else if (node.Tt.Class == ass.CDouble) {
		if (node.DblVal != node.DblVal)
			stream << "((float)(1e+300 * 1e+300) * 0.0f)";
		else if (node.DblVal == (float)(1e+300 * 1e+300))
			stream << "((float)(1e+300 * 1e+300))";
		else if (node.DblVal == -(float)(1e+300 * 1e+300))
			stream << "(-(float)(1e+300 * 1e+300))";
		else {
			if (IsNull(node.DblVal))
				stream << "-1.79769e+308";
			else {
				String s = FormatDouble(node.DblVal);
				stream << s;
				if (s.Find('.') == -1 && s.Find('e') == -1)
					stream << ".0";
			}
		}
	}
	else if (node.Tt.Class == ass.CBool) {
		if (node.IntVal == 0)
			stream << "false";
		else
			stream << "true";
	}
	else if (node.Tt.Class == ass.CChar) {
		if (node.IntVal >= 127) {
			if (node.Base == 16)
				stream << "0x" << Format64Hex(node.IntVal);
			else
				stream << node.IntVal;
			return;
		}
		
		stream << '\'';
		
		if (node.IntVal == '\t')
			stream << "\\t";
		else if (node.IntVal == '\n')
			stream << "\\n";
		else if (node.IntVal == '\r')
			stream << "\\r";
		else if (node.IntVal == '\a')
			stream << "\\a";
		else if (node.IntVal == '\b')
			stream << "\\b";
		else if (node.IntVal == '\f')
			stream << "\\f";
		else if (node.IntVal == '\v')
			stream << "\\v";
		else if (node.IntVal == '\'')
			stream << "\\\'";
		else if (node.IntVal == '\\')
			stream << "\\\\";
		else if (node.IntVal == '\0')
			stream << "\\0";
		else
			stream << (char)node.IntVal;
		
		stream << '\'';
	}
	else if (node.Tt.Class == ass.CClass) {
		ZClass& ccc = ass.Classes[(int)node.IntVal];
		//stream << ass.CCls->NamespaceQual;
		stream << "Class(" << ccc.RTTIIndex << ")";
	}
	else if (node.Tt.Class == ass.CNull)
		stream << "nullptr";
	else if (node.Tt.Class == ass.CString) {
		//stream << "S_[" << (int)node.IntVal << ']';
		stream << AsCString(ass.StringConsts[(int)node.IntVal]);
	}
}

void ZTranspiler::Proc(OpNode& node) {
	Node *l = node.OpA;
	Node *r = node.OpB;
	ASSERT(l);
	ASSERT(r);
	
	if (node.Op == OpNode::opTernary) {
		cs << "((";
		Walk(l);
		cs << ") ? ";
		Walk(r);
		cs << " : ";
		ASSERT(node.OpC);
		Walk(node.OpC);
		cs << ")";
	}
	else if (node.Op == OpNode::opMod && (ass.IsFloat(l->Tt) || ass.IsFloat(l->Tt))) {
		if (l->Tt.Class != ass.CDouble && r->Tt.Class != ass.CDouble)
			cs << "(float)";
		cs << "fmod(";
		Walk(l);
		cs << ", ";
		Walk(r);
		cs << ')';
	}
	else {
		//if (!node.Assign)
		//	cs << "(";
		Walk(l);
		cs << ' ' << opss[node.Op];
		//if (node.Assign)
		//	cs << '=';
		cs << ' ';
		Walk(r);
		//if (!node.Assign)
		//	cs << ")";
	}
}

void ZTranspiler::Proc(UnaryOpNode& node) {
	Node *f = node.OpA;
	ASSERT(f);
	
	if (node.Op == OpNode::opInc) {
		if (node.Prefix) {
			cs << "++";
			Walk(f);
		}
		else {
			Walk(f);
			cs << "++";
		}
	}
	else if (node.Op == OpNode::opDec) {
		if (node.Prefix) {
			cs << "--";
			Walk(f);
		}
		else {
			Walk(f);
			cs << "--";
		}
	}
	else {
		if (node.Op == OpNode::opNot && ass.IsFloat(node.Tt)) {
			cs << "::bitnot(";
			Walk(f);
			cs << ')';
		}
		else {
			cs << opss[node.Op];
			if (f->NT != NodeType::List) {
				cs << '(';
				Walk(f);
				cs << ')';
			}
			else
				Walk(f);
		}
	}
}

void ZTranspiler::Proc(DefNode& node) {
	ZFunction& f = *node.Function;
	
	if (f.Trait.Flags & ZTrait::BINDC)
		cs << "::";
	else if (f.IsStatic) {
		cs << f.Namespace().BackName << "::";
		if (f.InClass)
			cs << f.Owner().BackName << "::";
	}
	else {
		//ASSERT(node.Object);
		if (node.Object) {
			Walk(node.Object);
			cs << ".";
		}
	}
	
	cs << f.BackName;
	cs << '(';
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node* p = node.Params[i];

		Walk(p);
		
		if (i < count -1)
			cs << ", ";
	}
	cs << ')';
}

void ZTranspiler::Proc(MemNode& node) {
	ASSERT(node.Mem);
	
	if (node.IsLocal == false && node.IsParam == false && node.Mem->InClass == false) {
		if (node.Mem->InClass)
			cs << node.Mem->Namespace().BackName << "::" << node.Mem->Owner().BackName << "::";
		else
			cs << node.Mem->Owner().BackName << "::";
		cs << node.Mem->BackName;
	}
	else if (node.Mem->InClass == true && node.Mem->IsStatic) {
		cs << node.Mem->Namespace().BackName << "::" << node.Mem->Owner().BackName << "::";
		cs << node.Mem->BackName;
	}
	else if (node.Object) {
		Walk(node.Object);
		cs << ".";
		cs << node.Mem->BackName;
	}
	else
		cs << node.Mem->BackName;
	
}

void ZTranspiler::Proc(BlockNode& node) {
	//NL();
	cs << "{";
	EL();
	
	indent++;
	WalkChildren(&node.Nodes);
	indent--;
	
	NL();
	cs << '}';
	
	if (node.EndBlockStat)
		EL();
}

void ZTranspiler::Proc(IfNode& node) {
	ASSERT(node.Cond);
	
	//NL();
	cs << "if (";
	Walk(node.Cond);
	cs << ")";
	// dangling end statement
	
	if (node.TrueBranch) {
		if (node.TrueBranch->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.TrueBranch);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.TrueBranch);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
	
	if (node.FalseBranch) {
		NL();
		cs << "else";
		// dangling end statement

		if (node.FalseBranch->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.FalseBranch);
			if (node.FalseBranch->NT == NodeType::If)
				EL();
			else
				ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.FalseBranch);
		}
	}
}

void ZTranspiler::Proc(WhileNode& node) {
	ASSERT(node.Cond);
	
	cs << "while (";
	Walk(node.Cond);
	cs << ")";
	// dangling end statement
	
	if (node.Body) {
		if (node.Body->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.Body);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.Body);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
}

void ZTranspiler::Proc(DoWhileNode& node) {
	ASSERT(node.Cond);

	cs << "do ";
	if (node.Body->NT == NodeType::Block)
		((BlockNode*)node.Body)->EndBlockStat = false;
	WalkNode(node.Body);
	
	cs << " while (";
	Walk(node.Cond);
	cs << ")";
}

void ZTranspiler::Proc(ForLoopNode& node) {
	ASSERT(node.Cond);
	
	cs << "for (";
	if (node.Init)
		Walk(node.Init);
	cs << "; ";
	Walk(node.Cond);
	cs << "; ";
	if (node.Iter)
		Walk(node.Iter);
	cs << ")";
	
	if (node.Body) {
		if (node.Body->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.Body);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.Body);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
}

void ZTranspiler::Proc(LocalNode& node) {
	ASSERT(node.Var);
	
	if (node.Var->IsConst)
		cs << "const ";
	if (node.Tt.Class->CoreSimple)
		cs << node.Tt.Class->BackName;
	else {
		cs << node.Tt.Class->Namespace().BackName << "::";
		cs << node.Tt.Class->BackName;
	}
	cs << " " << node.Var->Name;
	
	if (node.Var->Value) {
		cs << " = ";
		Walk(node.Var->Value);
	}
}

void ZTranspiler::Proc(CastNode& node) {
	ASSERT(node.Object);
	
	cs << '(';
	
	/*if (ass.IsPtr(node->Tt)) {
		WriteClassName(*node->Tt.Next->Class);
		cs << "*";
	}
	else {*/
		/*if (!node->Tt.Class->CoreSimple)
			cs << node->Tt.Class->NamespaceQual;
		if (node->Tt.Class->CoreSimple && node->Tt.Class->Scan.IsEnum)
			cs << node->Tt.Class->NamespaceQual;*/
		
		cs << node.Tt.Class->BackName;
		/*if (node->Tt.Class->Scan.IsEnum)
			*cs << "::Type";

		if (node->Ptr)
			*cs << "*";*/
	//}
	
	cs << ')';
	
	cs << '(';
	Walk(node.Object);
	cs << ')';
}

void ZTranspiler::Proc(ReturnNode& node) {
	cs << "return";
	
	if (node.Value) {
		cs << " ";
		Walk(node.Value);
	}
}

void ZTranspiler::Proc(IntrinsicNode& node) {
	cs << "(";
	for (int i = 0; i < node.Value.GetCount(); i++) {
		if (i > 0)
			cs << ", ";
		
		cs << "::printf";
		if (node.Value[i]->Tt.Class == ass.CChar)
			cs << "c";
		cs << "(";
		
		Walk(node.Value[i]);
		
		cs << ")";
	}
	cs << ")";
}

void ZTranspiler::Proc(LoopControlNode& node) {
	if (node.Break)
		cs << "break";
	else
		cs << "continue";
}

void ZTranspiler::Proc(TempNode& node) {
	ZClass& cls = *node.Tt.Class;
	
	if (cls.CoreSimple) {
		cs << cls.Namespace().BackName << "::";
		cs << cls.Name << "::" << node.Constructor->BackName << '(';
	}
	else {
		cs << cls.Namespace().BackName << "::";
		cs << cls.Name;
		cs << '(';
	}
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node* p = node.Params[i];
		
		Walk(p);
		if (i < count - 1)
			cs << ", ";
	}
	
	cs << ')';
}

void ZTranspiler::Proc(ListNode& node) {
	cs << '(';
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node *p = node.Params[i];
	
		Walk(p);
		if (i < count - 1)
			cs << ", ";
	}
	
	cs << ')';
}

void ZTranspiler::Proc(PtrNode& node) {
	ASSERT(node.Object);
	if (node.Nop) {
		// TODO: fix
		cs << "(uint8*)(";
		Walk(node.Object);
		cs << ")";
	}
	else {
		cs << "&(";
		Walk(node.Object);
		cs << ')';
	}
}


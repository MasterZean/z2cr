#include <z2crlib/ZTranspiler.h>

extern String opss[];

void ZTranspiler::WriteIntro() {
	NL();
	cs << "#include \"cppcode.h\"";
	EL();
	NL();
	cs << "#include \"stdio.h\"";
	EL();
	
	NL();
	EL();
	
	//new llvm::LLVMContext();
}

void ZTranspiler::WriteOutro() {
	if (comp.MainFunction == nullptr)
		return;
	
	NL();
	cs << "int main() {";
	EL();
	
	indent++;
	
	NL();
	cs << comp.MainFunction->Namespace().BackName << "::" << comp.MainFunction->BackName << "();";
	EL();
	
	NL();
	cs << "return 0;";
	EL();
	
	indent--;
	
	NL();
	cs << "}";
	EL();
}

void ZTranspiler::TranspileDeclarations(ZNamespace& ns) {
	if (ns.Variables.GetCount() + ns.Methods.GetCount() == 0)
		return;
	
	NL();
	cs << "namespace " << ns.BackName << " {";
	EL();
	
	indent++;
	
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.I.Tt.Class);
		ASSERT(v.Value);
		
		NL();
		cs << "extern "<< v.I.Tt.Class->BackName << " " << v.Name;
		ES();
	}
	
	if (ns.Variables.GetCount())
		EL();
		
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		for (int j = 0; j < ns.Methods[i].Functions.GetCount(); j++) {
			NL();
			WriteFunctionDef(*ns.Methods[i].Functions[j]);
			ES();
		}
	}
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	EL();
}

void ZTranspiler::TranspileDefinitions(ZNamespace& ns, bool vars, bool fDecl, bool wrap) {
	if (vars) {
		TranspileValDefintons(ns);
	}
	
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			NL();
			
			if (fDecl)
				WriteFunctionDecl(f);
			WriteFunctionBody(f, wrap);
		}
	}
}

void ZTranspiler::TranspileValDefintons(ZNamespace& ns, bool trail) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.I.Tt.Class);
		ASSERT(v.Value);
		
		cs << v.I.Tt.Class->BackName << " " << v.Namespace().BackName << "::" << v.Name << " = ";
		Walk(v.Value);
		ES();
	}
				
	if (trail && ns.Variables.GetCount())
		EL();
}

void ZTranspiler::WriteFunctionDef(ZFunction& f) {
	cs << f.Return.Tt.Class->BackName << " " << f.BackName;
	WriteFunctionParams(f);
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	cs << f.Return.Tt.Class->BackName << " " << f.Namespace().BackName << "::" << f.BackName;
	WriteFunctionParams(f);
}

void ZTranspiler::WriteFunctionParams(ZFunction& f) {
	cs << "(";
	
	for (int i = 0; i < f.Params.GetCount(); i++) {
		ZVariable& var = f.Params[i];
		
		if (i > 0)
			cs << ", ";
		
		cs << var.I.Tt.Class->ParamType->BackName << " " << var.Name;
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
	
	if (PrintDebug) {
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
	}
	
	WalkChildren(&f.Nodes);
		
	if (PrintDebug) {
		NL();
		cs << "printf(\"exit: %s::%s(%s)\\n\", ";
		cs << "\"" << f.Namespace().BackName << "\"" << ", " << "\"" << f.BackName << "\""  << ", " << "\"" << params << "\"" ;
		cs << ")";
		ES();
	}
	
	indent--;
	
	if (wrap) {
		NL();
		cs << "}";
		EL();
		
		NL();
		EL();
	}
}

void ZTranspiler::WalkNode(Node* node) {
	if (node->NT == NodeType::Block || node->NT == NodeType::Local) {
		Walk(node);
		return;
	}
	
	if (!PrintDebug) {
		Walk(node);
		return;
	}
	
	if (node->Tt.Class == ass.CQWord) {
		cs << "printf(\"%llu\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CLong) {
		cs << "printf(\"%lld\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CDWord || node->Tt.Class == ass.CWord || node->Tt.Class == ass.CByte) {
		cs << "printf(\"%u\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CPtrSize) {
		cs << "printf(\"%u\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CInt || node->Tt.Class == ass.CSmall || node->Tt.Class == ass.CShort) {
		cs << "printf(\"%d\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CFloat) {
		cs << "printf(\"%g\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CDouble) {
		cs << "printf(\"%g\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CBool) {
		cs << "printf(\"%s\\n\", ";
		Walk(node);
		cs << " ? \"true\" : \"false\")";
	}
	else if (node->Tt.Class == ass.CChar) {
		Walk(node);
	}
	else
		Walk(node);
}

void ZTranspiler::Walk(Node* node) {
	ASSERT_(node, "Null node");
	if (node->NT == NodeType::Const)
		Proc(*(ConstNode*)node, cs);
	else if (node->NT == NodeType::BinaryOp)
		Proc(*(OpNode*)node);
	/*else if (node->NT == NodeType::UnaryOp)
		Walk((UnaryOpNode*)node);*/
	else if (node->NT == NodeType::Memory)
		Proc(*(MemNode*)node);
	else if (node->NT == NodeType::Cast)
		Proc(*(CastNode*)node);
	/*else if (node->NT == NodeType::Temporary)
		Proc((TempNode*)node);*/
	else if (node->NT == NodeType::Def)
		Proc(*(DefNode*)node);
	/*else if (node->NT == NodeType::List)
		Proc((ListNode*)node);
	else if (node->NT == NodeType::Construct)
		Proc((ConstructNode*)node);
	else if (node->NT == NodeType::Ptr)
		Proc((PtrNode*)node);
	else if (node->NT == NodeType::Index)
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
	else
		ASSERT_(0, "Invalid node");
}

void ZTranspiler::WalkChildren(Node* node) {
	Node* child = node->First;
	
	while (child) {
		//if (child->NT != NodeType::Block/* && child->NT != NodeType::If*/)
			NL();
		WalkNode(child);
		if (child->NT != NodeType::Block && child->NT != NodeType::If && child->NT != NodeType::While)
			ES();
		
		child = child->Next;
	}
}

void ZTranspiler::Proc(ConstNode& node, Stream& stream)
{
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
	else if (node.Tt.Class == ass.CCls) {
		ZClass& ccc = ass.Classes[(int)node.IntVal];
		//stream << ass.CCls->NamespaceQual;
		stream << "Class(" << ccc.RTTIIndex << ")";
	}
}

void ZTranspiler::Proc(OpNode& node) {
	Node *l = node.OpA;
	Node *r = node.OpB;
	ASSERT(l);
	ASSERT(r);
	
	if (!node.Assign)
		cs << "(";
	Walk(l);
	cs << ' ' << opss[node.Op];
	//if (node.Assign)
	//	cs << '=';
	cs << ' ';
	Walk(r);
	if (!node.Assign)
		cs << ")";
}

void ZTranspiler::Proc(DefNode& node) {
	cs << node.Overload->Namespace().BackName << "::" << node.Overload->BackName;
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
	
	if (node.IsLocal == false && node.IsParam == false)
		cs << node.Mem->Namespace().BackName << "::";
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
	WalkNode(node.Body);
	
	NL();
	cs << "while(";
	Walk(node.Cond);
	cs << ")";
}

void ZTranspiler::Proc(LocalNode& node) {
	ASSERT(node.Var);
	
	cs << node.Tt.Class->BackName << " ";
	cs << node.Var->Name;
	
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
	for (int i = 0; i < node.Value.GetCount(); i++) {
		if (i > 0)
			cs <<"; ";
		
		cs << "::printf";
		if (node.Value[i]->Tt.Class == ass.CChar)
			cs << "c";
		cs << "(";
		
		Walk(node.Value[i]);
		
		cs << ")";
	}
}

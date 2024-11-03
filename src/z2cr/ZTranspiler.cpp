#include "z2cr.h"

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
	cs << comp.MainFunction->GetNamespace().BackName << "::" << comp.MainFunction->BackName << "();";
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
	NL();
	cs << "namespace " << ns.BackName << " {";
	EL();
	
	indent++;
	
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.Tt.Class);
		ASSERT(v.Value);
		
		NL();
		cs << "extern "<< v.Tt.Class->BackName << " " << v.Name;
		ES();
	}
	
	if (ns.Variables.GetCount())
		EL();
		
	for (int i = 0; i < ns.Definitions.GetCount(); i++) {
		for (int j = 0; j < ns.Definitions[i].Functions.GetCount(); j++) {
			NL();
			WriteFunctionDef(*ns.Definitions[i].Functions[j]);
			ES();
		}
	}
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	EL();
}

void ZTranspiler::TranspileDefinitions(ZNamespace& ns) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.Tt.Class);
		ASSERT(v.Value);
		
		NL();
		cs << v.Tt.Class->BackName << " " << v.GetNamespace().BackName << "::" << v.Name << " = ";
		Walk(v.Value);
		ES();
	}
	
	if (ns.Variables.GetCount())
		EL();
	
	for (int i = 0; i < ns.Definitions.GetCount(); i++) {
		ZDefinition& d = ns.Definitions[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			WriteFunctionDecl(f);
			WriteFunctionBody(f);
		}
	}
}

void ZTranspiler::WriteFunctionDef(ZFunction& f) {
	cs << "void " << f.BackName << "()";
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	cs << "void " << f.GetNamespace().BackName << "::" << f.BackName << "()";
}

void ZTranspiler::WriteFunctionBody(ZFunction& f) {
	NL();
	cs << " {";
	EL();
	
	indent++;
	
	NL();
	cs << "printf(\"enter: %s::%s\\n\", ";
	cs << "\"" << f.GetNamespace().BackName << "\"" << ", " << "\"" << f.BackName << "\"";
	cs << ")";
	ES();
	
	Node* node = f.Nodes.First;
	
	while (node) {
		NL();
		WalkNode(node);
		ES();
		
		node = node->Next;
	}
	
	NL();
	cs << "printf(\"exit: %s::%s\\n\", ";
	cs << "\"" << f.GetNamespace().BackName << "\"" << ", " << "\"" << f.BackName << "\"";
	cs << ")";
	ES();
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	
	NL();
	EL();
}

void ZTranspiler::WalkNode(Node* node) {
	if (node->Tt.Class == ass.CQWord) {
		cs << "printf(\"%ull\\n\", ";
		Walk(node);
		cs << ")";
	}
	else if (node->Tt.Class == ass.CLong) {
		cs << "printf(\"%ll\\n\", ";
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
	/*else if (node->NT == NodeType::Cast)
		Proc((CastNode*)node);
	else if (node->NT == NodeType::Temporary)
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
		Proc((DerefNode*)node);
	else if (node->NT == NodeType::Intrinsic)
		Proc((IntNode*)node);
	else if (node->NT == NodeType::Return)
		Proc((ReturnNode*)node);
	else if (node->NT == NodeType::Var)
		Proc((VarNode*)node);
	else if (node->NT == NodeType::Alloc)
		Proc((AllocNode*)node);
	else if (node->NT == NodeType::Array)
		Proc((RawArrayNode*)node);
	else if (node->NT == NodeType::Using)
		Proc((UsingNode*)node);*/
	else
		ASSERT_(0, "Invalid node");
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
}

void ZTranspiler::Proc(OpNode& node) {
	Node *l = node.OpA;
	Node *r = node.OpB;
	ASSERT(l);
	ASSERT(r);
	
	cs << "(";
	Walk(l);
	cs << ' ' << opss[node.Op];
	if (node.Assign)
		cs << '=';
	cs << ' ';
	Walk(r);
	cs << ")";
}

void ZTranspiler::Proc(DefNode& node) {
	cs << node.Overload->GetNamespace().BackName << "::" << node.Overload->BackName;
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
	
	cs << node.Mem->BackName;
}
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

void ZTranspiler::WriteFunctionDef(ZFunction& f) {
	cs << "void " << f.BackName << "()";
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	cs << "void " << f.GetNamespace().BackName << "::" << f.BackName << "()";
}

void ZTranspiler::WriteFunctionBody(Vector<Node*>& nodes) {
	NL();
	cs << " {";
	EL();
	
	indent++;
	
	for (int i = 0; i < nodes.GetCount(); i++) {
		NL();
		Walk(nodes[i]);
		ES();
	}
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	
	NL();
	EL();
}

void ZTranspiler::Walk(Node* node) {
	ASSERT_(node, "Null node");
	if (node->NT == NodeType::Const)
		Walk(*(ConstNode*)node, cs);
	else if (node->NT == NodeType::BinaryOp)
		Walk(*(OpNode*)node);
	/*else if (node->NT == NodeType::UnaryOp)
		Walk((UnaryOpNode*)node);
	else if (node->NT == NodeType::Memory)
		Walk((MemNode*)node);
	else if (node->NT == NodeType::Cast)
		Walk((CastNode*)node);
	else if (node->NT == NodeType::Temporary)
		Walk((TempNode*)node);
	else if (node->NT == NodeType::Def)
		Walk((DefNode*)node);
	else if (node->NT == NodeType::List)
		Walk((ListNode*)node);
	else if (node->NT == NodeType::Construct)
		Walk((ConstructNode*)node);
	else if (node->NT == NodeType::Ptr)
		Walk((PtrNode*)node);
	else if (node->NT == NodeType::Index)
		Walk((IndexNode*)node);
	else if (node->NT == NodeType::SizeOf)
		Walk((SizeOfNode*)node);
	else if (node->NT == NodeType::Property)
		Walk((PropertyNode*)node);
	else if (node->NT == NodeType::Deref)
		Walk((DerefNode*)node);
	else if (node->NT == NodeType::Intrinsic)
		Walk((IntNode*)node);
	else if (node->NT == NodeType::Return)
		Walk((ReturnNode*)node);
	else if (node->NT == NodeType::Var)
		Walk((VarNode*)node);
	else if (node->NT == NodeType::Alloc)
		Walk((AllocNode*)node);
	else if (node->NT == NodeType::Array)
		Walk((RawArrayNode*)node);
	else if (node->NT == NodeType::Using)
		Walk((UsingNode*)node);*/
	else
		ASSERT_(0, "Invalid node");
}

void ZTranspiler::Walk(ConstNode& node, Stream& stream)
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
		stream << "printf(\"%d\\n\", ";
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
		stream << ")";
	}
	else if (node.Tt.Class == ass.CFloat) {
		stream << "printf(\"%g\\n\", ";
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
		stream << ")";
	}
	else if (node.Tt.Class == ass.CDouble) {
		stream << "printf(\"%g\\n\", ";
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
		stream << ")";
	}
	else if (node.Tt.Class == ass.CBool) {
		stream << "printf(\"%s\\n\", ";
		if (node.IntVal == 0)
			stream << "false";
		else
			stream << "true";
		stream << ")";
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

void ZTranspiler::Walk(OpNode& node) {
	Node *l = node.OpA;
	Node *r = node.OpB;
	ASSERT(l);
	ASSERT(r);
	
	Walk(l);
	cs << ' ' << opss[node.Op];
	if (node.Assign)
		cs << '=';
	cs << ' ';
	Walk(r);
}


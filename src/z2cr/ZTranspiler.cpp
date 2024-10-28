#include "z2cr.h"

void ZTranspiler::WriteIntro() {
	NL();
	cs << "#include \"z2stdcompat.h\"";
	EL();
	
	NL();
	EL();
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	NL();
	cs << "void " << f.BackName << "()";
	//EL();
}

void ZTranspiler::WriteFunctionBody(Vector<Node*>& nodes) {
	NL();
	cs << " {";
	EL();
	
	indent++;
	
	for (int i = 0; i < nodes.GetCount(); i++) {
		NL();
		Walk((ConstNode&)*nodes[i], cs);
		ES();
	}
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	
	NL();
	EL();
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
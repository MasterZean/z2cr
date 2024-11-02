#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

class ZExprParser {
public:
	ZExprParser(ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.GetAssembly()) {
	}
	
	static void Initialize();
	
	Node* Parse();
	
	Node* ParseBin(int prec, Node* left, CParser::Pos& backupPoint);
	Node* ParseAtom();
	Node* ParseNumeric();
	Node* ParseNamespace();
	
private:
	Assembly& ass;
	ZParser& parser;
	IR& irg;
	static bool initialized;
	
	int GetPriority(int& op, bool& opc);
	
	void IncompatOp(const ZSource& src, Point& p, const String& op, Node* left, Node* right) {
		ErrorReporter::ErrIncompatOp(src, p, op, ass.ClassToString(left), ass.ClassToString(right));
	}
	
	static Point OPS[256];
	static bool OPCONT[256];
};

#endif

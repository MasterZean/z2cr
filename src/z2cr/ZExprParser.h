#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

class ZExprParser {
public:
	ZNamespaceSection* Section = nullptr;
	
	ZExprParser(ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.Ass()) {
	}
	
	static void Initialize();
	
	Node* Parse();
	
	Node* ParseBin(int prec, Node* left, CParser::Pos& backupPoint);
	Node* ParseAtom();
	Node* ParseNumeric();
	Node *ParseId();
	Node* ParseNamespace();
	Node* ParseNamespace(const String& s, Point opp);
	Node* ParseMember(ZNamespace& ns, const String& aName, const Point& opp);
	
	ZFunction* GetBase(ZMethodBundle* def, ZClass* spec, Vector<Node*>& params, int limit, bool conv, bool& ambig);
	
private:
	Assembly& ass;
	ZParser& parser;
	IR& irg;
	static bool initialized;
	
	int GetPriority(int& op, bool& opc);
	
	void IncompatOp(const ZSource& src, Point& p, const String& op, Node* left, Node* right) {
		ErrorReporter::ErrIncompatOp(src, p, op, ass.ClassToString(left), ass.ClassToString(right));
	}
	
	void getParams(Vector<Node*>& params, char end = ')');
	
	static Point OPS[256];
	static bool OPCONT[256];
};

#endif

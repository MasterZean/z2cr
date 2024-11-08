#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

class ZExprParser {
public:
	ZNamespaceSection* Section = nullptr;
	ZNamespace* Namespace = nullptr;
	ZFunction* Function = nullptr;
	
	ZExprParser(ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.Ass()) {
	}
	
	ZExprParser(ZEntity& entity, ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.Ass()) {
		ASSERT(entity.Section);
		Section = entity.Section;
		Namespace = &entity.Namespace();
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
	
	static ZClass* ParseType(Assembly& ass, ZParser& parser);
	
private:
	Assembly& ass;
	ZParser& parser;
	IR& irg;
	
	static bool initialized;
	
	int GetPriority(int& op, bool& opc);
	
	void IncompatOp(const ZSource& src, Point& p, const String& op, Node* left, Node* right) {
		ER::ErrIncompatOp(src, p, op, ass.ClassToString(left), ass.ClassToString(right));
	}
	
	void getParams(Vector<Node*>& params, char end = ')');
	
	static Point OPS[256];
	static bool OPCONT[256];
};

#endif

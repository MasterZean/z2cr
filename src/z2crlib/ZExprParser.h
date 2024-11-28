#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

#include <z2crlib/ZParser.h>
#include <z2crlib/ZIR.h>

class ZExprParser {
public:
	ZNamespaceSection* Section = nullptr;
	ZNamespace* Namespace = nullptr;
	ZFunction* Function = nullptr;
	ZClass* Class = nullptr;
	
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
	Node* ParseId();
	Node* ParseNamespace();
	Node* ParseNamespace(const String& s, Point opp);
	Node* ParseMember(ZNamespace& ns, const String& aName, const Point& opp);
	Node* ParseDot(Node* exp);
	
	ZFunction* GetBase(ZMethodBundle* def, ZClass* spec, Vector<Node*>& params, int limit, bool conv, bool& ambig);
	
	static ZClass* ParseType(Assembly& ass, ZParser& parser);
	static Node* Temporary(Assembly& ass, IR& irg, ZClass& cls, const Vector<Node*>&);
	
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
	
	void TestAccess(ZEntity& f, const Point& opp);
};

#endif

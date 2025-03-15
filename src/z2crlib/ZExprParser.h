#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

#include <z2crlib/ZParser.h>
#include <z2crlib/ZIR.h>
#include <z2crlib/ErrorReporter.h>
#include <z2crlib/ZCompiler.h>

class ZCompiler;

class ZExprParser {
public:
	ZNamespaceSection* Section = nullptr;
	ZNamespace* Namespace = nullptr;
	ZFunction* Function = nullptr;
	ZNamespace* Class = nullptr;
	
	//ZExprParser(ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.Ass()) {
	//}
	
	ZExprParser(ZEntity& entity, ZNamespace* ns, ZFunction* f, ZCompiler& aComp, ZParser& aPos, IR& aIrg): parser(aPos), comp(aComp), irg(aIrg), ass(aIrg.Ass()) {
		ASSERT(entity.Section);
		Section = entity.Section;
		Namespace = &entity.Namespace();
		Function = f;
		if (ns && ns->IsClass)
			Class = ns;

	}
	
	static void Initialize();
	
	Node* Parse(bool secondOnlyAttempt = false);
	
	Node* ParseBin(int prec, Node* left, CParser::Pos& backupPoint);
	Node* ParseAtom();
	Node* ParseNumeric();
	Node* ParseId();
	Node* ParseNamespace();
	Node* ParseNamespace(const String& s, Point opp);
	Node* ParseMember(ZNamespace& ns, const String& aName, const Point& opp, bool onlyStatic, Node* object = nullptr);
	Node* ParseDot(Node* exp);
	Node* ParseAtomClassInst(Node* exp);
	Node *ParseSpec(ZClass& mainClass, Node *exp, Vector<Node *>& nodes, const Point& p);
	
	ZFunction* GetBase(ZMethodBundle* def, ZClass* spec, Vector<Node*>& params, int limit, bool conv, bool& ambig);
	
	static ObjectInfo ParseType(ZCompiler& comp, ZParser& parser, ZNamespace* aclass = nullptr);
	
	Node* Temporary(ZClass& cls, Vector<Node*>&, const ZSourcePos* pos = nullptr);
	ZFunction* FindConstructor(ZClass& cls, Vector<Node*>& params, const ZSourcePos* pos);
	
private:
	Assembly& ass;
	ZParser& parser;
	IR& irg;
	ZCompiler& comp;
	
	Node* lastValid = nullptr;
	
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

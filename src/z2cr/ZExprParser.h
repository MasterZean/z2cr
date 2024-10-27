#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

class ZExprParser {
public:
	ZExprParser(ZParser& aPos, IR& aIrg): parser(aPos), irg(aIrg), ass(aIrg.GetAssembly()) {
	}
	
	static void Initialize();
	
	Node* Parse();
	
	Node* ParseAtom();
	
	Node* ParseNumeric();
	
private:
	Assembly& ass;
	ZParser& parser;
	IR& irg;
	
	static Point OPS[256];
	static bool OPCONT[256];
};

#endif

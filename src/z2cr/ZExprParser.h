#ifndef _z2cr_ExprePArser_h_
#define _z2cr_ExprePArser_h_

class ZExprParser {
public:
	ZExprParser(ZParser& aPos): parser(aPos) {
	}
	
	static void Initialize();
	
	void Parse();
	
	void ParseAtom();
	
	void ParseNumeric();
	
private:
	ZParser& parser;
	
	static Point OPS[256];
	static bool OPCONT[256];
};

#endif

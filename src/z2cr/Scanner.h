#ifndef _z2cr_Scanner_h_
#define _z2cr_Scanner_h_

#include "z2cr.h"

class Scanner {
public:
	Scanner(ZSource& aSrc, bool windows): parser(aSrc), ass(aSrc.GetAssembly()), win(windows) {
		parser.Mode = ": scan";
	}

	void Scan();
	
	void ScanNamespace();
	ZFunction& ScanFunc(bool aFunc);
	
protected:
	Assembly& ass;
	ZParser parser;
	String nameSpace;
	//Entity::AccessType insertAccess;
	String bindName;
	
	ZNamespace* nmspace = nullptr;
	
	bool isIntrinsic = false;
	bool isDllImport = false;
	bool isStdCall = false;
	bool isNoDoc = false;
	bool isForce = false;
	bool isCDecl = false;
	
	bool win;
	
	void InterpretTrait(const String& trait);
	void TraitLoop();
};

#endif

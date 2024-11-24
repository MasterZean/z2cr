#ifndef _z2cr_Scanner_h_
#define _z2cr_Scanner_h_

#include "z2cr.h"

enum class PlatformType {
	WINDOWS,
	POSIX
};
	
class ZScanner {
public:
	Array<ZException> Errors;
	
	ZScanner(ZSource& aSrc, PlatformType aType): parser(aSrc), source(aSrc), ass(aSrc.Package().Ass()), pt(aType) {
		parser.Mode = ": scan";
	}

	void Scan();
	
	void ScanSingle(const ZSourcePos& p);
	void ScanNamespace(const ZSourcePos& p);
	bool ScanVar(AccessType accessType, bool aConst, bool isStatic);
	ZFunction& ScanFunc(AccessType accessType, bool aFunc, bool isStatic);
	void ScanBlock();
	void ScanToken();
	bool ScanDeclaration(const ZSourcePos& p, AccessType accessType);
	void ScanType();
	void ScanUsing(const ZSourcePos& p);
	
protected:
	Assembly& ass;
	ZSource& source;
	ZParser parser;
	String bindName;
	Index<String> usingNames;
	bool inNamespaceBlock = false;
	bool inClass = false;
	
	ZNamespace* nameSpace = nullptr;
	ZNamespaceSection* section = nullptr;
	ZClass* curClass = nullptr;
	
	bool isIntrinsic = false;
	bool isDllImport = false;
	bool isStdCall = false;
	bool isNoDoc = false;
	bool isForce = false;
	bool isCDecl = false;
	
	PlatformType pt;
	int namespaceCount = 0;
	ZSourcePos namespacePos;
	
	void InterpretTrait(const String& trait);
	void TraitLoop();
	
	bool ScanDeclarationItem(AccessType accessType, const ZSourcePos* tp, bool isStatic);
	void ScanClassBody(const ZSourcePos& p, AccessType accessType, bool isStatic, const ZSourcePos* tp);
};

#endif

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
	
	void ScanSingle();
	void ScanNamespace();
	ZFunction& ScanFunc(AccessType accessType, bool aFunc);
	void ScanBlock();
	void ScanToken();
	bool ScanDeclaration(AccessType accessType);
	void ScanType();
	void ScanUsing();
	
protected:
	Assembly& ass;
	ZSource& source;
	ZParser parser;
	String nameSpace;
	String bindName;
	Index<String> usingNames;
	bool inNamespaceBlock = false;
	
	ZNamespace* nmspace = nullptr;
	ZNamespaceSection* section = nullptr;
	
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
	
	bool ScanDeclarationItem(AccessType accessType, ZSourcePos* tp = nullptr);
};

#endif

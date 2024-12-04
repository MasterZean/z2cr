#ifndef _z2cr_Scanner_h_
#define _z2cr_Scanner_h_

#include <z2crlib/ZParser.h>
#include <z2crlib/ZException.h>

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
	
	void ScanSingle(const ZSourcePos& p, bool isStatic);
	void ScanNamespace(const ZSourcePos& p);
	bool ScanVar(AccessType accessType, bool aConst, bool isStatic);
	bool ScanFuncMulti(AccessType accessType, const ZTrait& trait, int isCons, bool aFunc, bool isStatic);
	ZFunction& ScanFunc(AccessType accessType, int isCons, bool aFunc, bool isStatic);
	void ScanBlock();
	void ScanToken();
	bool ScanDeclaration(const ZSourcePos& p, AccessType accessType, bool isStatic);
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
	Vector<String> libLink;
	
	PlatformType pt;
	int namespaceCount = 0;
	ZSourcePos namespacePos;
	
	int InterpretTrait(ZParser& parser, int flags, const String& trait);
	int TraitLoop();
	
	bool ScanDeclarationItem(AccessType accessType, const ZTrait& trait, bool isStatic);
	bool ScanClassBody(const ZSourcePos& p, AccessType accessType, bool isStatic, const ZTrait& trait);
};

#endif

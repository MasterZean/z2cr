#ifndef _z2cr_Assembly_h_
#define _z2cr_Assembly_h_

enum class AccessType {
	Public,
	Private,
	Protected
};

class ZSource;

class ZSourcePos: Moveable<ZSourcePos> {
public:
	ZSource* Source = nullptr;
	Point P;
	CParser::Pos Pos;
	
	ZSourcePos() {
	}
	
	ZSourcePos(ZSource& aSrc, Point aPt): Source(&aSrc), P(aPt) {
	}
	
	ZSourcePos(ZSource& aSrc, Point aPt, CParser::Pos aPos): Source(&aSrc), P(aPt), Pos(aPos) {
	}
	
	String ToString() const;
};

class ZNamespace;
class ZPackage;

class ZFunction {
public:
	String Name;
	String BackName;
	
	bool IsFunction = false;
	bool InClass = false;
	
	CParser::Pos TraitPos;
	ZSourcePos DefPos;
	CParser::Pos ParamPos;
	CParser::Pos BodyPos;
	
	ZFunction(ZNamespace& aNmspace): nmsspace(aNmspace) {
	}
	
	ZNamespace& GetNamespace() {
		return nmsspace;
	}
	
	void GenerateSignatures();
	
	const String& DupSig() const {
		return dsig;
	}
	
private:
	ZNamespace& nmsspace;
	String dsig;
};

class ZDefinition {
public:
	Array<ZFunction> Functions;
};

class ZNamespace {
public:
	String Name;
	
	Array<ZFunction> PreFunctions;
	ArrayMap<String, ZDefinition> Definitions;
	
	ZFunction& PrepareFunction(const String& aName);
};

class Assembly {
public:
	ArrayMap<String, ZNamespace> Namespaces;
	ArrayMap<String, ZPackage> Packages;
	
	Assembly();
	
	ZNamespace& FindAddNamespace(const String& aName);
	
	ZNamespace& DefaultNamespace() {
		return Namespaces[0];
	}
	
	ZPackage& AddPackage(const String& aName, const String& aPath);
	
	ZSource* FindSource(const String& aName);
};

#endif

#ifndef _z2cr_Assembly_h_
#define _z2cr_Assembly_h_

enum class AccessType {
	Public,
	Private,
	Protected
};

class ZNamespace;

class ZFunction {
public:
	String Name;
	String BackName;
	
	bool IsFunction = false;
	
	CParser::Pos TraitPos;
	CParser::Pos DefPos;
	CParser::Pos ParamPos;
	CParser::Pos BodyPos;
	
	ZFunction(ZNamespace& aNmspace): nmsspace(aNmspace) {
	}
	
	ZNamespace& GetNamespace() {
		return nmsspace;
	}
	
private:
	ZNamespace& nmsspace;
};

class ZNamespace {
public:
	ZFunction& PrepareFunction(const String& aName);
	
private:
	Array<ZFunction> preFunctions;
};

class Assembly {
public:
	ArrayMap<String, ZNamespace> Namespaces;
	
	ZNamespace& FindAddNamespace(const String& aName);
	
	ZNamespace& DefaultNamespace() {
		return defaultNamespace;
	}
private:
	ZNamespace defaultNamespace;
};

#endif

#ifndef _z2cr_entities_h_
#define _z2cr_entities_h_

class ZNamespace;
class ZPackage;
class ZFunction;
class ZSource;

enum class AccessType {
	Public,
	Private,
	Protected
};

enum class EntityType {
	Unknown,
	Variable,
	Function,
	Class,
};

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

class ZClassScanInfo {
public:
	//String Namespace;
	WithDeepCopy<Vector<String>> TName;
	
	bool HasDefaultCons = false;
	bool IsEnum = false;
	bool IsTemplate = false;
	bool HasVirtuals = false;
	bool IsStatic = false;
};

class ZClassMeth {
public:
	ZFunction* CopyCon = nullptr;
	ZFunction* MoveCon = nullptr;
	ZFunction* Copy = nullptr;
	ZFunction* Move = nullptr;
	ZFunction* Default = nullptr;
	ZFunction* Allocate = nullptr;
	
	ZFunction* Eq = nullptr;
	ZFunction* Neq = nullptr;
	ZFunction* Less = nullptr;
	ZFunction* More = nullptr;
	ZFunction* LessEq = nullptr;
	ZFunction* MoreEq = nullptr;
};

class ZEntity {
public:
	String Name;
	String BackName;
	EntityType Type;
	
	ZSourcePos DefPos;
	
	ZEntity(ZNamespace& aNmspace): nmsspace(aNmspace) {
		Type = EntityType::Unknown;
	}
	
	ZNamespace& GetNamespace() {
		return nmsspace;
	}
	
private:
	ZNamespace& nmsspace;
};

class ZClass: public ZEntity, Moveable<ZClass> {
public:
	ZClassScanInfo Scan;
	ZClassMeth Meth;
	
	ZClass* ParamType;
	
	ZClass(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Class;
	}
	
	bool CoreSimple = false;
	bool IsDefined = true;

	bool IsEvaluated = false;
	
	bool MIsNumeric = false;
	bool MIsInteger = false;
	bool MIsRawVec = false;
	
	int Index = -1;
};

class ZVariable: public ZEntity {
public:
	ZVariable(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Variable;
	}
};

class ZFunction: public ZEntity {
public:
	bool IsFunction = false;
	bool InClass = false;
	
	ZSourcePos TraitPos;
	ZSourcePos ParamPos;
	ZSourcePos BodyPos;
	
	ZFunction(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Function;
	}
	
	void GenerateSignatures();
	
	const String& DupSig() const {
		return dsig;
	}
	
private:
	String dsig;
};

class ZDefinition {
public:
	Array<ZFunction*> Functions;
};

class ZNamespace {
public:
	String Name;
	
	Array<ZFunction> PreFunctions;
	ArrayMap<String, ZDefinition> Definitions;
	Array<ZVariable> PreVariables;
	
	ZFunction& PrepareFunction(const String& aName);
	ZVariable& PrepareVariable(const String& aName);
};

#endif

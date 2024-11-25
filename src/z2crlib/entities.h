#ifndef _z2cr_entities_h_
#define _z2cr_entities_h_

#include <z2crlib/common.h>
#include <z2crlib/node.h>

class ZSource;
class ZPackage;

class ZNamespace;
class ZVariable;
class ZMethodBundle;
class ZFunction;

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

class ZNamespaceItem {
public:
	ArrayMap<String, ZNamespaceItem> Names;
	ZNamespace* Namespace = nullptr;
	
	ZNamespaceItem* Add(const String& aName);
};

class ZNamespaceSection {
public:
	WithDeepCopy<Index<String>> UsingNames;
	Vector<ZNamespace*> Using;
};

class ZEntity {
public:
	String Name;
	String BackName;
	EntityType Type;
	AccessType Access;
	
	bool InClass = false;
		
	ZSourcePos DefPos;
	
	ZNamespaceSection* Section = nullptr;
	
	ZEntity(ZNamespace& aNmspace): nmsspace(aNmspace) {
		Type = EntityType::Unknown;
	}
	
	ZNamespace& Namespace() {
		return nmsspace;
	}
	
	inline Assembly& Ass();

private:
	ZNamespace& nmsspace;
};

class ZNamespace: public ZEntity {
public:
	String ProperName;
	
	ZNamespaceItem* NamespaceItem = nullptr;
	
	ZNamespace(Assembly& aAss): ZEntity(*this), ass(aAss) {
		Type = EntityType::Namespace;
	}
	
	ZNamespace(Assembly& aAss, ZNamespace& aNs): ZEntity(aNs), ass(aAss) {
		Type = EntityType::Namespace;
	}
	
	Array<ZFunction> PreFunctions;
	Array<ZVariable> PreVariables;
	Array<ZClass>    PreClasses;
	
	Array<ZNamespaceSection> Sections;
	ArrayMap<String, ZMethodBundle> Methods;
	VectorMap<String, ZVariable*> Variables;
	VectorMap<String, ZClass*> Classes;

	ZFunction& PrepareFunction(const String& aName);
	ZVariable& PrepareVariable(const String& aName);
	ZClass& PrepareClass(const String& aName);
	
	bool IsResolved = false;
	
	Assembly& Ass() {
		return ass;
	}
	
	bool HasMember(const String& aName);
	
private:
	Assembly& ass;
};

Assembly& ZEntity::Ass() {
	return nmsspace.Ass();
}

class ZClass: public ZNamespace, Moveable<ZClass> {
public:
	ZClassScanInfo Scan;
	ZClassMeth Meth;
	
	ObjectType Tt;
	ZClass* ParamType;
	
	int RTTIIndex = 0;
	
	ZClass(ZNamespace& aNmspace): ZNamespace(aNmspace.Ass(), aNmspace) {
		Type = EntityType::Class;
	}
	
	bool CoreSimple = false;
	bool IsDefined = true;

	bool IsEvaluated = false;
	
	bool MIsNumeric = false;
	bool MIsInteger = false;
	bool MIsRawVec = false;
	
	int Index = -1;
	
	const String& ColorSig() const {
		return csig;
	}
	
	void GenerateSignatures();
	
private:
	String csig;
};

class ZVariable: Moveable<ZVariable>, public ZEntity {
public:
	enum ParamType {
		tyAuto,
		tyRef,
		tyConstRef,
		tyVal,
		tyMove,
	};
	
	bool FromTemplate = false;
	bool IsDuplicate = false;
	bool IsConst = false;
	
	Node* Value = nullptr;
	ObjectInfo I;
	
	ParamType PType;
		
	ZVariable(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Variable;
	}
	
	const String& ColorSig() const {
		return csig;
	}
	
	void GenerateSignatures();
	
private:
	String csig;
};

class ZBlock: Moveable<ZBlock> {
public:
	WithDeepCopy<VectorMap<String, ZVariable*>> Locals;
	int Temps = 0;
};

class ZFunction: Moveable<ZFunction>, public ZEntity {
public:
	bool IsFunction = false;
	bool IsConstructor = false;
	bool IsValid = false;
	
	ZSourcePos TraitPos;
	ZSourcePos ParamPos;
	ZSourcePos BodyPos;
	
	Node Nodes;
	
	Array<ZVariable> Params;
	ObjectInfo Return;
	Vector<ZClass*> TParam;
	WithDeepCopy<Vector<ZBlock>> Blocks;
	Array<ZVariable> Locals;
	
	int Score = 0;
	
	ZFunction(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Function;
	}
	
	void GenerateSignatures();
	
	const String& FuncSig() const {
		return fsig;
	}
	
	const String& DupSig() const {
		return dsig;
	}
	
	const String& ColorSig() const {
		return csig;
	}
	
private:
	String dsig;
	String fsig;
	String csig;
};

class ZMethodBundle: public ZEntity {
public:
	Array<ZFunction*> Functions;
	
	ZMethodBundle(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::MethodBundle;
	}
};

#endif

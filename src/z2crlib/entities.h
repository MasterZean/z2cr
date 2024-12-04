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

class ZTrait {
public:
	enum {
		BINDC =   0b01,
		BINDCPP = 0b10
	};
	
	const ZSourcePos* TP = nullptr;
	int Flags = 0;
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
	bool IsStatic = false;
		
	ZSourcePos DefPos;
	
	ZNamespaceSection* Section = nullptr;
	
	ZEntity(ZNamespace& aNmspace): nmsspace(aNmspace) {
		Type = EntityType::Unknown;
		parent = &aNmspace;
	}
	
	ZEntity(const ZEntity& aEnitity) = default;
	
	inline ZNamespace& Namespace();
	
	inline ZNamespace& Owner();
	
	void SetOwner(ZNamespace& aNmspace) {
		parent = &aNmspace;
	}
	
	inline Assembly& Ass();
	
	virtual const String& ColorSig() const {
		return dummy;
	}
	
	virtual const String& OwnerSig() const {
		return dummy;
	}

private:
	ZNamespace& nmsspace;
	ZNamespace* parent;
	
	static String dummy;
};

class ZNamespace: public ZEntity {
public:
	Vector<String> NameElements;
	String ProperName;
	Vector<ZSource*> Sources;
	bool IsClass = false;
	String BackNameLegacy;
	String LegacySufix;
	WithDeepCopy<Vector<String>> LibLink;
	
	ZNamespaceItem* NamespaceItem = nullptr;
	
	ZNamespace(Assembly& aAss): ZEntity(*this), ass(aAss) {
		Type = EntityType::Namespace;
	}
	
	ZNamespace(Assembly& aAss, ZNamespace& aNs): ZEntity(aNs), ass(aAss) {
		Type = EntityType::Namespace;
	}
	
	ZNamespace(const ZNamespace& aNs): ZEntity(aNs), ass(aNs.Ass()) {
		ProperName = aNs.ProperName;
		LibLink = aNs.LibLink;
		
		for (int i = 0; i < aNs.PreConstructors.GetCount(); i++)
			PreConstructors.Add(aNs.PreConstructors[i]);
		for (int i = 0; i < aNs.PreFunctions.GetCount(); i++)
			PreFunctions.Add(aNs.PreFunctions[i]);
		for (int i = 0; i < aNs.PreVariables.GetCount(); i++)
			PreVariables.Add(aNs.PreVariables[i]);
		for (int i = 0; i < aNs.PreClasses.GetCount(); i++)
			PreClasses.Add(aNs.PreClasses[i]);
	}
	
	Array<ZFunction> PreFunctions;
	Array<ZFunction> PreConstructors;
	Array<ZVariable> PreVariables;
	Array<ZClass>    PreClasses;
	
	Array<ZNamespaceSection> Sections;
	ArrayMap<String, ZMethodBundle> Methods;
	VectorMap<String, ZVariable*> Variables;
	VectorMap<String, ZClass*> Classes;

	ZFunction& PrepareConstructor(const String& aName);
	ZFunction& PrepareFunction(const String& aName);
	ZVariable& PrepareVariable(const String& aName);
	ZClass&    PrepareClass(const String& aName);
	
	bool IsResolved = false;
	
	Assembly& Ass() {
		return ass;
	}
	
	Assembly& Ass() const {
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
	ObjectType Pt;
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
		
	void GenerateSignatures();
	
	const String& ColorSig() const {
		return csig;
	}
	
	const String& OwnerSig() const {
		return osig;
	}
	
private:
	String csig;
	String osig;
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
	
	ZVariable(const ZVariable& aEnitity) = default;
	
	const String& ColorSig() const {
		return csig;
	}
	
	const String& OwnerSig() const {
		return osig;
	}
	
	void GenerateSignatures();
	
private:
	String csig;
	String osig;
};

class ZBlock: Moveable<ZBlock> {
public:
	WithDeepCopy<VectorMap<String, ZVariable*>> Locals;
	int Temps = 0;
};

class ZFunction: Moveable<ZFunction>, public ZEntity {
public:
	bool IsFunction = false;
	int IsConstructor = 0;
	bool IsValid = false;
	bool IsDeleted = false;
	
	ZTrait Trait;
	ZSourcePos ParamPos;
	ZSourcePos BodyPos;
	
	Node Nodes;
	
	WithDeepCopy<Array<ZVariable>> Params;
	ObjectInfo Return;
	WithDeepCopy<Vector<ZClass*>> TParam;
	WithDeepCopy<Vector<ZBlock>> Blocks;
	WithDeepCopy<Array<ZVariable>> Locals;
	
	int Score = 0;
	
	ZFunction(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Function;
	}
	
	ZFunction(const ZFunction& aEnitity) = default;
	
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
	
	const String& OwnerSig() const {
		return osig;
	}
	
	const bool IsExternBind() const {
		return (Trait.Flags & ZTrait::BINDC) || (Trait.Flags & ZTrait::BINDCPP);
	}
	
private:
	String dsig;
	String fsig;
	String csig;
	String osig;
};

class ZMethodBundle: public ZEntity {
public:
	Array<ZFunction*> Functions;
	
	//TODO:
	bool IsTemplate = false;
	
	ZMethodBundle(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::MethodBundle;
	}
};

ZNamespace& ZEntity::Namespace() {
	if (InClass == true)
		return nmsspace.Namespace();
	else
		return nmsspace;
}

ZNamespace& ZEntity::Owner() {
	return *parent;
}


#endif

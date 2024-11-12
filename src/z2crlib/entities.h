#ifndef _z2cr_entities_h_
#define _z2cr_entities_h_

#include <Core/Core.h>

using namespace Upp;

class Assembly;
class ZNamespace;
class ZPackage;
class ZFunction;
class ZSource;
class ZClass;
class Node;

enum class AccessType {
	Public,
	Private,
	Protected
};

enum class EntityType {
	Unknown,
	Variable,
	MethodBundle,
	Function,
	Class,
};

class ObjectType {
public:
	ZClass* Class = nullptr;
	ObjectType* Next = nullptr;
	int Param = 0;
	
	ObjectType() {
	}
	
	ObjectType(ZClass& cls): Class(&cls) {
	}
};

class ObjectInfo: Moveable<ObjectInfo> {
public:
	ObjectType Tt;
	ZClass* C1 = nullptr;
	ZClass* C2 = nullptr;
	
	bool IsTemporary = false;
	bool IsIndirect = false;
	bool IsConst = false;
	bool IsRef = false;
	
	ObjectInfo() = default;
	
	ObjectInfo(ZClass* cls) {
		Tt.Class = cls;
	}
	
	ObjectInfo(ZClass* cls, bool ref) {
		Tt.Class = cls;
		//IsRef = ref;
	}
	
	ObjectInfo(ObjectType* tt) {
		Tt = *tt;
	}
	
	bool CanAssign(Assembly& ass, Node* node);
	bool CanAssign(Assembly& ass, ObjectInfo& sec, bool isCt);
};

class NodeType {
public:
	enum Type {
		Invalid,
		Const,
		BinaryOp,
		UnaryOp,
		Memory,
		Cast,
		Temporary,
		Def,
		List,
		Construct,
		Ptr,
		Index,
		SizeOf,
		Destruct,
		Property,
		Deref,
		Intrinsic,
		Return,
		Local,
		Alloc,
		Array,
		Using,
		Params,
		Block,
		If,
		While,
		DoWhile,
		Switch,
	};
};

class Node: public ObjectInfo {
public:
	Node* Next = nullptr;
	Node* Prev = nullptr;
	Node* First = nullptr;
	Node* Last = nullptr;

	NodeType::Type NT = NodeType::Invalid;
	
	bool IsCT = false;
	bool IsLiteral = false;
	bool IsSymbolic = false;
	
	bool HasSe = false;
	bool LValue = false;
	
	bool IsRef = false;
	bool IsAddressable = false;
	bool IsIndirect = false;
	
	double DblVal = 0;
	int64  IntVal = 0;

	void SetType(ObjectType* type, ZClass* efType, ZClass* sType) {
		Tt = *type;
		C1 = efType;
		C2 = sType;
	}

	void SetType(ObjectType* type) {
		Tt = *type;
		C1 = type->Class;
		C2 = nullptr;
	}

	void SetType(ObjectType type) {
		Tt = type;
		C1 = type.Class;
		C2 = nullptr;
	}
	
	void SetValue(int i, double d) {
		IntVal = i;
		DblVal = d;
	}

	void AddChild(Node* node) {
		if (First == NULL) {
			First = node;
			Last = node;
		}
		else {
			node->Prev = Last;
			Last->Next = node;

			Last =  node;
		}
	}
	
	bool IsZero(Assembly& ass);
	void PromoteToFloatValue(Assembly& ass);
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

class ZNamespaceItem {
public:
	ArrayMap<String, ZNamespaceItem> Names;
	ZNamespace* Namespace = nullptr;
	
	ZNamespaceItem* Add(const String& aName);
};

class ZFunction;
class ZVariable;
class ZMethodBundle;
class Assemly;

class ZNamespaceSection {
public:
	WithDeepCopy<Index<String>> UsingNames;
	Vector<ZNamespace*> Using;
};

class ZNamespace {
public:
	String Name;
	String BackName;
	
	ZNamespaceItem* NamespaceItem = nullptr;
	
	ZNamespace(Assembly& aAss): ass(aAss) {
	}
	
	Array<ZFunction> PreFunctions;
	Array<ZVariable> PreVariables;
	
	Array<ZNamespaceSection> Sections;
	ArrayMap<String, ZMethodBundle> Methods;
	VectorMap<String, ZVariable*> Variables;
	VectorMap<String, ZClass*> Classes;

	ZFunction& PrepareFunction(const String& aName);
	ZVariable& PrepareVariable(const String& aName);
	
	bool IsResolved = false;
	
	Assembly& Ass() {
		return ass;
	}
	
	bool HasMember(const String& aName);
	
private:
	Assembly& ass;
};

class ZEntity {
public:
	String Name;
	String BackName;
	EntityType Type;
	
	ZSourcePos DefPos;
	
	ZNamespaceSection* Section = nullptr;
	
	ZEntity(ZNamespace& aNmspace): nmsspace(aNmspace) {
		Type = EntityType::Unknown;
	}
	
	ZNamespace& Namespace() {
		return nmsspace;
	}
	
	Assembly& Ass() {
		return nmsspace.Ass();
	}

private:
	ZNamespace& nmsspace;
	
};

class ZClass: public ZEntity, Moveable<ZClass> {
public:
	ZClassScanInfo Scan;
	ZClassMeth Meth;
	
	ObjectType Tt;
	ZClass* ParamType;
	
	int RTTIIndex = 0;
	
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
	
	Node* Value = nullptr;
	ObjectInfo I;
	
	ParamType PType;
		
	ZVariable(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Variable;
	}
};

class ZBlock: Moveable<ZBlock> {
public:
	WithDeepCopy<VectorMap<String, ZVariable*>> Locals;
	int Temps = 0;
};

class ZFunction: Moveable<ZFunction>, public ZEntity {
public:
	bool IsFunction = false;
	bool InClass = false;
	
	ZSourcePos TraitPos;
	ZSourcePos ParamPos;
	ZSourcePos BodyPos;
	
	Node Nodes;
	
	Array<ZVariable> Params;
	Vector<ZClass*> TParam;
	WithDeepCopy<Vector<ZBlock>> Blocks;
	Array<ZVariable> Locals;
	
	int Score = 0;
	
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

class ZMethodBundle: public ZEntity {
public:
	Array<ZFunction*> Functions;
	
	ZMethodBundle(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::MethodBundle;
	}
};

inline bool operator==(const ObjectType& Tt, ObjectType* tt) {
	return Tt.Class == tt->Class;
}

inline bool operator==(const ObjectInfo& t1, const ObjectInfo& t2) {
	return t1.Tt.Class == t2.Tt.Class && t1.IsIndirect == t2.IsIndirect && /*t1.IsConst == t2.IsConst &&*/ t1.IsTemporary == t2.IsTemporary;
}

inline bool operator!=(const ObjectInfo& t1, const ObjectInfo& t2) {
	return !(t1 == t2);
}

#endif

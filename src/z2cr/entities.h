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

enum class AccessType {
	Public,
	Private,
	Protected
};

enum class EntityType {
	Unknown,
	Variable,
	FunctionSet,
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
	bool IsConst = false;
	
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
		Var,
		Alloc,
		Array,
		Using,
		Params,
		Block,
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
	
	ObjectType Tt;
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
	Node* Value = nullptr;
	ObjectType Tt;
	
	ZVariable(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::Variable;
	}
};

class ZBlock: Moveable<ZBlock> {
public:
	WithDeepCopy<VectorMap<String, ZVariable*>> Vars;
	int Temps = 0;
};

class ZFunction: public ZEntity {
public:
	bool IsFunction = false;
	bool InClass = false;
	
	ZSourcePos TraitPos;
	ZSourcePos ParamPos;
	ZSourcePos BodyPos;
	
	Node Nodes;
	
	WithDeepCopy<Vector<ZBlock>> Blocks;
	
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

class ZDefinition: public ZEntity {
public:
	Array<ZFunction*> Functions;
	
	ZDefinition(ZNamespace& aNmspace): ZEntity(aNmspace) {
		Type = EntityType::FunctionSet;
	}
};

class ZNamespaceItem {
public:
	ArrayMap<String, ZNamespaceItem> Names;
	ZNamespace* Namespace = nullptr;
	
	ZNamespaceItem* Add(const String& aName);
};

class ZNamespace {
public:
	String Name;
	String BackName;
	
	Array<ZFunction> PreFunctions;
	Array<ZVariable> PreVariables;
	
	ArrayMap<String, ZDefinition> Definitions;
	ArrayMap<String, ZVariable*> Variables;
	
	ZFunction& PrepareFunction(const String& aName);
	ZVariable& PrepareVariable(const String& aName);
};

#endif

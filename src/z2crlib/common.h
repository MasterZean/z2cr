#ifndef _z2crlib_common_h_
#define _z2crlib_common_h_

#include <Core/Core.h>

using namespace Upp;

class Assembly;
class ZClass;
class Node;

enum class AccessType {
	Public,
	Protected,
	Private,
};

enum class EntityType {
	Unknown,
	Namespace,
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
	bool IsAddressable = false;
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

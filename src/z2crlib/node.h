#ifndef _z2crlib_node_h_
#define _z2crlib_node_h_

#include <z2crlib/common.h>

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
		Ptr,
		Index,
		SizeOf,
		//Property,
		Deref,
		Intrinsic,
		Return,
		Local,
		Alloc,
		Array,
		Using,
		//Params,
		Block,
		If,
		While,
		DoWhile,
		ForLoop,
		Switch,
		LoopControl,
		Chain,
		Construct,
		Destruct,
		Throw,
	};
};

class ChainNode;

class Node: public ObjectInfo {
public:
	Node* Next = nullptr;
	Node* Prev = nullptr;
	Node* First = nullptr;
	Node* Last = nullptr;
	
	ChainNode* Chain = nullptr;

	NodeType::Type NT = NodeType::Invalid;
	
	bool IsCT = false;
	bool IsConst = false;
	bool IsLiteral = false;
	bool IsSymbolic = false;
	
	bool HasSe = false;
	bool IsEffLValue = false;
	
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
	
	bool IsLValue() const {
		return IsEffLValue || (IsAddressable && IsConst == false);
	}
};

#endif

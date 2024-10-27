#ifndef _z2cr_ZNode_h_
#define _z2cr_ZNode_h_

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

	void Add(Node* node) {
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
};

class ConstNode: public Node {
public:
	//Constant* Const = nullptr;
	int Base = 10;

	ConstNode() {
		NT = NodeType::Const;
	}
};

class OpNode: public Node {
public:
	enum Type {
		opNotSet = -1,
		opAdd = 0,
		opSub = 1,
		opMul = 2,
		opDiv = 3,
		opMod = 4,
		opShl = 5,
		opShr = 6,
		opLess = 7,
		opLessEq = 8,
		opMore   = 9,
		opMoreEq = 10,
		opEq = 11,
		opNeq = 12,
		opBitAnd = 13,
		opBitXor = 14,
		opBitOr = 15,
		opLogAnd = 16,
		opLogOr = 17,
		opAssign = 18,
		opPlus = 19,
		opMinus = 20,
		opNot = 21,
		opComp = 22,
		opInc = 23,
		opDec = 24,
		opTernary = 25,
	};
	
	Type Op = opNotSet;
	bool Construct = false;
	int Move = 0;
	bool Assign = false;
	
	Node* OpA = nullptr;
	Node* OpB = nullptr;
	Node* OpC = nullptr;

	OpNode() {
		NT = NodeType::BinaryOp;
	}
};

class UnaryOpNode: public OpNode {
public:
	bool Prefix = false;
	
	UnaryOpNode() {
		NT = NodeType::UnaryOp;
	}
};

#endif

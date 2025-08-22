#ifndef _z2cr_ZNode_h_
#define _z2cr_ZNode_h_

#include <z2crlib/node.h>

class ConstNode: public Node {
public:
	//Constant* Const = nullptr;
	int Base = 10;

	ConstNode() {
		NT = NodeType::Const;
	}
};

class CastNode: public Node {
public:
	Node* Object = nullptr;
	bool Ptr = false;
	
	CastNode() {
		NT = NodeType::Cast;
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

class ParamsNode: public Node {
public:
	Vector<Node*> Params;
	
	ParamsNode() {
		NT = NodeType::Params;
	}
};

class DefNode: public ParamsNode {
public:
	ZFunction* Function = nullptr;
	Node* Object = nullptr;
	bool Property = false;
	bool IsDestructor = false;

	DefNode() {
		NT = NodeType::Def;
	}
};

class TempNode: public ParamsNode {
public:
	ZFunction *Constructor = nullptr;

	TempNode() {
		NT = NodeType::Temporary;
	}
};

class ListNode: public ParamsNode {
public:
	ListNode() {
		NT = NodeType::List;
	}
};

class BlockNode: public Node {
public:
	Node Nodes;
	bool EndBlockStat = true;
	
	BlockNode() {
		NT = NodeType::Block;
	}
};

class MemNode: public Node/*: public ObjectNode*/ {
public:
	//String Mem;
	ZEntity* Mem = nullptr;
	Node* Object = nullptr;

	bool IsThis = false;
	bool IsThisNop = false;
	bool IsParam = false;
	bool IsLocal = false;
	bool IsClass = false;

	MemNode() {
		NT = NodeType::Memory;
	}
	
	/*ZVariable* GetFullMemberAssignment() {
		if (Object != nullptr) {
			if (Object->NT == NodeType::Memory && ((MemNode*)Object)->IsThis) {
			}
			else
				return nullptr;
		}
		
		if (Var && IsClass)
			return Var;
		
		if (Object == nullptr)
			return nullptr;
		
		if (Object->NT != NodeType::Memory)
			return nullptr;
		
		MemNode& mem = *(MemNode*)(Object);
		if (mem.IsThis == false)
			return nullptr;
		
		if (Var == nullptr)
			return nullptr;
		
		return Var;
	}*/
};

class IfNode: public Node {
public:
	Node* Cond = nullptr;
	Node* TrueBranch = nullptr;
	Node* FalseBranch = nullptr;
	
	IfNode() {
		NT = NodeType::If;
	}
};

class WhileNode: public Node {
public:
	Node* Cond = nullptr;
	Node* Body = nullptr;
	
	WhileNode() {
		NT = NodeType::While;
	}
};

class DoWhileNode: public WhileNode {
public:
	DoWhileNode() {
		NT = NodeType::DoWhile;
	}
};

class ForLoopNode: public Node {
public:
	Node* Init = nullptr;
	Node* Cond = nullptr;
	Node* Iter = nullptr;
	Node* Body = nullptr;
	
	ForLoopNode() {
		NT = NodeType::ForLoop;
	}
};

class LocalNode: public Node {
public:
	ZVariable* Var = nullptr;

	LocalNode() {
		NT = NodeType::Local;
	}
};

class ReturnNode: public Node {
public:
	Node* Value = nullptr;

	ReturnNode() {
		NT = NodeType::Return;
	}
};

class IntrinsicNode: public Node {
public:
	Vector<Node*> Value;

	IntrinsicNode() {
		NT = NodeType::Intrinsic;
	}
};

class LoopControlNode: public Node {
public:
	bool Break = false;

	LoopControlNode() {
		NT = NodeType::LoopControl;
	}
};

class PtrNode: public Node {
public:
	//bool Ref = false;
	//bool Move = false;
	bool Nop = false;
	Node* Object = nullptr;

	PtrNode() {
		NT = NodeType::Ptr;
	}
};

class IndexNode: public Node {
public:
	Node* Object = nullptr;
	Node* Index = nullptr;
	
	IndexNode() {
		NT = NodeType::Index;
	}
};

class ChainNode: public Node {
public:
	int Count = 0;
	int PropCount = 0;
	
	ChainNode() {
		NT = NodeType::Chain;
	}
};

#endif

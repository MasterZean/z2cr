#ifndef _z2cr_ZNode_h_
#define _z2cr_ZNode_h_

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

#endif

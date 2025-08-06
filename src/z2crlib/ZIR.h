#ifndef _z2cr_ZIR_h_
#define _z2cr_ZIR_h_

#include <z2crlib/ZNode.h>

template<class Tt>
class NodePool {
private:
	Vector<Tt*> nodes;
	int count;

public:
	NodePool() {
		count = 0;
	}

	~NodePool() {
		for (int i = 0; i < nodes.GetCount(); i++)
			delete nodes[i];
	}

	inline Tt* Get() {
		Tt* node;

		if (count >= nodes.GetCount())
			node = nodes.Add(new Tt);
		else {
			node = nodes[count];
			*node = Tt();
		}

		count += 1;

		return node;
		//return new Tt();
	}

	inline void Clear() {
		count = 0;
	}
};

class IR {
protected:
	Assembly& ass;
		
	NodePool<OpNode> opNodes;
	NodePool<UnaryOpNode> unaryOpNodes;
	NodePool<ConstNode> constNodes;
	NodePool<CastNode> castNodes;
	
	NodePool<MemNode> memNodes;
	NodePool<DefNode> defNodes;
	NodePool<BlockNode> blockNodes;
	NodePool<IfNode> ifNodes;
	NodePool<WhileNode> whileNodes;
	NodePool<DoWhileNode> doWhileNodes;
	NodePool<ForLoopNode> forLoopNodes;
	NodePool<LocalNode> localNodes;
	NodePool<ReturnNode> retNodes;
	NodePool<IntrinsicNode> intNodes;
	NodePool<LoopControlNode> loopControlNodes;
	NodePool<TempNode> tempNodes;
	NodePool<ListNode> listNodes;
	NodePool<PtrNode> ptrNodes;
	NodePool<IndexNode> indexNodes;
		
public:
	public:
	bool FoldConstants = false;
	
	IR(Assembly& ass): ass(ass) {
	}
	
	Assembly& Ass() const {
		return ass;
	}
	
	void fillSignedTypeInfo(int64 l, Node* node, ZClass* cls = nullptr);
	void fillUnsignedTypeInfo(uint64 l, Node* node, ZClass* cls = nullptr);

	ConstNode* const_i(int64 l, ZClass* cls = nullptr, int base = 10);
	ConstNode* const_u(uint64 l, ZClass* cls = nullptr, int base = 10);
	ConstNode* const_r32(double l);
	ConstNode* const_r64(double l);

	ConstNode* const_char(int l, int base = 10);
	
	ConstNode* const_bool(bool l);

	ConstNode* const_void();
	ConstNode* const_null();
	
	ConstNode* const_str(int index);
	
	Node* const_class(ZClass& cls, Node* e = nullptr);
	
	Node* opArit(Node* left, Node* right, OpNode::Type op, const Point& p);
	Node* opRel(Node* left, Node* right, OpNode::Type op, const Point& p);
	Node* opLog(Node* left, Node* right, OpNode::Type op);
	
	Node* op_bitand(Node* left, Node* right);
	Node* op_bitor(Node* left, Node* right);
	Node* op_bitxor(Node* left, Node* right);
			
	Node* op_shl(Node* left, Node* right, const Point& p);
	Node* op_shr(Node* left, Node* right, const Point& p);
	
	Node *opTern(Node *cond, Node *left, Node *right);
	
	Node* minus(Node* node);
	Node* plus(Node* node);
	Node* op_not(Node* node);
	Node* bitnot(Node* node);
	Node* inc(Node* node, bool prefix = false);
	Node* dec(Node* node, bool prefix = false);
	
	Node* deref(Node* node);
	
	Node* cast(Node* left, ObjectType* tt, bool sc = true, bool ptr = false);
	
	ParamsNode* callfunc(ZFunction& over, Node* object);
	
	MemNode*   mem_var(ZEntity& var, Node* object = nullptr);
	TempNode*  mem_temp(ZClass& cls, ZFunction *constructor);
	PtrNode*   mem_ptr(Node* object);
	IndexNode* mem_array(Node *object, Node *index);
	MemNode*   mem_this(ZClass& cls);
	
	BlockNode* block();
	IfNode* ifcond(Node* cond, Node* truebranch, Node* falsebranch);
	WhileNode* whilecond(Node* cond, Node* body);
	DoWhileNode* dowhilecond(Node* cond, Node* body);
	ForLoopNode* forloop(Node* init, Node* cond, Node* iter, Node* body);

	ReturnNode* ret(Node* node);
	LoopControlNode* loopControl(bool aBreak);
	
	IntrinsicNode* intrinsic(Node* node);
	
	LocalNode* local(ZVariable& v);
	
	Node* attr(Node* left, Node* right);
	ListNode* list(Node *node);
	
	Node* op(Node* left, Node* right, OpNode::Type op, const Point& p) {
		if (op <= OpNode::opMod)
			return opArit(left, right, op, p);
		else if (op <= OpNode::opShl)
			return op_shl(left, right, p);
		else if (op <= OpNode::opShr)
			return op_shr(left, right, p);
		else if (op <= OpNode::opNeq)
			return opRel(left, right, op, p);
		else if (op <= OpNode::opBitAnd)
			return op_bitand(left, right);
		else if (op <= OpNode::opBitXor)
			return op_bitxor(left, right);
		else if (op <= OpNode::opBitOr)
			return op_bitor(left, right);
		else if (op <= OpNode::opLogOr)
			return opLog(left, right, op);
		else {
			ASSERT(0);
			return nullptr;
		}
	}
};

#endif

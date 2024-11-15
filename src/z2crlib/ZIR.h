#ifndef _z2cr_ZIR_h_
#define _z2cr_ZIR_h_

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
	NodePool<LocalNode> localNodes;
	NodePool<ReturnNode> retNodes;
	NodePool<IntrinsicNode> intNodes;
	
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
	
	Node* const_class(ZClass& cls, Node* e = nullptr);
	
	Node* opArit(Node* left, Node* right, OpNode::Type op, const Point& p);
	Node* opRel(Node* left, Node* right, OpNode::Type op, const Point& p);
	Node* opLog(Node* left, Node* right, OpNode::Type op);
	
	Node* op_bitand(Node* left, Node* right);
	Node* op_bitor(Node* left, Node* right);
	Node* op_bitxor(Node* left, Node* right);
			
	Node* op_shl(Node* left, Node* right, const Point& p);
	Node* op_shr(Node* left, Node* right, const Point& p);
	
	Node* deref(Node* node);
	
	Node* cast(Node* left, ObjectType* tt, bool sc = true, bool ptr = false);
	
	ParamsNode* mem_def(ZFunction& over, Node* object);
	MemNode* mem_var(ZEntity* var);
	
	BlockNode* block();
	IfNode* ifcond(Node* cond, Node* truebranch, Node* falsebranch);
	WhileNode* whilecond(Node* cond, Node* body);
	DoWhileNode* dowhilecond(Node* cond, Node* body);
	ReturnNode* ret(Node* node);
	
	IntrinsicNode* intrinsic(Node* node);
	
	LocalNode* local(ZVariable& v);
	
	Node* attr(Node* left, Node* right);
	
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

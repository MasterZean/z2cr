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
	
public:
	public:
	bool FoldConstants = true;
	
	IR(Assembly& ass): ass(ass) {
	}
	
	Assembly& GetAssembly() const {
		return ass;
	}
	
	void fillSignedTypeInfo(int64 l, Node* node, ZClass* cls = nullptr);
	void fillUnsignedTypeInfo(uint64 l, Node* node, ZClass* cls = nullptr);

	ConstNode* const_i(int64 l, ZClass* cls = nullptr, int base = 10);
	ConstNode* const_u(uint64 l, ZClass* cls = nullptr, int base = 10);
	ConstNode* const_r32(double l);
	ConstNode* const_r64(double l);
};

#endif

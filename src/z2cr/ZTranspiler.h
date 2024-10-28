#ifndef _z2cr_Transpiler_h_
#define _z2cr_Transpiler_h_

class ZNodeWalker {
public:
	ZNodeWalker(Assembly& aAss, Stream& aStream): ass(aAss), cs(aStream) {
	}
	
protected:
	Assembly& ass;
	Stream& cs;
	
	int indent = 0;
	
	void NL() {
		for (int i = 0; i < indent; i++)
			cs << '\t';
	}

	void EL() {
		cs << '\n';
	}

	void ES() {
		cs << ";\n";
	}

	virtual bool Traverse() {
		return false;
	}

	void Walk(Node* node, Stream& ss) {
		/*Stream* back = cs;
		cs = &ss;
		WalkNode(node);
		cs = back;*/
		WalkNode(node);
	}

	virtual void WalkNode(Node* node) {
	}

	virtual void WalkStatement(Node* node) {
	}
};

class ZTranspiler: public ZNodeWalker {
public:
	ZTranspiler(Assembly& aAss, Stream& aStream): ZNodeWalker(aAss, aStream) {
	}
	
	void WriteIntro();
	
	void WriteFunctionDecl(ZFunction& f);
	void WriteFunctionBody(Vector<Node*>& nodes);
	
	void Walk(ConstNode& node, Stream& stream);
};

#endif

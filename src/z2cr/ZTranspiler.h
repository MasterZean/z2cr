#ifndef _z2cr_Transpiler_h_
#define _z2cr_Transpiler_h_

class ZNodeWalker {
public:
	ZNodeWalker(ZCompiler& aComp, Stream& aStream): comp(aComp), ass(aComp.GetAssembly()), cs(aStream) {
	}
	
protected:
	ZCompiler& comp;
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
	ZTranspiler(ZCompiler& aComp, Stream& aStream): ZNodeWalker(aComp, aStream) {
	}
	
	void WriteIntro();
	void WriteOutro();
	
	void WriteFunctionDef(ZFunction& f);
	void WriteFunctionDecl(ZFunction& f);
	void WriteFunctionBody(Vector<Node*>& nodes);
	
	void Walk(ConstNode& node, Stream& stream);
	void Walk(OpNode& node);
	
	void Walk(Node* node);
	
	void TranspileDeclarations(ZNamespace& ns);
	
	virtual void WalkNode(Node* node) {
		Walk(node);
	}
};

#endif

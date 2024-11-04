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
	void WriteFunctionBody(ZFunction& f);
	
	void Proc(ConstNode& node, Stream& stream);
	void Proc(OpNode& node);
	void Proc(DefNode& node);
	void Proc(MemNode& node);
	void Proc(BlockNode& node);
	void Proc(IfNode& node);
	
	void TranspileDeclarations(ZNamespace& ns);
	void TranspileDefinitions(ZNamespace& ns);
	
	virtual void WalkNode(Node* node);
	
private:
	void Walk(Node* node);
	void WalkChildren(Node* node);
};

#endif

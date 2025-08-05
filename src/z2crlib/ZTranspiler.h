#ifndef _z2cr_Transpiler_h_
#define _z2cr_Transpiler_h_

#include <z2crlib/ZCompiler.h>

class ZNodeWalker {
public:
	ZNodeWalker(ZCompiler& aComp, Stream& aStream): comp(aComp), ass(aComp.Ass()), cs(aStream) {
	}
	
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
	
protected:
	ZCompiler& comp;
	Assembly& ass;
	Stream& cs;
	
	int indent = 0;

	virtual bool Traverse() {
		return false;
	}

	void Walk(Node* node, Stream& ss) {
		WalkNode(node);
	}

	virtual void WalkNode(Node* node) {
	}

	virtual void WalkStatement(Node* node) {
	}
};

class ZTranspiler: public ZNodeWalker {
public:
	int CppVersion = 2017;
	bool CheckUse = true;
	
	ZTranspiler(ZCompiler& aComp, Stream& aStream): ZNodeWalker(aComp, aStream) {
	}
	
	void WriteIntro();
	void WriteClassForward();
	void WriteOutro();
	
	void WriteFunctionDef(ZFunction& f);
	void WriteFunctionDecl(ZFunction& f);
	void WriteFunctionParams(ZFunction& f);
	void WriteFunctionBody(ZFunction& f, bool wrap = true);
	
	void Proc(ConstNode& node, Stream& stream);
	void Proc(OpNode& node);
	void Proc(UnaryOpNode& node);
	void Proc(DefNode& node);
	void Proc(MemNode& node);
	void Proc(BlockNode& node);
	void Proc(IfNode& node);
	void Proc(WhileNode& node);
	void Proc(DoWhileNode& node);
	void Proc(ForLoopNode& node);
	void Proc(LocalNode& node);
	void Proc(CastNode& node);
	void Proc(ReturnNode& node);
	void Proc(IntrinsicNode& node);
	void Proc(LoopControlNode& node);
	void Proc(TempNode& node);
	void Proc(ListNode& node);
	void Proc(PtrNode& node);
	void Proc(IndexNode& node);
	
	void BeginNamespace(ZNamespace& ns) {
		inNamespace = &ns;
		firstInNamespace = true;
		namespaceWrites = 0;
	}
	
	void NewMember() {
		if (inNamespace) {
			if (firstInNamespace) {
				NL();
				cs << "namespace ";
				NsIntro(*inNamespace);
				EL();
				
				indent++;
				
				firstInNamespace = false;
			}
			
			namespaceWrites++;
		}
		
		if (inClass) {
			if (firstInClass) {
				if (classWrites)
					NL();
				
				NL();
				cs << "class " << inClass->BackName << " {";
				EL();
				
				indent++;
				
				firstInClass = false;
			}
			
			classWrites++;
		}
	}
	
	void EndNamespace() {
		if (inNamespace && namespaceWrites) {
			indent--;
			
			NL();
			NsOutro(*inNamespace);
			EL();
			EL();
		}
		
		inNamespace = nullptr;
	}
	
	void BeginClass(ZNamespace& cls) {
		inClass = &cls;
		firstInClass = true;
		classWrites = 0;
	}
	
	void EndClass() {
		if (inClass && classWrites) {
			indent--;
			
			NL();
			cs << "}";
			ES();
			//EL();
		}
		
		inClass = nullptr;
	}
	
	void TranspileDeclarations(ZNamespace& ns, int accessFlags, bool classes);
	void TranspileNamespaceDecl(ZNamespace& ns, int accessFlags = 0, bool doBinds = false);
	bool TranspileClassDeclMaster(ZNamespace& cls, int accessFlags);
	int  TranspileClassDecl(ZNamespace& ns, int accessFlags = 0);
	int  TranspileMemberDeclVar(ZNamespace& ns, int accessFlags);
	int  TranspileMemberDeclFunc(ZNamespace& ns, int accessFlags, bool doBinds, int vc);

	void TranspileDefinitions(ZNamespace& ns, bool vars = true, bool fDecl = true, bool wrap = true);
	void TranspileValDefintons(ZNamespace& ns, bool trail = true);
	
	virtual void WalkNode(Node* node);
	
private:
	void Walk(Node* node);
	void WalkChildren(Node* node);
	
	ZNamespace* inNamespace = nullptr;
	ZNamespace* inClass = nullptr;
	bool firstInNamespace = true;
	bool firstInClass = true;
	int namespaceWrites = 0;
	int classWrites = 0;
	
	void WriteClassAccess(AccessType access);
	
	void NsIntro(ZNamespace& ns);
	void NsOutro(ZNamespace& ns);
	
	void WriteType(ObjectType* tt);
	void WriteTypePost(ObjectType *tt);
};

#endif

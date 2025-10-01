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
	
	bool WriteFunctionDef(ZFunction& f);
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
	void Proc(DestructNode& node);
	void Proc(ThrowNode& node);
	void Proc(PlacementNewNode& node);
	
	void ProcLeftSet(Node* l, Node* r, OpNode::Type extraOp, Node* extra);
	void ProcLeftSet(String& cs, Node *n);
	void WriteCArrayVarBoiler(ZVariable& var);
	
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
				cs << "class ";
				WriteClassName(*inClass);
				if (inClass->Super) {
					cs << ": public ";
					cs << inClass->Super->Owner().BackName;
					cs << "::";
					WriteClassName(*inClass->Super);
				}
				cs << " {";
				EL();
				
				indent++;
				
				if (inClass->CABICompensation) {
					cs << "\tpublic:" << "\n";
					cs << "\t\t";
					WPClsName(*inClass);
					cs << "(_";
					WPClsName(*inClass);
					cs << " v";
					cs << ") {\n";
					
					cs << "\t\t\tmemcpy(this, &v, sizeof(_";
					WPClsName(*inClass);
					cs << "));\n";
					
					cs << "\t\t}\n";
				}
				
				// todo: fix
				if (inClass == ass.CString) {
					cs << "\tpublic:" << "\n";
					cs << "\t\tString(): text((uint8*)\"\") {}" << "\n";
					cs << "\t\tString(const char *aTxt): text((uint8*)aTxt) {}" << "\n";
					cs << "\tpublic:" << "\n";
					cs << "\t\tuint8 *text = nullptr;" << "\n";
				}
				
				if (inClass && inClass->TBase == ass.CSlice && inClass->T->TBase != ass.CRaw) {
					cs << "\tpublic:" << "\n";
					
					cs << "\t\t";
					WStorageName(*inClass->T);

					cs << "& operator[](size_t idx) {\n";
					cs << "\t\t\t";
					cs << "if (idx >= length) throw sys::exception::IndexOutOfBounds();\n";
					cs << "\t\t\t";
					cs << "return ptr[idx];\n";
					cs << "\t\t}\n";
					
					cs << "\t\t";
					cs << "const ";
					WStorageName(*inClass->T);

					cs << "& operator[](size_t idx) const {\n";
					cs << "\t\t\t";
					cs << "if (idx >= length) throw sys::exception::IndexOutOfBounds();\n";
					cs << "\t\t\t";
					cs << "return ptr[idx];\n";
					cs << "\t\t}\n";
				}
				
				if (inClass && inClass->TBase == ass.CRaw && inClass->T->TBase != ass.CRaw) {
					cs << "\tpublic:" << "\n";
					cs << "\t\t";
					WriteClassName(*inClass);
					cs << "(";
					WStorageName(*inClass->T);
					cs << "* aPtr, size_t aLen): ";
					WriteClassName(*inClass->Super);
					cs << "(aPtr, aLen) {}\n";
				}
				
				firstInClass = false;
			}
			
			classWrites++;
		}
	}
	
	void WriteClassName(ZClass& cls) {
		if (!cls.CoreSimple)
			cs << cls.BackName;
		else
			cs << cls.Name;
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
		inClass = (ZClass*)&cls;
		firstInClass = true;
		classWrites = 0;
	}
	
	void EndClass() {
		if (inClass && classWrites) {
			indent--;
			
			NL();
			cs << "}";
			ES();
		}
		
		inClass = nullptr;
	}
	
	void TranspileDeclarations(ZNamespace& ns, int accessFlags, bool classes);
	void TranspileNamespaceDecl(ZNamespace& ns, int accessFlags = 0, bool doBinds = false);
	bool TranspileClassDeclMaster(ZClass& cls, int accessFlags, bool checkDepends = true);
	int  TranspileClassDecl(ZNamespace& ns, int accessFlags = 0);
	int  TranspileMemberDeclVar(ZNamespace& ns, int accessFlags);
	int  TranspileMemberDeclFunc(ZNamespace& ns, int accessFlags, bool doBinds, int vc);
	int  TranspileCABIStruct(ZClass& cls);

	void TranspileDefinitions(ZNamespace& ns, bool vars = true, bool fDecl = true, bool wrap = true);
	void TranspileValDefintons(ZNamespace& ns, bool trail = true);
	
	virtual void WalkNode(Node* node);
		
	void WriteType(ObjectType* tt, bool useauto = false);
	void WriteReturnType(ObjectType* tt, bool useCABI);
	void WriteTypePost(ObjectType *tt, bool array = false);
	
private:
	void Walk(Node* node);
	int WalkChildren(Node* node);
	
	ZNamespace* inNamespace = nullptr;
	ZClass* inClass = nullptr;
	ZFunction* inFunction = nullptr;
	bool firstInNamespace = true;
	bool firstInClass = true;
	int namespaceWrites = 0;
	int classWrites = 0;
	
	int tmpCount = 0;
	int refCount = 0;
	
	void WriteClassAccess(AccessType access);
	
	void NsIntro(ZNamespace& ns);
	void NsOutro(ZNamespace& ns);
	
	void WPNsName(ZEntity& e) {
		cs << e.Namespace().BackName;
		cs << "::";
	}
	
	void WPClsName(ZEntity& e) {
		cs << e.BackName;
	}
	
	void WClsName(ZEntity& e) {
		WPNsName(e);
		WPClsName(e);
	}
	
	void WStorageName(ZClass& cls) {
		if (!cls.CoreSimple)
			WPNsName(cls);
		
		cs << cls.StorageName;
	}
	
	void WFOwnerClsName(ZEntity& e, bool dots) {
		WPNsName(e);
		if (e.InClass) {
			WPClsName(e.Owner());
			if (dots)
				cs << "::";
		}
	}
};

#endif

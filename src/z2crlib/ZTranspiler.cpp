#include <z2crlib/ZTranspiler.h>

extern String opss[];

bool CanAccess(AccessType access, int accessFlags) {
	if (accessFlags == -1)
		return true;
	
	if (access == AccessType::Public && ((accessFlags & 0b1) == 0))
		return false;
	if (access == AccessType::Protected && ((accessFlags & 0b10) == 0))
		return false;
	if (access == AccessType::Private && ((accessFlags & 0b100) == 0))
		return false;
	
	return true;
}

void ZTranspiler::WriteIntro() {
	NL();
	cs << "#include \"cppcode.h\"";
	EL();
	
	NL();
	EL();
	
	//cs << "struct IndexOutOfBoundsException {};\n";
}

void ZTranspiler::NsIntro(ZNamespace& ns) {
	if (CppVersion >= 2017)
		cs << ns.BackName << " {";
	else
		cs << ns.BackNameLegacy;
}

void ZTranspiler::NsOutro(ZNamespace& ns) {
	if (CppVersion >= 2017)
		cs << "}";
	else
		cs << ns.LegacySufix;
}

void ZTranspiler::WriteClassForward() {
	int count = 0;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		for (int j = 0; j < ns.Classes.GetCount(); j++) {
			ZClass& cls = *ns.Classes[j];
			
			if (CheckUse && !cls.InUse)
				continue;
					
			NL();
			cs << "namespace ";
			NsIntro(ns);
			cs << " ";
			cs << "class " << cls.Name << ";";
			cs << " ";
			NsOutro(ns);
			
			EL();
			
			count++;
		}
	}
	
	if (count)
		EL();
}

void ZTranspiler::WriteOutro() {
	if (comp.MainFunction == nullptr)
		return;
	
	NL();
	cs << "int main() {";
	EL();
	
	indent++;
	
	NL();
	cs << "try {";
	EL();
	indent++;
	
	if (comp.MainFunction->InClass == false) {
		NL();
		WPNsName(*comp.MainFunction);
		cs << comp.MainFunction->BackName << "();";
		EL();
	}
	else {
		NL();
		cs << "auto temp = new ";
		WFOwnerClsName(*comp.MainFunction);
		cs << "()";
		ES();
		
		NL();
		cs << "temp->" << comp.MainFunction->BackName << "()";
		ES();
		
		NL();
		cs << "delete temp";
		ES();
	}
	
	indent--;
	
	NL();
	cs << "}";
	EL();
	
	NL();
	cs << "catch (const sys::exception::IndexOutOfBounds& e) {";
	EL();
	
	indent++;
	NL();
	cs << "::zprintf(\"caught IndexOutOfBounds exception\\n\");";
	EL();
	indent--;
	
	NL();
	cs << "}";
	EL();
	NL();
	cs << "catch (const sys::exception::AssertionFailed& e) {";
	EL();
	
	indent++;
	NL();
	cs << "::zprintf(\"caught AssertionFailed exception\\n\");";
	EL();
	indent--;
	
	NL();
	cs << "}";
	EL();
	
	NL();
	cs << "return 0;";
	EL();
	
	indent--;
	
	NL();
	cs << "}";
	EL();
}

void ZTranspiler::TranspileDeclarations(ZNamespace& ns, int accessFlags, bool classes) {
	BeginNamespace(ns);
	TranspileNamespaceDecl(ns, accessFlags, false);
	EndNamespace();
	
	if (!classes)
		return;
	
	// TODO: fix
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		TranspileClassDeclMaster(cls, accessFlags);
	}
	
	/// transpile namespace and class binds
	if (TranspileMemberDeclFunc(ns, accessFlags, true, 0))
		EL();
	
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		
		if (TranspileMemberDeclFunc(cls, accessFlags, true, 0))
			EL();
	}
}

bool ZTranspiler::TranspileClassDeclMaster(ZNamespace& cls, int accessFlags, bool checkDepends) {
	ZClass& acls = (ZClass&)cls;
	
	if (CheckUse && !acls.InUse)
		return false;
	
	if (!CanAccess(cls.Access, accessFlags))
		return false;
	
	if (cls.IsDefined)
		return false;
	
	cls.IsDefined = true;
	
	if (checkDepends)
		for (int j = 0; j < acls.DependsOn.GetCount(); j++)
			TranspileClassDeclMaster(*acls.DependsOn[j], accessFlags);
	
	if (acls.TBase == ass.CSlice && acls.T->TBase == ass.CRaw)
		return false;
	if (acls.TBase == ass.CRaw && acls.T->TBase == ass.CRaw)
		return false;
	
	if (acls.CABICompensation) {
		cs << "struct _";
		WPClsName(acls);
		cs << " {";
		EL();
		
		indent++;
		TranspileCABIStruct(acls);
		indent--;
		
		NL();
		cs << "};";
		EL();
		
		EL();
	}
	
	// TODO: fix
	BeginNamespace(acls._Namespace());
	BeginClass(acls);

	if (TranspileClassDecl(acls, -1) == 0) {
		// empty user classes still need declarations
		if (acls.InUse && !acls.CoreSimple && &acls != ass.CClass && &acls != ass.CRaw && !(acls.FromTemplate/* && acls.TBase != ass.CRaw*/) && !acls.IsStatic)
			NewMember();
	}
	EndClass();
	EndNamespace();
	
	return true;
}

void ZTranspiler::TranspileNamespaceDecl(ZNamespace& ns, int accessFlags, bool doBinds) {
	int vc = TranspileMemberDeclVar(ns, accessFlags);
	int fc = TranspileMemberDeclFunc(ns, accessFlags, doBinds, vc);
}

int ZTranspiler::TranspileClassDecl(ZNamespace& ns, int accessFlags) {
	if (CheckUse && !ns.InUse)
		return 0;
	
	if (ns.Variables.GetCount() + ns.Methods.GetCount() == 0)
		return 0;
	
	int vc = TranspileMemberDeclVar(ns, 0b1);
	vc += TranspileMemberDeclVar(ns, 0b10);
	vc += TranspileMemberDeclVar(ns, 0b100);
	
	int fc = TranspileMemberDeclFunc(ns, 0b1, false, vc);
	fc += TranspileMemberDeclFunc(ns, 0b10, false, vc);
	fc += TranspileMemberDeclFunc(ns, 0b100, false, vc);
	
	return vc + fc;
}

int ZTranspiler::TranspileCABIStruct(ZClass& cls) {
	//if (CheckUse && !ns.InUse)
	//	return 0;
	
	if (cls.Variables.GetCount() == 0)
		return 0;
	
	int vc = TranspileMemberDeclVar(cls, 0b1);
	vc += TranspileMemberDeclVar(cls, 0b10);
	vc += TranspileMemberDeclVar(cls, 0b100);
	
	return vc;
}

void ZTranspiler::WriteType(ObjectType* tt, bool useauto) {
	if (tt->Class == ass.CPtr) {
		WriteType(tt->Next);
		cs << "*";
	}
	else if (tt->Class->FromTemplate && tt->Class->TBase == ass.CRaw) {
		WriteType(&tt->Class->T->Tt);
	}
	else if (tt->Class->CoreSimple)
		WStorageName(*tt->Class);//cs << tt->Class->BackName;
	else {
		if (useauto)
			cs << "auto";
		else
			WStorageName(*tt->Class);
	}
}

void ZTranspiler::WriteReturnType(ObjectType *tt, bool useCABI) {
	if (useCABI && tt->Class->CABICompensation) {
		cs << "_";
		WPClsName(*tt->Class);
	}
	else
		WriteType(tt, false);
}

void ZTranspiler::WriteTypePost(ObjectType* tt, bool array) {
	if (tt->Class->FromTemplate && tt->Class->TBase == ass.CRaw) {
		cs << '[' << tt->Param;
		if (array) {
			cs << " * sizeof(";
			WriteType(tt);
			cs <<")";
		}
		cs << ']';
		//ASSERT(tt->Next);
		if (tt->Next)
			WriteTypePost(tt->Next);
	}
}

int ZTranspiler::TranspileMemberDeclVar(ZNamespace& ns, int accessFlags) {
	if (CheckUse && !ns.InUse)
		return 0;
	
	bool first = true;
	
	int count = 0;
	
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		
		if (!CanAccess(v.Access, accessFlags))
			continue;
		
		if (CheckUse && v.IsStatic && !v.InUse)
			continue;
		
		// TODO: fix
		if (&ns == ass.CString)
			if (v.Name == "text")
				continue;
			
		//if (v.I.Tt.Class)
		//	continue;
		
		ASSERT(v.I.Tt.Class);
		//ASSERT(v.Value);
		
		NewMember();
				
		count++;
		
		if (first) {
			if (v.InClass == true) {
				WriteClassAccess(v.Access);
			}
			first = false;
		}
		
		NL();
		
		if (v.InClass) {
			if (v.IsStatic && v.IsConst)
				cs << "static const ";
			else if (v.IsStatic)
				cs << "static ";
		}
		else {
			cs << "extern ";
			if (v.IsConst)
				cs << "const ";
		}

//		if (v.Name == "TileVariant")
//			v.Name == "TileVariant"

		WriteType(&v.I.Tt);
		cs << " " << v.Name;
		WriteTypePost(&v.I.Tt);
		
		if (v.I.Tt.Class->FromTemplate && v.I.Tt.Class->TBase == ass.CRaw) {
		}
		else if (v.Value) {
			if (v.InClass && !v.IsStatic) {
				cs << " = ";
				Walk(v.Value);
			}
		}
		
		ES();
	}
	
	return count;
}

int ZTranspiler::TranspileMemberDeclFunc(ZNamespace& ns, int accessFlags, bool doBinds, int vc) {
	if (CheckUse && !ns.InUse)
		return 0;
	
	bool first = true;
	
	int count = 0;
	
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		for (int j = 0; j < ns.Methods[i].Functions.GetCount(); j++) {
			ZFunction& f = *ns.Methods[i].Functions[j];
			
			if (CheckUse && !f.InUse)
				continue;
			
			if (f.IsGenerated)
				continue;
			
			if (!CanAccess(f.Access, accessFlags))
				continue;
						
			bool bindc = f.Trait.Flags & ZTrait::BINDC;
			
			if (doBinds == false && bindc == true)
				continue;
			if (doBinds == true && bindc == false)
				continue;
						
			if (doBinds == true && (f.Trait.Flags & ZTrait::INTRINSIC))
				continue;
			
			NewMember();
			count++;
			
			if (first) {
				if (f.InClass == false) {
					if (vc)
						EL();
				}
				else if (doBinds == false) {
					WriteClassAccess(f.Access);
				}
				first = false;
			}
			
			NL();
			if (bindc) {
				if (f.Trait.Flags & ZTrait::BINDC) {
					int ii = ass.Binds.Find(f.Name);
					if (ii != -1) {
						cs << ass.Binds[ii] << "\n";
						f.WroteDeclaration = true;
						continue;
					}
				}
				cs << "extern \"C\" ";
			}
			else if (f.Trait.Flags & ZTrait::BINDCPP)
				cs << "extern ";
			if (f.Trait.Flags & ZTrait::DLLIMPORT)
				cs << "__declspec(dllimport) ";
			
			if (WriteFunctionDef(f) == false)
				ES();
			
			f.WroteDeclaration = true;
		}
	}
	
	return count;
}

void ZTranspiler::TranspileDefinitions(ZNamespace& ns, bool vars, bool fDecl, bool wrap) {
	if (vars) {
		BeginNamespace(ns);
		TranspileValDefintons(ns);
		EndNamespace();
		
		for (int i = 0; i < ns.Classes.GetCount(); i++) {
			ZClass& cls = *ns.Classes[i];
			BeginNamespace(cls._Namespace());
			TranspileValDefintons(cls);
			EndNamespace();
		}
	}
	
	//BeginNamespace(ns);
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			if (f.IsExternBind())
				continue;
			
			if (!f.WroteDeclaration)
				continue;
			
			NewMember();
			NL();
			
			if (fDecl && (CheckUse == false || f.InUse) && !f.IsSimpleGetter) {
				WriteFunctionDecl(f);
				WriteFunctionBody(f, wrap);
			}
		}
	}
	//EndNamespace();
	
	for (int i = 0; i < ns.Classes.GetCount(); i++) {
		ZClass& cls = *ns.Classes[i];
		
		BeginNamespace(ns);
		BeginClass(cls);
		for (int j = 0; j < cls.Methods.GetCount(); j++) {
			ZMethodBundle& d = cls.Methods[j];
			
			for (int k = 0; k < d.Functions.GetCount(); k++) {
				ZFunction& f = *d.Functions[k];
				
				if (f.IsExternBind())
					continue;
				
				if (!f.WroteDeclaration)
					continue;
				
				NL();
				
				if (fDecl && (CheckUse == false || f.InUse) && f.IsGenerated == false) {
					WriteFunctionDecl(f);
					WriteFunctionBody(f, wrap);
				}
			}
		}
		EndClass();
		EndNamespace();
	}
}

void ZTranspiler::TranspileValDefintons(ZNamespace& ns, bool trail) {
	if (CheckUse && !ns.InUse)
		return;
	
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		auto v = *ns.Variables[i];
		ASSERT(v.I.Tt.Class);
		
		if (v.InClass && !v.IsStatic)
			continue;
		
		if (CheckUse && v.IsStatic && !v.InUse)
			continue;
		
		NewMember();
		
		NL();
		
		if (v.IsConst)
			cs << "const ";
		
		WriteType(&v.I.Tt);

		cs << " ";
		if (!v.InClass)
			cs << v.Name;
		else {
			WriteClassName(v.Class());
			cs << "::" << v.Name;
		}
		WriteTypePost(&v.I.Tt);
		
		if (v.I.Tt.Class->FromTemplate && v.I.Tt.Class->TBase == ass.CRaw) {
		}
		else if (v.Value){
			cs << " = ";
			Walk(v.Value);
		}
		ES();
	}
}

bool ZTranspiler::WriteFunctionDef(ZFunction& f) {
	if (f.IsSimpleGetter && f.IsGetter) {
		WriteType(&f.Return.Tt);
	
		cs << " " << f.BackName;
		WriteFunctionParams(f);
		
		cs << " const {";
		EL();
		
		indent++;
		NL();
		cs << "return _" << ToLower(f.Name);
		ES();
		indent--;
		
		NL();
		cs << "}";
		EL();
		
		NL();
		WriteType(&f.Return.Tt);
	
		cs << " _" << ToLower(f.Name) << " = 0";
		
		return false;
	}
	
	if (f.IsSimpleGetter && f.IsGetter == false) {
		WriteType(&ass.CVoid->Tt);
	
		cs << " " << f.BackName;
		WriteFunctionParams(f);
		
		cs << " {";
		EL();
		
		indent++;
		NL();
		cs << "_" << ToLower(f.Name) << " = " << "__value";
		ES();
		indent--;
		
		NL();
		cs << "}";
		EL();
		
		return false;
	}
	
	if (f.IsConstructor == 1 && f.Class().TBase == ass.CRaw) {
		/*if (f.InClass && f.Class().CoreSimple)
			cs << "static " << "_";
		else
			WPClsName(f.Class());*/
		cs << "void ";
		cs << "Init";
		WriteFunctionParams(f);

		return false;
	}
	else if (f.IsConstructor == 1) {
		if (f.InClass && f.Class().CoreSimple)
			cs << "static " << "_";
		else
			WPClsName(f.Class());
		WriteFunctionParams(f);
		return false;
	}
	else if (f.IsConstructor == 2) {
		cs << "static ";
		WStorageName(f.Class());
		cs << " " << f.Name;
		WriteFunctionParams(f);
		return false;
	}
	else if (f.IsDestructor) {
		cs << "~" << f.Owner().BackName;
		WriteFunctionParams(f);
		return false;
	}
	
	if (f.Trait.Flags & ZTrait::BINDC)
		;
	else if (f.InClass == false && f.Access == AccessType::Private)
		cs << "static ";
	else if (f.InClass == true && f.IsStatic)
		cs << "static ";
	else if (f.InClass && f.Class().CoreSimple)
		cs << "static ";
	//else if (f.InClass && f.Class().TBase == ass.CRaw)
	//	cs << "static ";
	
	WriteReturnType(&f.Return.Tt, f.Trait.Flags & ZTrait::BINDC);
	
	cs << " " << f.BackName;
	WriteFunctionParams(f);
	
	if (f.InClass && f.Class().CoreSimple)
		;
	else if (f.InClass == true && f.IsFunction && !f.IsStatic)
		cs << " const";
	
	return false;
}

void ZTranspiler::WriteFunctionDecl(ZFunction& f) {
	if (f.IsConstructor == 1 && f.Class().TBase == ass.CRaw) {
		cs << "void ";
		WPNsName(f);
		WPClsName(f.Class());
		cs << "::";
		cs << "Init";
		
		WriteFunctionParams(f);
		
		return;
	}
	else if (f.IsConstructor == 1) {
		ASSERT(f.Owner().IsClass);
		
		// ns+cls::f
		WPNsName(f);
		WPClsName(f.Class());
		cs << "::";
		cs << f.BackName;
		
		WriteFunctionParams(f);
		
		return;
	}
	else if (f.IsConstructor == 2) {
		ASSERT(f.Owner().IsClass);
		
		// ret
		WStorageName(f.Class());
		cs << " ";
		
		// ns+cls::f
		WPNsName(f);
		WPClsName(f.Class());
		cs << "::";
		cs << f.BackName;
		
		WriteFunctionParams(f);
		
		return;
	}
	else if (f.IsDestructor) {
		cs << f._Namespace().BackName << "::";
		cs << f.Owner().BackName << "::" << "~" << f.Owner().BackName;
		WriteFunctionParams(f);
		return;
	}
	
	// ret
	WriteReturnType(&f.Return.Tt, f.Trait.Flags & ZTrait::BINDC);
	cs << " ";
	
	// ns
	WPNsName(f);
	// +cls?
	if (f.InClass) {
		WPClsName(f.Class());
		cs << "::";
	}
	cs << f.BackName;
	WriteFunctionParams(f);
	
	if (f.InClass && f.Class().CoreSimple)
		;
	else if (f.InClass && f.IsFunction && !f.IsStatic)
		cs << " const";
}

void ZTranspiler::WriteFunctionParams(ZFunction& f) {
	cs << "(";
	
	//if (f.InClass && f.Class() ==
	if (f.IsConstructor == 0 && f.InClass) {
		 ZClass& pcls = f.Class();
		 if (pcls.CoreSimple) {
		     cs << pcls.StorageName << " _this";
		 
			 if (f.Params.GetCount())
			     cs << ", ";
		 }
	}
	
	
	for (int i = 0; i < f.Params.GetCount(); i++) {
		ZVariable& var = f.Params[i];
		
		if (i > 0)
			cs << ", ";
		
		ZClass& parCls = *var.I.Tt.Class->ParamType;
		
		if (var.I.Tt.Class == ass.CPtr)
			WriteType(&var.I.Tt);
		else if (parCls.CoreSimple)
			WStorageName(parCls);
			//cs << parCls.BackName;
		else {
			if (f.Trait.Flags & ZTrait::BINDC)
				cs << parCls._Namespace().BackName << "::" << parCls.BackName;
			else
				cs << "const "<< parCls._Namespace().BackName << "::" << parCls.BackName << "&";
		}
		cs << " " << var.Name;
	}
	
	cs << ")";
}

void ZTranspiler::WriteFunctionBody(ZFunction& f, bool wrap) {
	if (CheckUse && !f.InUse)
		return;
	
	inFunction = &f;
	tmpCount = 0;
	refCount = 0;
	
	if (wrap) {
		cs << " {";
		EL();
	}
		
	indent++;
	
	bool special = f.IsConstructor == 2 || (f.IsConstructor == 1 && f.InClass && f.Class().CoreSimple);
	if (special) {
		NL();
		if (f.InClass && f.Class().CoreSimple) {
			WStorageName(f.Class());//cs << f.Owner().BackName << " ";
			cs << " ";
		}
		else {
			cs << f._Namespace().BackName << "::";
			cs << f.Owner().Name << " ";
		}
		
		cs << "_this";
		ES();
		
		NL();
		EL();
	}
	
	int count = WalkChildren(&f.Nodes);
	
	if (special) {
		if (count) {
			NL();
			EL();
		}
		
		NL();
		cs << "return _this";
		ES();
	}
	
	indent--;
	
	if (wrap) {
		NL();
		cs << "}";
		EL();
		
		NL();
		EL();
	}
	
	inFunction = nullptr;
}

void ZTranspiler::WriteClassAccess(AccessType access) {
	if (access == AccessType::Public) {
		indent--;
		NL();
		cs << "public:";
		EL();
		indent++;
	}
	if (access == AccessType::Protected)  {
		indent--;
		NL();
		cs << "protected:";
		EL();
		indent++;
	}
	if (access == AccessType::Private) {
		indent--;
		NL();
		cs << "private:";
		EL();
		indent++;
	}
}

void ZTranspiler::WalkNode(Node* node) {
	Walk(node);
}

void ZTranspiler::Walk(Node* node) {
	ASSERT_(node, "Null node");
	
	if (node->NT == NodeType::Const)
		Proc(*(ConstNode*)node, cs);
	else if (node->NT == NodeType::BinaryOp)
		Proc(*(OpNode*)node);
	else if (node->NT == NodeType::UnaryOp)
		Proc(*(UnaryOpNode*)node);
	else if (node->NT == NodeType::Memory)
		Proc(*(MemNode*)node);
	else if (node->NT == NodeType::Cast)
		Proc(*(CastNode*)node);
	else if (node->NT == NodeType::Temporary)
		Proc(*(TempNode*)node);
	else if (node->NT == NodeType::Def)
		Proc(*(DefNode*)node);
	else if (node->NT == NodeType::List)
		Proc(*(ListNode*)node);
	else if (node->NT == NodeType::Ptr)
		Proc(*(PtrNode*)node);
	else if (node->NT == NodeType::Index)
		Proc(*(IndexNode*)node);
	/*else if (node->NT == NodeType::SizeOf)
		Proc((SizeOfNode*)node);
	else if (node->NT == NodeType::Property)
		Proc((PropertyNode*)node);
	else if (node->NT == NodeType::Deref)
		Proc((DerefNode*)node);*/
	else if (node->NT == NodeType::Intrinsic)
		Proc(*(IntrinsicNode*)node);
	else if (node->NT == NodeType::Return)
		Proc(*(ReturnNode*)node);
	else if (node->NT == NodeType::Local)
		Proc(*(LocalNode*)node);
	/*else if (node->NT == NodeType::Alloc)
		Proc((AllocNode*)node);
	else if (node->NT == NodeType::Array)
		Proc((RawArrayNode*)node);
	else if (node->NT == NodeType::Using)
		Proc((UsingNode*)node);*/
	else if (node->NT == NodeType::Block)
		Proc(*(BlockNode*)node);
	else if (node->NT == NodeType::If)
		Proc(*(IfNode*)node);
	else if (node->NT == NodeType::While)
		Proc(*(WhileNode*)node);
	else if (node->NT == NodeType::DoWhile)
		Proc(*(DoWhileNode*)node);
	else if (node->NT == NodeType::ForLoop)
		Proc(*(ForLoopNode*)node);
	else if (node->NT == NodeType::LoopControl)
		Proc(*(LoopControlNode*)node);
	/*else if (node->NT == NodeType::Construct)
		Proc((ConstructNode*)node);*/
	else if (node->NT == NodeType::Destruct)
		Proc(*(DestructNode*)node);
	else if (node->NT == NodeType::Throw)
		Proc(*(ThrowNode*)node);
	else if (node->NT == NodeType::PlacamentNew)
		Proc(*(PlacementNewNode*)node);
	else
		ASSERT_(0, "Invalid node");
}

int ZTranspiler::WalkChildren(Node* node) {
	int count = 0;
	Node* child = node->First;
	
	while (child) {
		count++;
		NL();
		WalkNode(child);
		if (child->NT != NodeType::Block && child->NT != NodeType::If && child->NT != NodeType::While && child->NT != NodeType::ForLoop)
			ES();
		
		child = child->Next;
	}
	
	return count;
}

void ZTranspiler::Proc(ConstNode& node, Stream& stream) {
	if (node.Tt.Class == ass.CQWord) {
		if (IsNull(node.IntVal))
			stream << "9223372036854775808ull";
		else
			stream << Format64((uint64)node.IntVal) << "ull";
	}
	else if (node.Tt.Class == ass.CLong) {
		if (IsNull(node.IntVal))
			stream << "-9223372036854775808ll";
		else
			stream << IntStr64(node.IntVal) << "ll";
	}
	else if (node.Tt.Class == ass.CDWord || node.Tt.Class == ass.CWord || node.Tt.Class == ass.CByte) {
		if (node.Base == 16)
			stream << "0x" << Format64Hex((uint32)node.IntVal) << 'u';
		else
			stream << Format64((uint32)node.IntVal) << 'u';
	}
	else if (node.Tt.Class == ass.CPtrSize) {
		if (node.Base == 16)
			stream << "0x" << Format64Hex(node.IntVal) << "u";
		else
			stream << Format64(node.IntVal) << "u";
	}
	else if (node.Tt.Class == ass.CInt || node.Tt.Class == ass.CSmall || node.Tt.Class == ass.CShort) {
		if (node.Base == 16) {
			if (node.IntVal == -2147483648ll)
				stream << "(int32)" << "0x" << ToUpper(Format64Hex(node.IntVal)) << "ll";
			else
				stream << "0x" << ToUpper(Format64Hex(node.IntVal));
		} else {
			if (node.IntVal == -2147483648ll)
				stream << "(int32)" << IntStr64(node.IntVal) << "ll";
			else
				stream << IntStr64(node.IntVal);
		}
	}
	else if (node.Tt.Class == ass.CFloat) {
		if (node.DblVal != node.DblVal)
			stream << "((float)(1e+300 * 1e+300) * 0.0f)";
		else if (node.DblVal == (float)(1e+300 * 1e+300))
			stream << "((float)(1e+300 * 1e+300))";
		else if (node.DblVal == -(float)(1e+300 * 1e+300))
			stream << "(-(float)(1e+300 * 1e+300))";
		else {
			if (IsNull(node.DblVal))
				stream << "-1.79769e+308" << "\f";
			else {
				String z = FormatDouble(node.DblVal);
				stream << z;
				if (z.Find('.') == -1 && z.Find('e') == -1)
				    stream << ".0";
				stream << 'f';
			}
		}
	}
	else if (node.Tt.Class == ass.CDouble) {
		if (node.DblVal != node.DblVal)
			stream << "((float)(1e+300 * 1e+300) * 0.0f)";
		else if (node.DblVal == (float)(1e+300 * 1e+300))
			stream << "((float)(1e+300 * 1e+300))";
		else if (node.DblVal == -(float)(1e+300 * 1e+300))
			stream << "(-(float)(1e+300 * 1e+300))";
		else {
			if (IsNull(node.DblVal))
				stream << "-1.79769e+308";
			else {
				String s = FormatDouble(node.DblVal);
				stream << s;
				if (s.Find('.') == -1 && s.Find('e') == -1)
					stream << ".0";
			}
		}
	}
	else if (node.Tt.Class == ass.CBool) {
		if (node.IntVal == 0)
			stream << "false";
		else
			stream << "true";
	}
	else if (node.Tt.Class == ass.CChar) {
		if (node.IntVal >= 127) {
			if (node.Base == 16)
				stream << "0x" << Format64Hex(node.IntVal);
			else
				stream << node.IntVal;
			return;
		}
		
		stream << '\'';
		
		if (node.IntVal == '\t')
			stream << "\\t";
		else if (node.IntVal == '\n')
			stream << "\\n";
		else if (node.IntVal == '\r')
			stream << "\\r";
		else if (node.IntVal == '\a')
			stream << "\\a";
		else if (node.IntVal == '\b')
			stream << "\\b";
		else if (node.IntVal == '\f')
			stream << "\\f";
		else if (node.IntVal == '\v')
			stream << "\\v";
		else if (node.IntVal == '\'')
			stream << "\\\'";
		else if (node.IntVal == '\\')
			stream << "\\\\";
		else if (node.IntVal == '\0')
			stream << "\\0";
		else
			stream << (char)node.IntVal;
		
		stream << '\'';
	}
	else if (node.Tt.Class == ass.CClass) {
		ZClass& ccc = ass.Classes[(int)node.IntVal];
		//stream << ass.CCls->NamespaceQual;
		stream << "Class(" << ccc.RTTIIndex << ")";
	}
	else if (node.Tt.Class == ass.CNull)
		stream << "nullptr";
	else if (node.Tt.Class == ass.CString) {
		//stream << "S_[" << (int)node.IntVal << ']';
		stream << AsCString(ass.StringConsts[(int)node.IntVal]);
	}
}

void ZTranspiler::Proc(OpNode& node) {
	Node *l = node.OpA;
	Node *r = node.OpB;
	ASSERT(l);
	ASSERT(r);
	
	if (node.Op == OpNode::opTernary) {
		cs << "((";
		Walk(l);
		cs << ") ? ";
		Walk(r);
		cs << " : ";
		ASSERT(node.OpC);
		Walk(node.OpC);
		cs << ")";
	}
	else if (node.Op == OpNode::opMod && (ass.IsFloat(l->Tt) || ass.IsFloat(l->Tt))) {
		if (l->Tt.Class != ass.CDouble && r->Tt.Class != ass.CDouble)
			cs << "(float)";
		cs << "fmod(";
		Walk(l);
		cs << ", ";
		Walk(r);
		cs << ')';
	}
	else {
		if (node.Op == OpNode::opAssign && l->Chain && l->Chain->PropCount)
			ProcLeftSet(l, r, node.ExtraOp, node.ExtraNode);
		else {
			Walk(l);
			
			if (l->Tt.Class->TBase == ass.CRaw) {
				cs << ".Copy(";
				Walk(r);
				cs << ")";
			}
			else {
				cs << ' ';
				if (node.ExtraOp != OpNode::Type::opNotSet)
					cs << opss[node.ExtraOp];
				cs << opss[node.Op];
				cs << ' ';
				Walk(r);
			}
		}

	}
}

void ZTranspiler::ProcLeftSet(String& cs, Node* n) {
	if (cs.GetCount())
		cs << ".";
	
	if (n->NT == NodeType::Memory) {
		auto mn = (MemNode*)n;
		
		if (mn->IsThis)
			cs << "(*this)";
		else
			cs << mn->Mem->BackName;
	}
	else if (n->NT == NodeType::Def) {
		auto dn = (DefNode*)n;
		cs << dn->Function->BackName;
	}
	/*else if (n->NT == NodeType::Index) {
		auto in = (IndexNode*)n;
		ProcLeftSet(cs, in->Object);
		cs << "[";
		Walk(in->Index);
		cs << "]";
	}*/
	else
		ASSERT(0);
}

void ZTranspiler::ProcLeftSet(Node* l, Node* r, OpNode::Type extraOp, Node* extra) {
	Node* child = l->Chain->First;
	
	bool state = 0;
	DefNode* prop = nullptr;
	int index = 0;
	
	String seq;
	Vector<DefNode*> props;
	
	while (child) {
		if (child->NT == NodeType::Def) {
			DefNode *p = (DefNode*)child;
			if (child->Next == nullptr) {
				if (p->Function->IsProperty) {
					if (prop == nullptr) {
						if (seq.GetCount() == 0 && p->Object) {
							if (p->Object->NT == NodeType::Memory) {
								ProcLeftSet(seq, p->Object);
							}
							else if (p->Object->NT == NodeType::Index) {
								auto as = (IndexNode*)p->Object;
								seq << "[";
								Walk(as->Index);
								seq << "]";
							}
							else
								ASSERT(0);
						}

						ProcLeftSet(seq, p);
						
						cs << seq;
						cs << "(";
						Walk(r);
						cs << ")";
					}
					else {
						NL();
						cs << "_tmp" << tmpCount << "." << p->Function->BackName << "(";
						Walk(r);
						cs << ")";
						ES();
						
						NL();
	
						cs << "_ref" << refCount << ".";
						cs << prop->Function->BackName;
						cs << "(" <<  "_tmp" << tmpCount << ")";
					}
					
					return;
				}
			}
			else {
				if (prop)
					NL();
			
				prop = p;
				props << p;
				
				
				WriteType(&p->Object->Tt);
				refCount++;
				cs << "& " << "_ref" << refCount;
				cs << " = ";
				//Walk(child->Prev);
				if (seq.GetCount())
					cs << seq;
				else
					cs << "_tmp" << tmpCount;
				seq = "";
				ES();
				
				NL();
				
				WriteType(&p->Tt);
				tmpCount++;
				cs << " " << "_tmp" << tmpCount;
				cs << " = ";
				cs << "_ref" << refCount << ".";
				cs << p->Function->BackName << "()";
					
				ES();

				state = 1;
			}
		}
		else if (child->NT == NodeType::Memory) {
			ProcLeftSet(seq, child);
		}
		else if (child->NT == NodeType::Index) {
			auto in = (IndexNode*)child;
			//ProcLeftSet(seq, in->Object);
			
			StringStream ss;
			ZTranspiler z(comp, ss);
			seq << "[";
			z.Walk(in->Index);
			seq << ss.GetResult();
			seq << "]";
		}
		else
			ASSERT(0);
			
		child = child->Next;
		index++;
	}
		
	int startTmp = tmpCount;
	int startRef = refCount;
	
	if (state == 1) {
		for (int i = props.GetCount() - 1; i >= 0; i--) {
			NL();
			
			if (seq.GetCount()) {
				cs << "_tmp" << startTmp << ".";
				cs << seq;
				cs << " ";
				if (extraOp != OpNode::Type::opNotSet)
					cs << opss[extraOp];
				cs << "= ";
				Walk(r);
				ES();
				
				NL();
				seq = "";
				extraOp = OpNode::Type::opNotSet;
			}
			
			cs << "_ref" << startRef << ".";
			cs << props[i]->Function->BackName;
			cs << "(";
			
			if (i == props.GetCount() - 1 && extraOp != OpNode::Type::opNotSet) {
				if (extraOp != OpNode::Type::opNotSet) {
					if (extra && extra->NT == NodeType::Def) {
						cs << "_tmp" << startTmp << ".";
						
						DefNode* def = (DefNode*)extra;
						ZFunction& f = *def->Function;
	
						if (f.Trait.Flags & ZTrait::BINDC)
							cs << "::";
						else if (f.IsStatic) {
							cs << f._Namespace().BackName << "::";
							if (f.InClass)
								cs << f.Owner().BackName << "::";
						}
						
						cs << f.BackName;
						cs << '(';
						Walk(r);
						cs << ")";
					}
					else {
						cs << "_tmp" << startTmp;
						cs << " ";
						cs << opss[extraOp];
					}
				}
				else {
					cs << "_tmp" << startTmp;
					cs << " ";
					Walk(r);
				}
			}
			else {
				cs << "_tmp" << startTmp;
			}
				
			cs << ")";
			
			if (i > 0)
				ES();
			
			startTmp--;
			startRef--;
		}
	}
}
	
void ZTranspiler::Proc(UnaryOpNode& node) {
	Node *f = node.OpA;
	ASSERT(f);
	
	if (node.Op == OpNode::opInc) {
		if (node.Prefix) {
			cs << "++";
			Walk(f);
		}
		else {
			Walk(f);
			cs << "++";
		}
	}
	else if (node.Op == OpNode::opDec) {
		if (node.Prefix) {
			cs << "--";
			Walk(f);
		}
		else {
			Walk(f);
			cs << "--";
		}
	}
	else {
		if (node.Op == OpNode::opNot && ass.IsFloat(node.Tt)) {
			cs << "::bitnot(";
			Walk(f);
			cs << ')';
		}
		else {
			cs << opss[node.Op];
			if (f->NT != NodeType::List) {
				cs << '(';
				Walk(f);
				cs << ')';
			}
			else
				Walk(f);
		}
	}
}

void ZTranspiler::Proc(DefNode& node) {
	ZFunction& f = *node.Function;
	
	if (f.Trait.Flags & ZTrait::BINDC)
		cs << "::";
	else if (f.IsStatic) {
		cs << f._Namespace().BackName << "::";
		if (f.InClass)
			cs << f.Owner().BackName << "::";
	}
	else if (f.InClass && f.Class().CoreSimple) {
		cs << f._Namespace().BackName << "::";
		WriteClassName(*node.Tt.Class);
		cs << "::";
		cs << f.BackName;
		cs << "(";
		Walk(node.Object);
		cs << ")";
		return;
	}
	else {
		//ASSERT(node.Object);
		if (node.Object) {
			/*if (node.Object->Tt.Class->TBase == ass.CRaw) {
				if (f.IsProperty && f.Name == "Length") {
					cs << node.Object->Tt.Param;
					return;
				}
				else {
					cs << ass.CSlice->Owner().BackName;
					cs << "::";
					cs << "CArray_";
					cs << node.Object->Tt.Class->T->Name;
					cs << "(";
					Walk(node.Object);
					cs << ", ";
					cs << node.Object->Tt.Param;
					cs << ").";
				}
			}
			else {*/
				Walk(node.Object);
				cs << ".";
			//}
		}
	}
	
	cs << f.BackName;
	cs << '(';
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node* p = node.Params[i];

		Walk(p);
		
		if (i < count -1)
			cs << ", ";
	}
	cs << ')';
}

void ZTranspiler::Proc(MemNode& node) {
	if (node.IsThis) {
		if (node.Tt.Class->CoreSimple)
			cs << "_this";
		else if (inFunction && inFunction->IsConstructor == 2)
			cs << "_this";
		else
			cs << "(*this)";
		return;
	}
	
	ASSERT(node.Mem);
	
	if (node.IsLocal == false && node.IsParam == false && node.Mem->InClass == false) {
		if (node.Mem->InClass) {
			//if (&node.Mem->Namespace() != inNamespace)
				cs << node.Mem->_Namespace().BackName << "::";
			//cs << node.Mem->Owner().BackName << "::";
			WriteClassName(node.Mem->Class());
		}
		else {
			//if (&node.Mem->Namespace() != inNamespace)
				cs << node.Mem->Owner().BackName << "::";
		}
		cs << node.Mem->BackName;
	}
	else if (node.Mem->InClass == true && node.Mem->IsStatic) {
		//if (&node.Mem->Namespace() != inNamespace)
			cs << node.Mem->_Namespace().BackName << "::";
		//cs << node.Mem->Owner().BackName << "::";
		WriteClassName(node.Mem->Class());
		cs << "::";
		cs << node.Mem->BackName;
	}
	else if (node.Object) {
		Walk(node.Object);
		cs << ".";
		cs << node.Mem->BackName;
	}
	else {
		if (inFunction && inFunction->IsConstructor == 2 && node.Mem->InClass)
			cs << "_this.";
		cs << node.Mem->BackName;
	}
}

void ZTranspiler::Proc(BlockNode& node) {
	//NL();
	cs << "{";
	EL();
	
	indent++;
	WalkChildren(&node.Nodes);
	indent--;
	
	NL();
	cs << '}';
	
	if (node.EndBlockStat)
		EL();
}

void ZTranspiler::Proc(IfNode& node) {
	ASSERT(node.Cond);
	
	//NL();
	cs << "if (";
	Walk(node.Cond);
	cs << ")";
	// dangling end statement
	
	if (node.TrueBranch) {
		if (node.TrueBranch->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.TrueBranch);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.TrueBranch);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
	
	if (node.FalseBranch) {
		NL();
		cs << "else";
		// dangling end statement

		if (node.FalseBranch->NT != NodeType::Block && node.FalseBranch->NT != NodeType::If) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.FalseBranch);
			if (node.FalseBranch->NT == NodeType::If)
				EL();
			else
				ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.FalseBranch);
		}
	}
}

void ZTranspiler::Proc(WhileNode& node) {
	ASSERT(node.Cond);
	
	cs << "while (";
	Walk(node.Cond);
	cs << ")";
	// dangling end statement
	
	if (node.Body) {
		if (node.Body->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.Body);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.Body);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
}

void ZTranspiler::Proc(DoWhileNode& node) {
	ASSERT(node.Cond);

	cs << "do ";
	if (node.Body->NT == NodeType::Block)
		((BlockNode*)node.Body)->EndBlockStat = false;
	WalkNode(node.Body);
	
	cs << " while (";
	Walk(node.Cond);
	cs << ")";
}

void ZTranspiler::Proc(ForLoopNode& node) {
	ASSERT(node.Cond);
	
	cs << "for (";
	if (node.Init)
		Walk(node.Init);
	cs << "; ";
	Walk(node.Cond);
	cs << "; ";
	if (node.Iter)
		Walk(node.Iter);
	cs << ")";
	
	if (node.Body) {
		if (node.Body->NT != NodeType::Block) {
			EL();
			
			indent++;
			NL();
			WalkNode(node.Body);
			ES();
			indent--;
		}
		else {
			cs << " ";
			WalkNode(node.Body);
		}
	}
	else {
		EL();
		
		indent++;
		NL();
		ES();
		indent--;
	}
}

void ZTranspiler::Proc(LocalNode& node) {
	ASSERT(node.Var);
	
	if (node.Tt.Class->TBase == ass.CRaw) {
		if (node.Tt.Class->T->CoreSimple) {
			WriteType(&node.Var->I.Tt, true);
			cs << " __" << node.Var->Name;
			WriteTypePost(&node.Var->I.Tt);
		}
		else {
			cs << "alignas(";
			WriteType(&node.Var->I.Tt);
			cs << ") ";
			cs << "uint8";
			cs << " __" << node.Var->Name;
			WriteTypePost(&node.Var->I.Tt, true);
		}
		ES();
		
		NL();
		cs << ass.CSlice->Owner().BackName;
		cs << "::";
		WriteClassName(*node.Tt.Class);
		cs << " " << node.Var->Name;
		cs << "(";
		if (!node.Tt.Class->T->CoreSimple) {
			cs << "(";
			WriteType(&node.Var->I.Tt);
			cs << "*)";
		}
		//
		cs << "__" << node.Var->Name;
		cs << ", ";
		cs << node.Tt.Param;
		cs << ")";
		
		if (node.Var->Value) {
			ES();
			
			NL();
			if (node.Var->Value->NT == NodeType::Temporary) {
				TempNode& tmp = (TempNode&)*node.Var->Value;
				cs << node.Var->Name;
				cs << ".Init(";
				for (int i = 0; i < tmp.Params.GetCount(); i++)
					WalkNode(tmp.Params[i]);
				cs << ")";
			}
			else {
				cs << node.Var->Name << ".Copy(";
				WalkNode(node.Var->Value);
				cs << ")";
			}
		}
		
		return;
	}
	
	if (node.Var->IsConst)
		cs << "const ";
	
	WriteType(&node.Var->I.Tt, true);
	cs << " " << node.Var->Name;
	WriteTypePost(&node.Var->I.Tt);
	
	if (node.Var->Value) {
		if (node.Var->I.Tt.Class->FromTemplate &&node.Var->I.Tt.Class->TBase == ass.CRaw) {
		}
		else {
			cs << " = ";
			Walk(node.Var->Value);
		}
	}
}

void ZTranspiler::Proc(CastNode& node) {
	ASSERT(node.Object);
	
	cs << '(';
	WStorageName(*node.Tt.Class);
	cs << ')';
	
	cs << '(';
	Walk(node.Object);
	cs << ')';
}

void ZTranspiler::Proc(ReturnNode& node) {
	cs << "return";
	
	if (node.Value) {
		cs << " ";
		Walk(node.Value);
	}
}

void ZTranspiler::Proc(IntrinsicNode& node) {
	if (node.Operation == 0) {
		cs << "(";
		for (int i = 0; i < node.Value.GetCount(); i++) {
			if (i > 0)
				cs << ", ";
			
			if (node.Value[i]->NT != NodeType::Intrinsic || ((IntrinsicNode*)node.Value[i])->Operation != 2) {
				cs << "::zprintf";
				if (node.Value[i]->Tt.Class == ass.CChar)
					cs << "c";
				else if (node.Value[i]->Tt.Class == ass.CPtrSize)
					cs << "s";
				cs << "(";
				
				Walk(node.Value[i]);
				
				cs << ")";
			}
			else {
				Walk(((IntrinsicNode*)node.Value[i])->Value[0]);
			}
		}
		cs << ")";
	}
	else if (node.Operation == 1) {
		cs << "sizeof(";
		ASSERT(node.Value.GetCount());
		if (node.Value[0]->Tt.Class == ass.CClass)
			WStorageName(ass.Classes[(int)node.Value[0]->IntVal]);
		else
			Walk(node.Value[0]);
		cs << ")";
	}
}

void ZTranspiler::Proc(LoopControlNode& node) {
	if (node.Break)
		cs << "break";
	else
		cs << "continue";
}

void ZTranspiler::Proc(TempNode& node) {
	//if (node.Constructor == nullptr)
	//	return;
	ASSERT(node.Constructor);
	
	ZClass& cls = *node.Tt.Class;
	
	if (cls.CoreSimple) {
		//if (&cls.Namespace() != inNamespace)
			cs << cls._Namespace().BackName << "::";
		cs << cls.Name << "::";
		if (node.Constructor->IsConstructor == 1)
			cs << "_(";
		else
			cs << node.Constructor->BackName << '(';
	}
	else {
		//if (&cls.Namespace() != inNamespace)
			cs << cls._Namespace().BackName << "::";
		if (node.Constructor->IsConstructor == 1)
			cs << cls.BackName;
		else if (node.Constructor->IsConstructor == 2) {
			cs << cls.Name;
			cs << "::";
			cs << node.Constructor->Name;
		}
			
		cs << '(';
	}
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node* p = node.Params[i];
		
		Walk(p);
		if (i < count - 1)
			cs << ", ";
	}
	
	cs << ')';
}

void ZTranspiler::Proc(ListNode& node) {
	cs << '(';
	
	int count = node.Params.GetCount();
	for (int i = 0; i < count; i++) {
		Node *p = node.Params[i];
	
		Walk(p);
		if (i < count - 1)
			cs << ", ";
	}
	
	cs << ')';
}

void ZTranspiler::Proc(PtrNode& node) {
	ASSERT(node.Object);
	if (node.Cast) {
		cs << "(";
		WStorageName(*node.Tt.Next->Class);
		cs << "*)(";
		Walk(node.Object);
		cs << ")";
	}
	else if (node.Nop) {
		// TODO: fix
		cs << "(uint8*)(";
		Walk(node.Object);
		cs << ")";
	}
	else {
		cs << "&(";
		Walk(node.Object);
		cs << ')';
	}
}

void ZTranspiler::Proc(IndexNode& node) {
	Walk(node.Object);
	cs << '[';
	Walk(node.Index);
	cs << ']';
}

void ZTranspiler::Proc(DestructNode& node) {
	Walk(node.Object);
	cs << ".~";
	WStorageName(*node.Object->Tt.Class);
	//cs << node.Object->Tt.Class->BackName;
	cs << "()";
}

void ZTranspiler::Proc(ThrowNode& node) {
	cs << "throw ";
	Walk(node.Exception);
}

void ZTranspiler::Proc(PlacementNewNode& node) {
	if (node.Object->Tt.Class->CoreSimple) {
		Walk(node.Object);
		cs << " = ";
		Walk(node.Value);
	}
	else {
		cs << "new (&";
		Walk(node.Object);
		cs << ") ";
		WClsName(*node.Value->Tt.Class);
		cs << "(";
		if (node.Value)
			Walk(node.Value);
		cs << ")";
	}
}

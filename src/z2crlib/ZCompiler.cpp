#include <z2crlib/ZCompiler.h>
#include <z2crlib/ZTranspiler.h>
#include <z2crlib/ZResolver.h>

extern String opss[];

inline bool invalidClass(ZClass* cls, Assembly& ass) {
	return cls == ass.CVoid || cls == ass.CNull || cls == ass.CClass/* || cls == ass.CDef*/;
}

bool ZCompiler::Compile(bool exc) {
	//MCU = true;
	if (exc == false)
		return compile();
	else {
		try {
			return compile();
		}
		catch (ZException& e) {
			/*for (int i = stack.GetCount() - 1; i >= 0; i--) {
				ZCompilerContext& c = stack[i];
				ZSourcePos& p = c.Pos;
				Cout() << p.Source->Path << "(" << p.P.x << ", " << p.P.y << "): ";
				Cout() << "context: ";
				
				if (c.InstCls)
					Cout() << "instantiating class: " << c.InstCls->Name;
				if (c.InstVar)
					Cout() << "initializing variable: " << c.InstVar->Name;
				if (c.CallFunc)
					Cout() << "calling function: " << c.CallFunc->FuncSig();
				
				Cout() << "\n";
			}*/
			//Cout() << e.Prelude;
			e.PrettyPrint(Cout());
			Cout() << "\n";
			return false;
		}
	}
}

void ZCompiler::SetInUse(ZNamespace& ns) {
	if (ns.InUse)
		return;
	
	ns.SetInUse();
	if (ns.IsClass) {
		ZClass& cls = (ZClass&)ns;
		SetInUse(cls);
	}
}

void ZCompiler::SetInUse(ZClass& cls) {
	if (cls.InUse)
		return;
	
	cls.SetInUse();
	
	/*if (cls.Meth.Default) {
		cls.Meth.Default->SetInUse();
		ExtraFunctions.Add(cls.Meth.Default);
	}*/
	if (cls.Meth.Destructor) {
		cls.Meth.Destructor->SetInUse();
		ExtraFunctions.Add(cls.Meth.Destructor);
	}
	if (cls.Meth.CopyCon) {
		cls.Meth.CopyCon->SetInUse();
		ExtraFunctions.Add(cls.Meth.CopyCon);
	}
	if (cls.Meth.Copy) {
		cls.Meth.Copy->SetInUse();
		ExtraFunctions.Add(cls.Meth.Copy);
	}
	if (cls.Meth.MoveCon) {
		cls.Meth.MoveCon->SetInUse();
		ExtraFunctions.Add(cls.Meth.MoveCon);
	}
	if (cls.Meth.Move) {
		cls.Meth.Move->SetInUse();
		ExtraFunctions.Add(cls.Meth.Move);
	}
	
	if (cls.TBase == ass.CSlice	|| cls.Super == ass.CSlice) {
		int index = cls.Methods.Find("Data");
		
		if (index != -1) {
			ZMethodBundle& meth = cls.Methods[index];
			
			for (ZFunction* f: meth.Functions) {
				if (f->IsProperty) {
					f->SetInUse();
					ExtraFunctions.Add(f);
				}
			}
		}
	}
}

bool ZCompiler::compile() {
	irg.FoldConstants = FoldConstants;
	
	if (!ScanSources())
		return false;
	
	ZResolver resolver(*this);
	resPtr = &resolver;
	if (!resolver.Resolve())
		return false;
	
	int excIndex = -1;
	
	excIndex = ass.Classes.Find("sys.exception.AssertionFailed");
	if (excIndex != -1) {
		SetInUse(ass.Classes[excIndex]);
		cuClasses.Insert(0, &ass.Classes[excIndex]);
	}
	excIndex = ass.Classes.Find("sys.exception.IndexOutOfBounds");
	if (excIndex != -1) {
		SetInUse(ass.Classes[excIndex]);
		cuClasses.Insert(1, &ass.Classes[excIndex]);
	}
	
	MainFound = FindMain();
	
	bool ndp = true;
	
	if (!ndp) {
		for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
			auto& ns = ass.Namespaces[i];
			PreCompileVars(ns);
			
			for (int j = 0; j < ns.Classes.GetCount(); j++) {
				Class = ns.Classes[j];
				
				if (Class->IsTemplate == false)
					PreCompileVars(*Class);
			}
			Class = nullptr;
		}
	}
	
	if (!ndp) {
			for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
	if (MainFunction) {
		MainFunction->SetInUse();
		SetInUse(MainFunction->Owner());
		
		if (MainFunction->InClass) {
			ZClass& pcls = MainFunction->Class();
			
			DoDeps(pcls);
			
			if (pcls.Meth.Default/* && pcls.Meth.Default->IsGenerated == false*/)
				CompileFunc(*pcls.Meth.Default);
		}
		
		if (MainFunction->ShouldEvaluate())
			CompileFunc(*MainFunction);
		
		cuCounter = cuClasses.GetCount() + 1;
		
		if (MainFunction->InClass)
			WriteDeps(MainFunction->Class());
	}
	
	for (ZFunction* func: ExtraFunctions) {
		if (func->ShouldEvaluate())
			CompileFunc(*func);
	}

	for (int i = 0; i < cuClasses.GetCount(); i++) {
		ZClass& cls = *cuClasses[i];
		if (cls.TBase == ass.CSlice) {
			for (int j = 0; j < cls.Methods.GetCount(); j++)
				if (cls.Methods[j].IsConstructor) {
					for (int k = 0; k < cls.Methods[j].Functions.GetCount(); k++) {
						ZFunction& f = *cls.Methods[j].Functions[k];
						if (f.IsConstructor == 1 && f.IsEvaluated == false) {
							f.SetInUse();
							CompileFunc(f);
						}
				}
			}
		}
		else if (cls.TBase != ass.CRaw) {
			for (int j = 0; j < cls.Methods.GetCount(); j++)
				if (cls.Methods[j].IsConstructor) {
					for (int k = 0; k < cls.Methods[j].Functions.GetCount(); k++) {
						ZFunction& f = *cls.Methods[j].Functions[k];
						if (f.IsConstructor == 1 && f.IsEvaluated == false && f.Params.GetCount() == 0 && f.IsGenerated == false) {
							f.SetInUse();
							CompileFunc(f);
						}
				}
			}
		}
	}
	}
	else {
		if (MainFunction) {
			if (MainFunction->InClass) {
				ZClass& pcls = MainFunction->Class();
				
				DoDeps(pcls);
				
				CompileFunc(*MainFunction);
				
				Use << MainFunction;
			}
			else {
				PreCompileVars2(MainFunction->Owner());
				CompileFunc(*MainFunction);
				Use << MainFunction;
			}
		}
		
		/*for (int i = 0; i < cuClasses.GetCount(); i++) {
			ZClass& cls = *cuClasses[i];
			DUMP(cls.Name);
		}*/
	}
	
	if (BuildMode) {
		String cppCode = AppendFileName(BuildPath, "cppcode.h");
		if (!FileExists(cppCode))
			if (!FileCopy(
					AppendFileName(AppendFileName(GetCurrentDirectory(), "codegen"), "cppcode.h"),
									cppCode)) {
				Cout() << "File copy error! Exiting!\n";
				return false;
		}
	}
	
#ifdef PLATFORM_WIN32
	ass.LibLink.FindAdd("winmm");
	ass.LibLink.FindAdd("user32");
#endif
	
	return true;
}

bool ZCompiler::DoDeps(ZClass& c, ZSourcePos* pos, ObjectType* tt) {
	if (c.IsTemplate)
		return true;
	if (c.IsEvaluated2)
		return true;
	
	int index = depCls.Find(&c);
	if (index != -1) {
		LINDENT(-1);
		return false;
	}
	
	// TODO: optimize
	String s = LI;
	if (c.IsEvaluated)
		s << "x ";
	s << "class " << c.Name;
	/*if (c.FromTemplate) {
		ASSERT(c.TBase);
		s << "<" << c.TBase->Name << ">";
	}
	else*/ if (&c == ass.CPtr && tt) {
		s << "<" << tt->Next->Class->Name << ">";
	}
			
	if (c.IsEvaluated) {
		LOG(s + " [" + c.Owner().ProperName + "]" + ": skipped");
		c.IsEvaluated2 = true;
		return true;
	}
		
	LOG(s + " [" + c.Owner().ProperName + "]" + ": instantiation skip static vars {");
	LINDENT(1);
	
	c.IsEvaluated = true;
	
	if (c.Super)
		DoDeps(*c.Super);
	
	if (c.TBase)
		DoDeps(*c.TBase);
	if (c.T)
		DoDeps(*c.T);
	
	ZCompilerContext& zcon = push();
	zcon.Class = &c;
	zcon.Reason = 1;
	zcon.InstCls = &c;
	zcon.Pos = pos ? *pos : c.DefPos;
	
	depCls.Add(&c);
	ZClass* clsBack = Class;
	Class = &c;
	PreCompileVars2(c);
	
	if (c.Meth.CopyCon)
		CompileFunc(*c.Meth.CopyCon);
	if (c.Meth.MoveCon)
		CompileFunc(*c.Meth.MoveCon);
	if (c.Meth.Copy)
		CompileFunc(*c.Meth.Copy);
	if (c.Meth.Copy2)
		CompileFunc(*c.Meth.Copy2);
	if (c.Meth.Move)
		CompileFunc(*c.Meth.Move);
	if (c.Meth.Destructor)
		CompileFunc(*c.Meth.Destructor);
	if (c.TBase == ass.CRaw) {
		ZClass& sup = *c.Super;
		
		index = sup.Methods.Find("this");
		if (index != -1) {
			ZMethodBundle& meth = sup.Methods[index];
			
			for (ZFunction* f: meth.Functions) {
				if (f->IsConstructor == 1 && f->Params.GetCount() == 2) {
					CompileFunc(*f);
					c.Dependencies2.FindAdd(f);
					//ExtraFunctions.Add(f);
					// TODO: WHAT?
					//f->CUIndex = 1;
					
					//DUMP(&cls);
					//DUMP(f);
				}
			}
		}
	}
	
	Class = clsBack;
	LINDENT(-1);
	depCls.RemoveKey(&c);
	
	SetInUse(c);
	
	Pop();
	
	//LOG(LI + c.Owner().Name + c.Name + ": done compiling class");
	LOG(LI + "}");
	
	c.IsEvaluated = true;
	
	return true;
}

ZCompilerContext& ZCompiler::push() {
	ZCompilerContext& zcon = stack.Add();
	zcon.Clear();
	return zcon;
}

bool ZCompiler::PreCompileVars2(ZNamespace& ns) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		ZVariable* v = ns.Variables[i];
		
		if (ns.IsClass && v->IsStatic)
			continue;
		
		PreCompileVars2(v);
	}
	
	return true;
}
		
bool ZCompiler::PreCompileVars2(ZVariable* v) {
	v->MidEval = true;
	
	if (v->IsStatic)
		LOG(LI + "static var " + v->Name + " in class " + v->Owner().Name + ": compiling");
	else
		LOG(LI + "var " + v->Name + " in class " + v->Owner().Name + ": compiling");
	LINDENT(1);
	
	/*if (v->IsEvaluated)
		continue;*/
		
	v->IsDefined = true;
	v->IsEvaluated = true;
	
	ZCompilerContext& zcon = push();
	if (v->Owner().IsClass)
		zcon.Class = (ZClass*)&v->Owner();
	zcon.TargetVar = v;
	zcon.Pos = v->DefPos;
	zcon.InstVar = v;
	
	int stackCount = stack.GetCount();
		 
	try {
		CompileVar(*v, zcon);
	}
	catch (ZException& e) {
		for (int j = 0; j < stack.GetCount(); j++) {
			ZCompilerContext& c = stack[j];
			ZSourcePos& p = c.Pos;
			
			ZException& pre = e.Prelude.Add();
			
			{
				StringStream ss;
				
				ss << p.Source->Path << "(" << p.P.x << ", " << p.P.y << ")";
				
				pre.Path = ss;
			}
						
			{
				StringStream ss;
				
				ss << "context: ";
				
				if (c.InstCls)
					ss << "instantiating class: " << c.InstCls->ColorSig();
				if (c.InstVar)
					ss << "initializing variable: " << c.InstVar->ColorSig();
				if (c.CallFunc)
					ss << "calling function: " << c.CallFunc->ColorSig();
								
				ss << "\n";
				
				pre.Error = ss;
			}
		}
		//e.PrettyPrint(Cout());
		//Cout() << "\n";
		
		//while (stack.GetCount() > stackCount)
		//	stack.Pop();
		
		stack.Clear();
				
		throw e;
		
		return false;
	}
	
	v->MidEval = false;
	
	Pop();
	
	LINDENT(-1);
	//LOG(LI + "}");

	return true;
}

void ZCompiler::LINDENT(int ind) {
	lindent += ind;
	LI = "";
	for (int i = 0; i < lindent; i++)
		LI << "  ";
}

void ZCompiler::WriteDeps(ZClass& cls) {
	if (cls.CUCounter == cuCounter)
		return;
	
	if (cls.IsClass == false)
		return;
	
	writeDeps(cls);
}

void ZCompiler::writeDeps(ZClass& cls) {
	//cls.SetInUse();
	
	/*ZClass * old = Class;
	Class = &cls;
	PreCompileVars(cls);
	Class = old;*/
	
	cls.CUCounter = cuCounter;
	
	for (int i = 0; i < cls.Variables.GetCount(); i++) {
		ZVariable& v = *cls.Variables[i];
		ZClass& used = *v.I.Tt.Class;
			
		if (used.CUCounter < cuCounter)
			writeDeps(used);
	}
	
	//DUMP("Add to cu " + cls.Name);
	cuClasses.FindAdd(&cls);
}

bool ZCompiler::FindMain() {
	Index<ZFunction*> vf;
	
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		ZSource& src = *ass.SourceLookup[i];
		
		if (mainFile.GetCount() && src.Path != mainFile)
			continue;
		
		auto list = FindMain(src);
		for (int j = 0; j < list.GetCount(); j++)
			vf.FindAdd(list[j]);
	}
	
	if (vf.GetCount() == 0)
		return false;
	else if (vf.GetCount() > 1) {
		String err;
		
		err << "multiple '@main' function: other candidates found at: " << "\n";
		for (int i = 1; i < vf.GetCount(); i++)
			err << "\t\t" << vf[i]->DefPos.ToString() << "\n";
		auto exc = ER::Duplicate(vf[0]->DefPos, err);
		Cout() << exc.ToString() << "\n";
		
		return false;
	}
	
	MainFunction = vf[0];
	
	return true;
}

ZFunction *ZCompiler::FindFunction(ZClass& cls, const String& name) {
	int index = cls.Methods.Find("Print");

	if (index == -1 && cls.Super)
		return FindFunction(*cls.Super, name);
	
	for (int i = 0; i < cls.Methods[index].Functions.GetCount(); i++) {
		ZFunction& func = *cls.Methods[index].Functions[i];
		
		if (func.Params.GetCount() == 0) {
			CompileAndUse(func);
			
			return &func;
		}
	}
	
	return nullptr;
}

bool ZCompiler::Transpile() {
	// TODO: enhance
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		if (ass.Namespaces.GetKey(i) == "sys.core.")
			ass.Namespaces[i].BackName = "zc";
		else if (ass.Namespaces.GetKey(i) == "sys.core.lang.")
			ass.Namespaces[i].BackName = "zl";
		//DUMP(ass.Namespaces.GetKey(i));
	}
	
	CppPath = AppendFileName(BuildPath, "out.cpp");
	FileOut out(CppPath);
	ZTranspiler cpp(*this, out);
	cpp.CppVersion = CppVersion;
	//cpp.CheckUse = false;
	
	cpp.WriteIntro();
	//cpp.WriteClassForward();
	
	/*for (int i = 0; i < cuClasses.GetCount(); i++)
		cpp.TranspileClassDeclMaster(*cuClasses[i], 0b11, true);
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i], 0b11, true);
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i], 0b100, false);
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDefinitions(ass.Namespaces[i]);
	for (int i = 0; i < tempInstances.GetCount(); i++)
		cpp.TranspileDefinitions(*tempInstances[i]);*/
		
	
	
	cpp.WriteFuncs(Use);
	
	cpp.WriteOutro();
	
	if (MCU)
		DoMCU();
	
	CheckUnused();
	
	return true;
}

void ZCompiler::CheckUnused() {
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		ZSource& src = *ass.SourceLookup[i];
		
		if (src.IsStdLib == false) {
			DUMP(src.Path);
			for (int j = 0; j < src.Functions.GetCount(); j++) {
				DUMP(src.Functions[j]->FuncSig());
				if (src.Functions[j]->IsEvaluated == false)
					CompileFunc(*src.Functions[j]);
			}
			
			for (int j = 0; j < src.Variables.GetCount(); j++) {
				DUMP(src.Variables[j]->Name);
				PreCompileVars2(src.Variables[j]);
			}
			
			for (int j = 0; j < src.Classes.GetCount(); j++) {
				ZClass& cls = *src.Classes[j];
				
				if (cls.IsEvaluated)
					continue;
				
				DUMP(cls.Name);
				DoDeps(cls);
				
				for (int k = 0; k < cls.Variables.GetCount(); k++) {
					ZVariable& v = *cls.Variables[k];
					
					if (v.IsStatic)
						PreCompileVars2(&v);
				}
			}
		}
	}
}

bool ZCompiler::DoMCU() {
	MCUPath = AppendFileName(BuildPath, "out");
	RealizeDirectory(MCUPath);
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		DoMCU(ass.Namespaces[i]);
	}
	
	return true;
}

void ZCompiler::DoMCU(ZNamespace& ns) {
	bool first = true;
	String nsPath;
	int count = 0;
	
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			if (f.IsExternBind())
				continue;
			
			/*NewMember();
			NL();
			
			if (fDecl && (CheckUse == false || f.InUse)) {
				WriteFunctionDecl(f);
				WriteFunctionBody(f, wrap);
			}*/
			
			Cout() << "MCU: " << f.FuncSig() << "\n";
			
			if (first) {
				nsPath = AppendFileName(MCUPath, ns.ProperName);
				RealizeDirectory(nsPath);
				first = false;
			}
			
			count++;
			String op = AppendFileName(nsPath, f.Name + f.ParamSig() + ".cpp"/*f.Name + IntStr(count) + ".cpp"*/);
			FileOut out(op);
			
			ZTranspiler cpp(*this, out);
			cpp.CppVersion = CppVersion;
			cpp.CheckUse = false;
						
			cpp.WriteIntro();
			
			cpp.BeginNamespace(ns);
			cpp.NewMember();
			
			cpp.NL();
			cpp.WriteFunctionDef(f);
			cpp.ES();
			
			/*for (int k = 0; k < f.Dependencies.GetCount(); k++) {
				ZEntity* entity = f.Dependencies[k];
				
				if (entity->Type == EntityType::Function) {
					cpp.NL();
					cpp.WriteFunctionDef(*(ZFunction*)f.Dependencies[k]);
					cpp.ES();
				}
				else if (entity->Type == EntityType::Variable) {
					cpp.NL();
					
					ZVariable& v = (ZVariable&)*entity;
					cpp.WriteType(&v.I.Tt);
					out << " " << v.Name;
					cpp.WriteTypePost(&v.I.Tt);
					cpp.ES();
				}
			}*/
			
			cpp.EndNamespace();
			
			cpp.WriteFunctionDecl(f);
			cpp.WriteFunctionBody(f, true);
			
			LLVMInput << &f;
			
			MCUPaths << op;
		}
	}
	
	if (count)
		Cout() << "\n";
}

bool ZCompiler::ScanSources() {
	Array<ZScanner> scanners;
	
	// prioritize main hint
	if (mainFile.GetCount()) {
		ZSource* src = ass.FindSource(mainFile);
		if (src)
			ScanSource(*src, scanners);
	}
	
	// scan all source files, throws on critical syntax error
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		ZSource& src = *ass.SourceLookup[i];
		if (src.IsScaned == false)
			ScanSource(src, scanners);
	}

	// print non critical errors
	bool found = false;
	String error;
	for (int i = 0; i < scanners.GetCount(); i++) {
		for (int j = 0; j < scanners[i].Errors.GetCount(); j++) {
			error << scanners[i].Errors[j].ToString() << "\n";
			found = true;
		}
	}
	
	if (found) {
		throw ZException("", error);
		return false;
	}
	
	return true;
}

void ZCompiler::ScanSource(ZSource& src, Array<ZScanner>& scanners) {
	//LOG("Scanning source: " + src.Path);
	src.AddStdClassRefs();
			
	auto& scanner = scanners.Add(ZScanner(ass, src, Platform));
	scanner.Scan();
	src.IsScaned = true;
}

Vector<ZFunction*> ZCompiler::FindMain(ZSource& src) {
	Vector<ZFunction*> mains;
	
	for (int i = 0; i < src.Functions.GetCount(); i++) {
		ZFunction& f = *src.Functions[i];
		if (mainClass.GetCount() && f.Namespace().Name != mainClass + ".")
			continue;
		
		if (f.Name == "@main" && f.Params.GetCount() == 0) {
			if ((f.InClass && !f.IsStatic) || f.InClass == false)
				mains.Add(src.Functions[i]);
		}
	}
	
	return mains;
}

bool ZCompiler::PreCompileVars(ZNamespace& ns) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		ZVariable* v = ns.Variables[i];
		
		if (v->IsEvaluated)
			continue;
		
		ZCompilerContext zcon;
		if (v->Owner().IsClass)
			zcon.Class = (ZClass*)&v->Owner();
		zcon.TargetVar = v;
			 
		CompileVar(*v, zcon);
	}
	
	return true;
}

bool ZCompiler::Compile(ZNamespace& ns) {
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			if (f.ShouldEvaluate())
				CompileFunc(f, f.Nodes);
		}
	}
	
	/*for (int j = 0; j < ns.Classes.GetCount(); j++) {
		Class = ns.Classes[j];
		Compile(*Class);
	}*/
	
	Class = nullptr;
	
	return true;
}

bool ZCompiler::CompileFunc(ZFunction& f, Node& target) {
	LOG(LI + f.FuncSig() + " in class " + f.Owner().Name + ": compiling {");
	LINDENT(1);
	
	if (f.Return.Tt.Class && f.Return.Tt.Class != ass.CVoid)
		DoDeps(*f.Return.Tt.Class);
	
	ZFunction* back = TargetFunc;
	TargetFunc = &f;
	
	ZCompilerContext& zcon = push();
	if (f.Owner().IsClass)
		zcon.Class = (ZClass*)&f.Owner();
	zcon.Func = &f;
	zcon.Pos = f.DefPos;
	zcon.CallFunc = &f;
		
	f.IsEvaluated = true;
	f.InUse = true;
	
	ZClass* clsBack = Class;
	ZFunction* funBack = Function;
	Function = &f;
	if (f.InClass) {
		Class = &f.Class();
		//f.Dependencies.FindAdd(Class);
		
		/*if (Class->IsEvaluated == false) {
			PreCompileVars(*Class);
			Class->IsEvaluated = true;
		}*/
	}
	
	if (f.IsGenerated) {
		LINDENT(-1);
		LOG(LI + f.Name + f.FuncSig() + ": generated function: compiling skipped");
		return true;
	}
	
	if (f.Trait.Flags & ZTrait::BINDC || f.Trait.Flags & ZTrait::BINDCPP) {
		LINDENT(-1);
		LOG(LI + f.Name + f.FuncSig() + ": external function: compiling skipped");
		return true;
	}
	
	ZParser parser(f.BodyPos);
	
	parser.Expect('{');
	
	f.Blocks.Add();
	f.Blocks.Top().Temps = 0;
	
	ZBlockContext con;
	
	while (!parser.IsChar('}')) {
		if (parser.Char2('@', '[')) {
			lastTrait.TP = parser.GetFullPos();
			lastTrait.Parse(parser);
			useLastTrait = true;
		}
		
		Node* node = CompileStatement(f, parser, con);
		
		if (node != nullptr)
			target.AddChild(node);
	}
	
	auto pp = parser.GetFullPos();
	parser.Expect('}');
	
	// TODO: make smarter
	/*if (!con.Return && f.Return.Tt.Class != ass.CVoid) {
		ZExprParser ep(f, Class, &f, *this, parser, irg);
		Vector<Node*> dummy;
		Node* defRet = ep.Temporary(*f.Return.Tt.Class, dummy, &pp);
		target.AddChild(irg.ret(defRet));
	}*/
	if (!con.Return && f.IsConstructor == 0 && f.Return.Tt.Class != ass.CVoid)
		parser.Error(pp.P, "function expected to have return statement");
	
	f.Blocks.Drop();
	
	Class = clsBack;
	Function = funBack;
	
	TargetFunc = back;
	
	Pop();
	
	LINDENT(-1);
	LOG(LI + "}");
	
	if (f.Dependencies2.GetCount()) {
		StringStream ss;
		for (int i = 0; i < f.Dependencies2.GetCount(); i++) {
			ZEntity& e = *f.Dependencies2[i];
			if (i)
				ss << ", ";
			if (e.Type == EntityType::Class)
				ss << "class " << e.Name;
			if (e.Type == EntityType::Variable)
				ss << "var " << e.Name << " in " << e.Owner().Name;
			if (e.Type == EntityType::Function)
				ss << ((ZFunction&)e).FuncSig() << " in " << e.Owner().Name;
		}
		LOG(LI + f.FuncSig() + " depends on: " + ss);
		//if (f.Name == "InitChunks" /*|| f.Name == "Set" || f.Name == "SetTileVariants" || f.Name == "SetTileVariants2" || f.Name == "SetTileVariants3" || f.Name == "AddTrees" || f.Name == "Update"*/)
		//	Use << &f;
	}
	
	return true;
}

Node* ZCompiler::CompileStatement(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	bool originalUse = useLastTrait;
	useLastTrait = false;
	
	if (parser.Char('{')) {
		Node* node = CompileBlock(f, parser, con);
		
		return node;
	}
	else if (parser.Char(';'))
		return nullptr;
	else if (parser.Id("if"))
		return CompileIf(f, parser, con);
	else if (parser.Id("while"))
		return CompileWhile(f, parser, con);
	else if (parser.Id("do"))
		return CompileDoWhile(f, parser, con);
	else if (parser.Id("for"))
		return CompileFor(f, parser, con);
	else if (parser.Id("val")) {
		Node* node = CompileLocalVar(f, parser, false, originalUse);
		parser.ExpectEndStat();
		return node;
	}
	else if (parser.Id("const")) {
		Node* node = CompileLocalVar(f, parser, true, originalUse);
		parser.ExpectEndStat();
		return node;
	}
	else if (parser.Id("return"))
		return CompileReturn(f, parser, con);
	else if (parser.IsId("break")) {
		ZSourcePos sp = parser.GetFullPos();
		parser.Id("break");
		if (!con.InLoop)
			parser.Error(sp.P, "'" + ER::Blue + "break" + ER::White + "' found outside of loop");
		return irg.loopControl(true);
	}
	else if (parser.IsId("continue")) {
		ZSourcePos sp = parser.GetFullPos();
		parser.Id("continue");
		if (!con.InLoop)
			parser.Error(sp.P, "'" + ER::Blue + "continue" + ER::White + "' found outside of loop");
		return irg.loopControl(false);
	}
	else if (parser.Id("throw")) {
		ZExprParser ep(f, Class, &f, *this, parser, irg);
		Node* exc = ep.Parse();
		parser.ExpectEndStat();
		return irg.throwExc(exc);
	}
	else {
		Node* node = CompileExpression(f, parser, con);
		parser.ExpectEndStat();
		return node;
	}
	
	return nullptr;
}

Node* ZCompiler::CompileExpression(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	ZExprParser ep(f, Class, &f, *this, parser, irg);
	auto pp = parser.GetFullPos();
	Node* node = ep.Parse();
	
	bool assign = true;
	bool useOp = true;
	OpNode::Type op = OpNode::Type::opNotSet;
	
	auto opp = parser.GetPoint();
	if (parser.Char('='))
		useOp = false;
	else if (parser.Char2('+', '='))
		op = OpNode::Type ::opAdd;
	else if (parser.Char2('-', '='))
		op = OpNode::Type ::opSub;
	else if (parser.Char2('*', '='))
		op = OpNode::Type ::opMul;
	else if (parser.Char2('/', '='))
		op = OpNode::Type ::opDiv;
	else if (parser.Char2('%', '='))
		op = OpNode::Type ::opMod;
	else
		assign = false;
	
	if (assign) {
		Node* rs = ep.Parse();
		Node* opAssignTemp = nullptr;
		
		if (op != OpNode::Type::opNotSet) {
			opAssignTemp = irg.op(node, rs, OpNode::Type(op), opp);
		
			if (opAssignTemp == nullptr)
				opAssignTemp = ep.ResolveOpOverload(node, rs, op, opp);
		}
		
		//if (node->IsAddressable == false && node->IsEffLValue == false)
		//	parser.Error(pp.P, "left side of assignment is not a L-value");
		if (node->IsConst)
			parser.Error(pp.P, "can't assign to readonly " + ass.ToQtColor(node) + " instance");
		
		if (!node->CanAssign(ass, rs)) {
			parser.Error(pp.P, "can't assign " + ass.ToQtColor(rs) + " instance to " + ass.ToQtColor(node) + " instance without a cast");
		}
		
		ZClass* cls = node->Tt.Class;
		
		if (cls == ass.CDef)
			FindLambda(rs);
		
		if (node && node->Chain && node->Chain->PropCount) {
			Node* child = node->Chain->First;

			while (child) {
				if (child->NT == NodeType::CallFunc) {
					CallNode *p = (CallNode*)child;
					if (p->Function->IsProperty && p->Function->Bundle->PropSetter) {
						//DUMP(p->Function->Bundle->PropSetter);
						ep.TestAccess(*p->Function->Bundle->PropSetter, pp.P);
						p->Function = p->Function->Bundle->PropSetter;
						p->Function->InUse = true;
						
						CompileAndUse(*p->Function);
						//if (p->Function->ShouldEvaluate())
						//	CompileFunc(*p->Function);
						
						//f.Dependencies.FindAdd(p->Function);
					}
				}
				child = child->Next;
			}
		}
		
		return irg.attr(node, rs, op, opAssignTemp);
	}

	return node;
}

Node* ZCompiler::CompileBlock(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	f.Blocks.Add();
	f.Blocks.Top().Temps = 0;
	
	ZBlockContext blockCon;
	blockCon.InLoop = con.InLoop;
	
	BlockNode* block = irg.block();
	
	while (!parser.IsChar('}')) {
		Node* node = CompileStatement(f, parser, blockCon);
		
		if (node != nullptr)
			block->Nodes.AddChild(node);
	}
	
	parser.Expect('}');
	parser.EatNewlines();
	
	if (blockCon.Return)
		con.Return = true;
	
	f.Blocks.Drop();
	
	return block;
}

Node* ZCompiler::CompileIf(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, Class, &f, *this, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.EatNewlines();
	
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, ER::Blue + "if" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
	
	ZBlockContext trueCon;
	trueCon.InLoop = con.InLoop;
	
	Node* tb = CompileStatement(f, parser, trueCon);
	Node* fb = nullptr;
	
	if (parser.Id("else")) {
		ZBlockContext falseCon;
		falseCon.InLoop = con.InLoop;
		fb = CompileStatement(f, parser, falseCon);
		
		if (trueCon.Return && falseCon.Return)
			con.Return = true;
	}
	else
		; // no else branch, can't be sure about return
	
	return irg.ifcond(node, tb, fb);
}

Node* ZCompiler::CompileWhile(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, Class, &f, *this, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.EatNewlines();
	
	ZBlockContext loopCon;
	loopCon.InLoop = true;
	Node* bd = CompileStatement(f, parser, loopCon);
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, ER::Blue + "while" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
		
	return irg.whilecond(node, bd);
}

Node* ZCompiler::CompileDoWhile(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	if (!parser.IsChar('{'))
		parser.Expect('{');
	
	ZBlockContext loopCon;
	loopCon.InLoop = true;
	Node* bd = CompileStatement(f, parser, loopCon);
	if (loopCon.Return)
		con.Return = true;
	
	parser.ExpectId("while");
	
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, Class, &f, *this, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.ExpectEndStat();
	parser.EatNewlines();
	
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, ER::Blue + "do" + ER::White + ".." + ER::Blue + "while" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
		
	return irg.dowhilecond(node, bd);
}

Node* ZCompiler::CompileFor(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	Node* init = nullptr;
	Node* iter = nullptr;
	bool addedBlock = false;
	
	parser.Expect('(');

	ZExprParser ep(f, Class, &f, *this, parser, irg);
	if (!parser.IsChar(';')) {
		if (parser.Id("val")) {
			f.Blocks.Add();
			f.Blocks.Top().Temps = 0;
			addedBlock = true;
			
			init = CompileLocalVar(f, parser, false, false);
		}
		else
			init = CompileExpression(f, parser, con);
	}
		
	parser.Expect(';');
	
	Point p = parser.GetPoint();

	Node* node = ep.Parse();
	
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, ER::Blue + "for" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
	
	parser.Expect(';');
	
	if (!parser.IsChar(')'))
		iter = CompileExpression(f, parser, con);
	
	parser.Expect(')');
	parser.EatNewlines();
	
	ZBlockContext loopCon;
	loopCon.InLoop = true;
	Node* bd = CompileStatement(f, parser, loopCon);
	if (loopCon.Return)
		con.Return = true;
	
	if (addedBlock)
		f.Blocks.Drop();
	
	return irg.forloop(init, node, iter, bd);
}

bool ZCompiler::CompileVar(ZVariable& v, const ZCompilerContext& zcon) {
	v.IsEvaluated = true;
	
	ZParser parser(v.DefPos);
	parser.ExpectZId();
	
	Node* node = compileVarDec(v, parser, v.DefPos, zcon);
	parser.ExpectEndStat();
	
	if (v.InClass && !v.I.Tt.Class->CoreSimple) {
		ObjectType* tt = &v.I.Tt;
		while (tt != nullptr) {
			if (tt->Class->FromTemplate) {
				v.Owner().DependsOn.FindAdd(tt->Class->TBase);
				v.Owner().DependsOn.FindAdd(tt->Class->T);
				tt = &tt->Class->TBase->Tt;
			}
			else {
				if (tt->Class == ass.CPtr)
					break;
				v.Owner().DependsOn.FindAdd(tt->Class);
				tt = tt->Next;
			}
		}
	}

	return node;
}

Node *ZCompiler::CompileLocalVar(ZFunction& f, ZParser& parser, bool aConst, bool aUseLastTrait) {
	auto vp = parser.GetFullPos();
	
	String name = parser.ExpectZId();
	if (name == "f1")
		name == "smoke";
	TestVarDup(Class, f, name, vp);
	
	ZVariable& v = f.Locals.Add(ZVariable(f.Namespace()));
	v.Name = name;
	v.BackName = name;
	v.DefPos = vp;
	v.Section = f.Section;
	v.IsConst = aConst;
	//v.IsDefined = true;
	if (aUseLastTrait)
		v.Trait = lastTrait;

	ZCompilerContext zcon;
	zcon.Class = (ZClass*)&f.Owner();
	zcon.Func = &f;
	zcon.TargetVar = &v;
		
	Node* node = compileVarDec(v, parser, vp, zcon);
	SetInUse(*v.I.Tt.Class);
	return node;
}

Node *ZCompiler::compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, const ZCompilerContext& zcon) {
	ZClass* cls = nullptr;

	if (parser.Char(':')) {
		auto ti = ZExprParser::ParseType(*this, parser, true, false, zcon.Class, zcon.Func);
		
		if (invalidClass(ti.Tt.Class, ass))
			parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ti.Tt.Class));
		if (ti.Tt.Class == ass.CPtr && ti.Tt.Next->Class == ass.CPtr)
			parser.Error(vp.P, "pointers to pointer are currently not supported");
		
		v.I = ti;
		cls = ti.Tt.Class;
		
		if (cls->IsEvaluated == false)
			DoDeps(*cls);
		
		//if (cls->IsEvaluated == false)
		//	DoDeps(*cls, &vp);
	}
	
	bool assign = false;
	/*if (ref) {
		parser.Expect('<', '-');
		assign = true;
	}
	else {*/
		if (parser.Char('='))
			assign = true;
	//}
	
	if (assign) {
		ZExprParser ep(v, zcon.Class, zcon.Func, *this, parser, irg);
		Node* node = ep.Parse();
		
		if (!cls) {
			if (invalidClass(node->Tt.Class, ass))
				parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(node->Tt.Class));
			if (node->Tt.Class == ass.CPtr && node->Tt.Next->Class == ass.CPtr)
				parser.Error(vp.P, "pointers to pointer are currently not supported");
			v.I.Tt = node->Tt;
			cls = node->Tt.Class;
		}
			
		if (v.I.CanAssign(ass, node)) {
			v.Value = node;
		}
		else if (node->IsLiteral && node->Tt.Class == ass.CVoid) {
			// literal null initialization, leave variable uninitialized
			v.Value = nullptr;
		}
		else {
			parser.Error(vp.P, "can't assign " + ass.TypeToColor(node->Tt) +
					" instance to " + ass.TypeToColor(v.I.Tt) + " instance without a cast");
		}
		
		if (cls == ass.CDef)
			FindLambda(node);
	}
	else {
		if (v.I.Tt.Class == NULL)
			parser.Error(vp.P, "variable must have either an explicit type or be initialized");
		
		if (!DoDeps(*cls, &vp, &v.I.Tt))
			parser.Error(vp.P, "circular class instantiation: " + ass.TypeToColor(cls->Tt));
		
		if (cls->CoreSimple == false && cls->TBase == nullptr) {
			if (cls->Meth.Default == nullptr)
				parser.Error(vp.P,  "class "+ ass.ToQtColor(&cls->Tt) + " does not have a default constructor");
			else {
				if (cls->Meth.Default && cls->Meth.Default->IsDeleted)
					parser.Error(vp.P,  "class "+ ass.ToQtColor(&cls->Tt) + ": default constructor is deleted");
		
				cls->Meth.Default->SetInUse();
			}
		}
		
		if (v.Value == nullptr) {
			ZExprParser ep(v, Class, zcon.Func, *this, parser, irg);
			Vector<Node*> params;
			//ep.getParams(params);
			
			if (cls->FromTemplate)
				Push(vp, *cls);
			
			//if (v.I.Tt.Class->TBase == nullptr || v.I.Tt.Class->TBase != ass.CRaw)
				v.Value = ep.Temporary(*v.I.Tt.Class, params, vp);
				
			if (cls->FromTemplate)
				Pop();
		}
	}
	
	/*if (cls && cls->IsEvaluated == false) {
		cls->IsEvaluated = true;
		PreCompileVars(*cls);
	}*/
		
	if (zcon.Func) {
		ZBlock& b = zcon.Func->Blocks[zcon.Func->Blocks.GetCount() - 1];
		b.Locals.Add(v.Name, &v);
	}
	
	return irg.local(v);
}

void ZCompiler::FindLambda(Node* node) {
	ZLambdaInfo& info = ass.Lambdas[node->Tt.Param];
	ASSERT(info.Params);
	ASSERT(node->NT == NodeType::Lambda);
	LambdaNode& lambda = *(LambdaNode*)node;
	ASSERT(lambda.Bundle);
	
	int index = Function->Owner().Methods.Find(lambda.Bundle->Name);

	if (index != -1) {
		for (int i = 0; i < lambda.Bundle->Functions.GetCount(); i++) {
			ZFunction& f = *lambda.Bundle->Functions[i];
			
			if (f.Params.GetCount() == info.Params->Params.GetCount()) {
				lambda.Function = &f;
				CompileAndUse(f);
			}
		}
	}
}

Node *ZCompiler::CompileReturn(ZFunction& f, ZParser& parser, ZBlockContext& con) {
	auto p = parser.GetPoint();
	
	if (f.IsConstructor)
		parser.Error(p, "constructors can't have a '" + ER::Magenta + "return" + ER::White + "' statement");
	
	Node* retVal = nullptr;
	
	if (f.Return.Tt.Class == ass.CVoid)
		parser.ExpectEndStat();
	else {
		ZExprParser ep(f, Class, &f, *this, parser, irg);
		retVal = ep.Parse();
		parser.ExpectEndStat();
		
		if (!f.Return.CanAssign(ass, retVal))
			parser.Error(p, "can't assign " + ass.ToQtColor(retVal) +
				" instance to " + ass.ToQtColor(f.Return.Tt.Class) + " instance without a cast");
	}
	
	con.Return = true;
	
	return irg.ret(retVal);
}

ZClass& ZCompiler::ResolveInstance(ZClass& cc, ZClass& sub, const ZSourcePos& p, bool eval) {
	String fullName;
	fullName << cc.Namespace().Name << cc.Name << "<" << sub.Namespace().Name << sub.Name << ">";
	
	int i = ass.Classes.Find(fullName);
	if (i != -1) {
		ZClass& tclass = ass.Classes[i];
		return tclass;
	}
	
	i = ass.Classes.GetCount();
	
	//LOG("Instantiating " + fullName);
	
	ZClass& tclass = ass.Classes.Add(fullName, ZClass(cc.Namespace()));
	
	tclass.Name = String().Cat() << cc.Name << "<" << sub.Name << ">";
	tclass.BackName = String().Cat() << cc.Name << "_";
	if (sub.FromTemplate)
		tclass.BackName << sub.BackName;
	else
		tclass.BackName << sub.Name;
	tclass.Index = i;
	tclass.IsClass = true;
	tclass.FromTemplate = true;
	tclass.TBase = &cc;
	tclass.T = &sub;
	tclass.Access = cc.Access;
	tclass.ParamType = &tclass;
	tclass.StorageName = tclass.BackName;
	
	ObjectType* t = &cc.Temps.Add();
	t->Class = &tclass;
	t->Next = 0;
	t->Param = 0;
	tclass.Tt = *t;
	tclass.Pt = ass.GetPtr(&t->Class->Tt);
	
	tclass.CopyPreSection(cc);
	resPtr->ResolveNamespaceMembers(tclass);
	resPtr->ResolveVariables(tclass);
	
	//DoDeps()
	
	//PreCompileVars(tclass);

	//PreCompileVars(tclass);
	
	tempInstances.Add(&tclass);
	
	if (cc.SuperPos.Source) {
		ZParser parser(cc.SuperPos);
		ZExprParser exp(tclass, &tclass, nullptr, *this, parser, irg);
		auto res = exp.ParseType(*this, parser, false, false, &tclass);
		tclass.Super = res.Tt.Class;
		
		//DUMP("Inst add to cu " + sub.Name);
		cuClasses.FindAdd(&sub);
		SetInUse(sub);
		//DUMP("Inst add to cu " + tclass.Super->Name);
		cuClasses.Add(tclass.Super);
		
		tclass.Super->Meth.Default->SetInUse();
	}
	
	//DUMP("Inst add to cu " + sub.Name);
	cuClasses.FindAdd(&sub);
	
	//DUMP("Inst add to cu " + tclass.Name);
	cuClasses.FindAdd(&tclass);
	
	SetInUse(tclass);
	
	return tclass;
}

void ZCompiler::Push(const ZSourcePos& pos, ZClass& cls) {
	ZCompilerContext& zcon = stack.Add();
	zcon.Clear();
	zcon.Pos = pos;
	zcon.InstCls = &cls;
}

void ZCompiler::Pop() {
	delete stack.PopDetach();
}

void ZCompiler::TestVarDup(ZClass* cls, ZFunction& over, const String& name, const ZSourcePos& cur) {
	if (over.Name == name)
		throw ER::Duplicate(name, cur, over.DefPos);

	if (cls && cls->Name == name)
		throw ER::Duplicate(name, cur, cls->DefPos);

	for (int i = 0; i < over.Params.GetCount(); i++)
		if (over.Params[i].Name == name)
			throw ER::Duplicate(name, cur, over.Params[i].DefPos);

	for (int j = 0; j < over.Blocks.GetCount(); j++)
		for (int k = 0; k < over.Blocks[j].Locals.GetCount(); k++) {
			if (over.Blocks[j].Locals[k]->Name == name)
				throw ER::Duplicate(name, cur, over.Blocks[j].Locals[k]->DefPos);
		}
		
	/*for (int i = 0; i < cls.Vars.GetCount(); i++)
		if (cls.Vars[i].Name == name)
			parser.Warning(p, "local '" + name + "' hides a class member");*/
}

ZCompiler::ZCompiler(Assembly& aAss): ass(aAss), irg(ass, *this) {
#ifdef PLATFORM_WIN32
	Platform = PlatformType::WINDOWS;
	PlatformString = "WIN32";
	PlatformSysLib = "microsoft.windows";
#endif
	
#ifdef PLATFORM_POSIX
	Platform = PlatformType::POSIX;
	PlatformString = "POSIX";
	PlatformSysLib = "ieee.posix";
#endif
			
	ZExprParser::Initialize();
}

String& ZCompiler::GetName() {
	static String name = "Z2CR 0.3.3 (pre-alpha)";
	return name;
}
#include <z2crlib/ZCompiler.h>
#include <z2crlib/ZTranspiler.h>

bool ZCompiler::Compile() {
	irg.FoldConstants = FoldConstants;
	
	if (!ScanSources())
		return false;
	
	ResolveNamespaces();
	
	if (!CheckForDuplicates())
		return false;
	
	MainFound = FindMain();
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		PreCompileVars(ass.Namespaces[i]);
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
	
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
	
	return true;
}

bool ZCompiler::FindMain() {
	Index<ZFunction*> vf;
	
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		if (mainFile.GetCount() && ass.SourceLookup[i]->Path != mainFile)
			continue;
		
		auto list = FindMain(*ass.SourceLookup[i]);
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

void ZCompiler::ResolveNamespaces() {
	ResolveNamespace(ass.CoreNamespace());
	ResolveNamespace(ass.LangNamespace());
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		if (ns.IsResolved || ns.Name == "::")
			continue;
		
		ResolveNamespace(ns);
	}
}

void ZCompiler::ResolveNamespace(ZNamespace& ns) {
	auto names = Split(ns.Name, '.', true);
	
	ZNamespaceItem* nsp = &ass.NsLookup;
	String backName;
	for (int j = 0; j < names.GetCount(); j++) {
		backName << names[j];
		nsp = nsp->Add(names[j]);
		if (j < names.GetCount() - 1)
			backName << "::";
		else {
			ASSERT(nsp->Namespace == nullptr);
			nsp->Namespace = &ns;
			
			ns.NamespaceItem = nsp;
		}
	}
	
	ns.BackName = backName;
	
	for (int j = 0; j < ns.Sections.GetCount(); j++) {
		ZNamespaceSection& sec = ns.Sections[j];
		
		for (int k = 0; k < sec.UsingNames.GetCount(); k++) {
			ZNamespace* usn = ass.Namespaces.FindPtr(sec.UsingNames[k]);
			if (usn)
				sec.Using.Add(usn);
		}
	}
	
	ns.IsResolved = true;
}

bool ZCompiler::Transpile() {
	CppPath = AppendFileName(BuildPath, "out.cpp");
	FileOut out(CppPath);
	ZTranspiler cpp(*this, out);
	//cpp.PrintDebug = true;
	
	cpp.WriteIntro();
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i]);
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDefinitions(ass.Namespaces[i]);
	cpp.WriteOutro();
	
	return true;
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
	for (int i = 0; i < scanners.GetCount(); i++) {
		for (int j = 0; j < scanners[i].Errors.GetCount(); j++) {
			Cout() << scanners[i].Errors[j].ToString() << "\n";
			found = true;
		}
	}
	
	if (found)
		return false;
	
	return true;
}

void ZCompiler::ScanSource(ZSource& src, Array<ZScanner>& scanners) {
	src.AddStdClassRefs();
			
	auto& scanner = scanners.Add(ZScanner(src, Platform));
	scanner.Scan();
	src.IsScaned = true;
}

Vector<ZFunction*> ZCompiler::FindMain(ZSource& src) {
	Vector<ZFunction*> mains;
	
	for (int i = 0; i < src.Functions.GetCount(); i++) {
		if (mainClass.GetCount() && src.Functions[i]->Namespace().Name != mainClass + ".")
			continue;
		if (src.Functions[i]->Name == "@main" && src.Functions[i]->Params.GetCount() == 0)
			mains.Add(src.Functions[i]);
	}
	
	return mains;
}

bool ZCompiler::PreCompileVars(ZNamespace& ns) {
	for (int i = 0; i < ns.Variables.GetCount(); i++) {
		ZVariable* v = ns.Variables[i];
		CompileVar(*v);
	}
	
	return true;
}

void printNode(Node* node, int level = 0) {
	String s;
	for (int i = 0; i < level; i++)
		s << "\t";
	s << "<" << (int)node->NT << ">\n";
	DUMP(s);
	
	Node* child = node->First;
	
	while (child) {
		printNode(child, level + 1);
		
		child = child->Next;
	}
	
	s = "";
	for (int i = 0; i < level; i++)
		s << "\t";
	s << "</" << (int)node->NT << ">\n";
	DUMP(s);
}

bool ZCompiler::Compile(ZNamespace& ns) {
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			CompileFunc(f, f.Nodes);
			
			//printNode(&f.Nodes);
		}
	}
	
	return true;
}

bool ZCompiler::CompileFunc(ZFunction& f, Node& target) {
	ZParser parser(f.BodyPos);
	
	parser.Expect('{');
	
	f.Blocks.Add();
	f.Blocks.Top().Temps = 0;
	
	ZContext con;
	
	while (!parser.IsChar('}')) {
		Node* node = CompileStatement(f, parser, con);
		
		if (node != nullptr)
			target.AddChild(node);
	}
	
	parser.Expect('}');
	
	// TODO: make smarter
	if (!con.Return && f.Return.Tt.Class != ass.CVoid) {
		Vector<Node*> dummy;
		Node* defRet = ZExprParser::Temporary(ass, irg, *f.Return.Tt.Class, dummy);
		target.AddChild(irg.ret(defRet));
	}
	
	f.Blocks.Drop();
	
	return true;
}

Node* ZCompiler::CompileStatement(ZFunction& f, ZParser& parser, ZContext& con) {
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
	else if (parser.Id("val"))
		return CompileLocalVar(f, parser);
	else if (parser.Id("return"))
		return CompileReturn(f, parser, con);
	else if (parser.IsId("break")) {
		ZSourcePos sp = parser.GetFullPos();
		parser.Id("break");
		if (!con.InLoop)
			parser.Error(sp.P, "'break' found outside of loop");
		return irg.loopControl(true);
	}
	else if (parser.IsId("continue")) {
		ZSourcePos sp = parser.GetFullPos();
		parser.Id("continue");
		if (!con.InLoop)
			parser.Error(sp.P, "'continue' found outside of loop");
		return irg.loopControl(false);
	}
	else {
		ZExprParser ep(f, parser, irg);
		ep.Function = &f;
		auto pp = parser.GetFullPos();
		Node* node = ep.Parse();
		
		if (parser.Char('=')) {
			Node* rs = ep.Parse();
			
			if (node->IsAddressable == false)
				parser.Error(pp.P, "left side of assignment is not a L-value");
			if (node->IsConst)
				parser.Error(pp.P, "can't assign to readonly '\f" + ass.ClassToString(rs) + "\f'");
			
			if (!node->CanAssign(ass, rs)) {
				parser.Error(pp.P, "can't assign '\f" + ass.ClassToString(rs) +
					"\f' instance to '\f" + ass.ClassToString(node) + "\f' instance without a cast");
			}
			
			return irg.attr(node, rs);
		}

		parser.ExpectEndStat();
		
		return node;
	}
	
	return nullptr;
}

Node* ZCompiler::CompileBlock(ZFunction& f, ZParser& parser, ZContext& con) {
	f.Blocks.Add();
	f.Blocks.Top().Temps = 0;
	
	ZContext blockCon;
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

Node* ZCompiler::CompileIf(ZFunction& f, ZParser& parser, ZContext& con) {
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.EatNewlines();
	
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, "if condition must be '\fBool\f', '\f" + ass.ClassToString(node) + "\f' found");
	
	ZContext trueCon;
	trueCon.InLoop = con.InLoop;
	
	Node* tb = CompileStatement(f, parser, trueCon);
	Node* fb = nullptr;
	
	if (parser.Id("else")) {
		ZContext falseCon;
		falseCon.InLoop = con.InLoop;
		fb = CompileStatement(f, parser, falseCon);
		
		if (trueCon.Return && falseCon.Return)
			con.Return = true;
	}
	else
		; // no else branch, can't be sure about return
	
	return irg.ifcond(node, tb, fb);
}

Node* ZCompiler::CompileWhile(ZFunction& f, ZParser& parser, ZContext& con) {
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.EatNewlines();
	
	ZContext loopCon;
	loopCon.InLoop = true;
	Node* bd = CompileStatement(f, parser, loopCon);
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, "while condition must be '\fBool\f', '\f" + ass.ClassToString(node) + "\f' found");
		
	return irg.whilecond(node, bd);
}

Node* ZCompiler::CompileDoWhile(ZFunction& f, ZParser& parser, ZContext& con) {
	if (!parser.IsChar('{'))
		parser.Expect('{');
	
	ZContext loopCon;
	loopCon.InLoop = true;
	Node* bd = CompileStatement(f, parser, loopCon);
	if (loopCon.Return)
		con.Return = true;
	
	parser.ExpectId("while");
	
	parser.Expect('(');
	Point p = parser.GetPoint();
	
	ZExprParser ep(f, parser, irg);
	Node* node = ep.Parse();
	parser.Expect(')');
	parser.ExpectEndStat();
	parser.EatNewlines();
	
	if (node->Tt.Class != ass.CBool)
		parser.Error(p, "do... while condition must be '\fBool\f', '\f" + ass.ClassToString(node) + "\f' found");
		
	return irg.dowhilecond(node, bd);
}

bool ZCompiler::CompileVar(ZVariable& v) {
	ZParser parser(v.DefPos);
	parser.ExpectZId();
	
	return compileVarDec(v, parser, v.DefPos, nullptr);
}

Node *ZCompiler::CompileLocalVar(ZFunction& f, ZParser& parser) {
	auto vp = parser.GetFullPos();
	
	String name = parser.ExpectZId();
	TestVarDup(/*cls, */f, name, vp);
	
	ZVariable& v = f.Locals.Add(ZVariable(f.Namespace()));
	v.Name = name;
	v.BackName = name;
	v.DefPos = vp;
	v.Section = f.Section;
	
	return compileVarDec(v, parser, vp, &f);
}

Node *ZCompiler::compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, ZFunction* f) {
	ZClass* cls = nullptr;
	if (parser.Char(':')) {
		cls = ZExprParser::ParseType(ass, parser);
		if (cls == ass.CVoid)
			parser.Error(vp.P, "can't create a variable of type '\fVoid\f'");
		v.I.Tt = cls->Tt;
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
		ZExprParser ep(v, parser, irg);
		ep.Function = f;
		Node* node = ep.Parse();
		parser.ExpectEndStat();
		
		if (!cls) {
			if (node->Tt.Class == ass.CVoid)
				parser.Error(vp.P, "can't create a variable of type '\fVoid\f'");
			v.I.Tt = node->Tt;
		}
		
		if (v.I.Tt.Class == ass.CCls)
			parser.Error(vp.P, "can't create a variable of type '\fClass\f'");
			
		if (v.I.CanAssign(ass, node)) {
			v.Value = node;
		}
		else if (node->IsLiteral && node->Tt.Class == ass.CVoid) {
			// literal null initialization, leave variable uninitialized
			v.Value = nullptr;
		}
		else {
			parser.Error(vp.P, "can't assign '\f" + ass.ClassToString(node) +
					"\f' instance to '\f" + ass.ClassToString(&v.I) + "\f' instance without a cast");
		}
	}
	else {
		parser.ExpectEndStat();
		
		if (v.I.Tt.Class == NULL)
			parser.Error(vp.P, "variable must have either an explicit type or be initialized");
		
		if (v.I.Tt.Class == ass.CCls)
			parser.Error(vp.P, "can't create a variable of type '\fClass\f'");
		if (v.I.Tt.Class == ass.CVoid)
			parser.Error(vp.P, "can't create a variable of type '\fVoid\f'");
		
		if (v.Value == nullptr) {
			Vector<Node*> params;
			v.Value = ZExprParser::Temporary(ass, irg, *v.I.Tt.Class, params);
		}
	}
		
	if (f) {
		ZBlock& b = f->Blocks[f->Blocks.GetCount() - 1];
		b.Locals.Add(v.Name, &v);
	}
	
	return irg.local(v);
}

Node *ZCompiler::CompileReturn(ZFunction& f, ZParser& parser, ZContext& con) {
	auto p = parser.GetPoint();
	
	if (f.IsConstructor)
		parser.Error(p, "constructors can't have a 'return' statement");
	
	Node* retVal = nullptr;
	
	if (f.Return.Tt.Class == ass.CVoid)
		parser.ExpectEndStat();
	else {
		ZExprParser ep(f, parser, irg);
		ep.Function = &f;
		retVal = ep.Parse();
		parser.ExpectEndStat();
		
		if (!f.Return.CanAssign(ass, retVal))
			parser.Error(p, "can't assign '\f" + ass.ClassToString(retVal) +
				"\f' instance to '\f" + ass.ClassToString(f.Return.Tt) + "\f' instance without a cast");
	}
	
	con.Return = true;
	
	return irg.ret(retVal);
}

void ZCompiler::TestVarDup(/*ZClass& cls,*/ ZFunction& over, const String& name, const ZSourcePos& cur) {
	if (over.Name == name)
		throw ER::Duplicate(name, over.DefPos, cur);

	/*if (cls.Scan.Name == name)
		parser.Dup(p, cls.Position, name);*/

	for (int i = 0; i < over.Params.GetCount(); i++)
		if (over.Params[i].Name == name)
			throw ER::Duplicate(name, over.Params[i].DefPos, cur);

	for (int j = 0; j < over.Blocks.GetCount(); j++)
		for (int k = 0; k < over.Blocks[j].Locals.GetCount(); k++) {
			if (over.Blocks[j].Locals[k]->Name == name)
				throw ER::Duplicate(name, over.Blocks[j].Locals[k]->DefPos, cur);
		}
		
	/*for (int i = 0; i < cls.Vars.GetCount(); i++)
		if (cls.Vars[i].Name == name)
			parser.Warning(p, "local '" + name + "' hides a class member");*/
}

bool ZCompiler::CheckForDuplicates() {
	dupes.Clear();
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		CheckForDuplicates(ass.Namespaces[i]);
	
	if (dupes.GetCount() == 0)
		return true;
		
	for (int i = 0; i < dupes.GetCount(); i++) {
		Vector<ZSourcePos>& list = dupes[i];
		
		ASSERT(list.GetCount() >= 2);
		
		String err;
		
		err << "Duplicate symbol: " << dupes.GetKey(i) << ", ";
		err << "other occurrences at:\n";
		for (int j = 1; j < list.GetCount(); j++)
			err << "\t\t" << list[j].ToString() << "\n";
		throw ER::Duplicate(list[0], err);
	}

	return false;
}


bool ZCompiler::CheckForDuplicates(ZNamespace& ns) {
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ZMethodBundle& d = ns.Methods.GetAdd(f.Name, ZMethodBundle(ns));
		
		f.GenerateSignatures();
		
		bool valid = true;
			
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& g = *d.Functions[j];
			if (g.DupSig() == f.DupSig()) {
				int index = dupes.Find(f.Name);
				if (index == -1) {
					Vector<ZSourcePos>& list = dupes.GetAdd(f.Name);
					list.Add(g.DefPos);
					list.Add(f.DefPos);
				}
				else {
					Vector<ZSourcePos>& list = dupes.GetAdd(f.Name);
					list.Add(f.DefPos);
				}
					
				valid = false;
			}
		}
		
		if (valid)
			d.Functions.Add(&f);
		f.DefPos.Source->Functions.Add(&f);
	}
	
	for (int i = 0; i < ns.PreVariables.GetCount(); i++) {
		ZVariable& f = ns.PreVariables[i];
		
		bool valid = true;
		int index = dupes.Find(f.Name);
		if (index != -1) {
			Vector<ZSourcePos>& list = dupes.GetAdd(f.Name);
			list.Add(f.DefPos);
			valid = false;
		}
		
		if (valid) {
			index = ns.Variables.Find(f.Name);
			if (index != -1) {
				Vector<ZSourcePos>& list = dupes.GetAdd(f.Name);
				list.Add(f.DefPos);
				list.Add(ns.Variables[index]->DefPos);
				valid = false;
			}
			
			if (valid)
				ns.Variables.Add(f.Name, &f);
		}
	}
	
	return true;
}

ZCompiler::ZCompiler(Assembly& aAss): ass(aAss), irg(ass) {
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
	static String name = "Z2CR 0.1.0 (pre-alpha)";
	return name;
}
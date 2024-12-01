#include <z2crlib/ZCompiler.h>
#include <z2crlib/ZTranspiler.h>
#include <z2crlib/ZResolver.h>

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

bool ZCompiler::Compile() {
	irg.FoldConstants = FoldConstants;
	
	if (!ScanSources())
		return false;
	
	ZResolver resolver(ass);
	if (!resolver.Resolve())
		return false;
	
	MainFound = FindMain();
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		auto& ns = ass.Namespaces[i];
		PreCompileVars(ns);
		
		for (int j = 0; j < ns.Classes.GetCount(); j++) {
			auto& c = *ns.Classes[j];
			PreCompileVars(c);
		}
	}
	
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

bool ZCompiler::Transpile() {
	CppPath = AppendFileName(BuildPath, "out.cpp");
	FileOut out(CppPath);
	ZTranspiler cpp(*this, out);
	cpp.CppVersion = CppVersion;
	//cpp.PrintDebug = true;
	
	cpp.WriteIntro();
	cpp.WriteClassForward();
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i], 0b11, true);
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i], 0b100, false);
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
	src.AddStdClassRefs();
			
	auto& scanner = scanners.Add(ZScanner(src, Platform));
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
		CompileVar(*v);
	}
	
	return true;
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
	
	for (int j = 0; j < ns.Classes.GetCount(); j++) {
		Compile(*ns.Classes[j]);
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
		Node* defRet = ZExprParser::Temporary(ass, irg, *f.Return.Tt.Class, dummy/*, Point(-1, -1)*/);
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
		return CompileLocalVar(f, parser, false);
	else if (parser.Id("const"))
		return CompileLocalVar(f, parser, true);
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
				parser.Error(pp.P, "can't assign to readonly " + ass.ToQtColor(node) + " instance");
			
			if (!node->CanAssign(ass, rs)) {
				parser.Error(pp.P, "can't assign " + ass.ToQtColor(rs) + " instance to '" + ass.ToQtColor(node) + " instance without a cast");
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
		parser.Error(p, ER::Blue + "if" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
	
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
		parser.Error(p, ER::Blue + "while" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
		
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
		parser.Error(p, ER::Blue + "do" + ER::White + ".." + ER::Blue + "while" + ER::White + " condition must be of class " + ass.ToQtColor(ass.CBool) + ", " + ass.ToQtColor(node) + " found");
		
	return irg.dowhilecond(node, bd);
}

bool ZCompiler::CompileVar(ZVariable& v) {
	ZParser parser(v.DefPos);
	parser.ExpectZId();
	
	return compileVarDec(v, parser, v.DefPos, nullptr);
}

Node *ZCompiler::CompileLocalVar(ZFunction& f, ZParser& parser, bool aConst) {
	auto vp = parser.GetFullPos();
	
	String name = parser.ExpectZId();
	TestVarDup(/*cls, */f, name, vp);
	
	ZVariable& v = f.Locals.Add(ZVariable(f.Namespace()));
	v.Name = name;
	v.BackName = name;
	v.DefPos = vp;
	v.Section = f.Section;
	v.IsConst = aConst;
	
	return compileVarDec(v, parser, vp, &f);
}

Node *ZCompiler::compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, ZFunction* f) {
	ZClass* cls = nullptr;
	if (parser.Char(':')) {
		cls = ZExprParser::ParseType(ass, parser);
		if (cls == ass.CVoid)
			parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ass.CVoid));
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
				parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ass.CVoid));
			v.I.Tt = node->Tt;
		}
		
		if (v.I.Tt.Class == ass.CClass)
			parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ass.CClass));
			
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
		
		if (v.I.Tt.Class == ass.CClass)
			parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ass.CClass));
		if (v.I.Tt.Class == ass.CVoid)
			parser.Error(vp.P, "can't create a variable of type " + ass.ToQtColor(ass.CVoid));
		
		if (v.Value == nullptr) {
			Vector<Node*> params;
			v.Value = ZExprParser::Temporary(ass, irg, *v.I.Tt.Class, params/*, Point(-1, -1)*/);
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
			parser.Error(p, "can't assign " + ass.ToQtColor(retVal) +
				" instance to " + ass.ToQtColor(f.Return.Tt.Class) + " instance without a cast");
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
	static String name = "Z2CR 0.1.3 (pre-alpha)";
	return name;
}
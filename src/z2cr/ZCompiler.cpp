#include "z2cr.h"

bool ZCompiler::Compile() {
	Vector<ZScanner*> scanners;
	// scan all source files, throws on critical syntax error
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		auto scanner = new ZScanner(*ass.SourceLookup[i], Platform);
		scanner->Scan();
		
		scanners << scanner;
	}
	
	bool found = false;
	// print non critical errors
	for (int i = 0; i < scanners.GetCount(); i++) {
		for (int j = 0; j < scanners[i]->Errors.GetCount(); j++) {
			Cout() << scanners[i]->Errors[j].ToString() << "\n";
			found = true;
		}
		delete scanners[i];
	}
	
	if (found)
		return false;
	
	if (!CheckForDuplicates())
		return false;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		if (ns.Name == "::")
			continue;
		
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
			}
		}
		
		ns.BackName = backName;
	}
	
	if (mainPath.GetCount()) {
		ZSource* src = ass.FindSource(mainPath);
		if (!src)
			return false;
		
		auto vf = FindMain(*src);
		if (vf.GetCount() == 0)
			return false;
		else if (vf.GetCount() > 1) {
			String err;
			
			err << "multiple '@main' function: other candidates found at: " << "\n";
			for (int i = 1; i < vf.GetCount(); i++)
				err << "\t\t" << vf[i]->DefPos.ToString() << "\n";
			auto exc = ErrorReporter::Duplicate(vf[0]->DefPos, err);
			Cout() << exc.ToString() << "\n";
			
			return false;
		}
		
		MainFunction = vf[0];
	}
	else
		return false;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		PreCompileVars(ass.Namespaces[i]);
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
	
	OutPath = AppendFileName(BuildPath, "out.cpp");
	FileOut out(OutPath);
	ZTranspiler cpp(*this, out);
	
	cpp.WriteIntro();
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDeclarations(ass.Namespaces[i]);
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		cpp.TranspileDefinitions(ass.Namespaces[i]);
	cpp.WriteOutro();
	
	return true;
}

Vector<ZFunction*> ZCompiler::FindMain(ZSource& src) {
	Vector<ZFunction*> mains;
	for (int i = 0; i < src.Functions.GetCount(); i++) {
		if (src.Functions[i]->Name == "@main")
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

bool ZCompiler::Compile(ZNamespace& ns) {
	for (int i = 0; i < ns.Definitions.GetCount(); i++) {
		ZDefinition& d = ns.Definitions[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			Compile(f);
		}
	}
	
	return true;
}

bool ZCompiler::Compile(ZFunction& f) {
	ZParser parser(f.BodyPos);
	
	parser.Expect('{');
	
	while (!parser.IsChar('}')) {
		ZExprParser ep(parser, irg);
		Node* node = ep.Parse();
		
		f.AddNode(node);
		
		parser.ExpectEndStat();
	}
	
	parser.Expect('}');
	
	return true;
}

bool ZCompiler::CompileVar(ZVariable& v) {
	ZParser parser(v.DefPos);
	
	parser.ExpectId();
	
	if (parser.Char(':')) {
		parser.ExpectId();
		
		if (parser.Char('=')) {
			ZExprParser ep(parser, irg);
			Node* node = ep.Parse();
			
			v.Value = node;
			v.Tt = node->Tt;
		}
	}
	else {
		parser.Expect('=');
		
		ZExprParser ep(parser, irg);
		Node* node = ep.Parse();
		
		v.Value = node;
		v.Tt = node->Tt;
	}
	
	return true;
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
		ErrorReporter::Duplicate(list[0], err);
	}

	return false;
}


bool ZCompiler::CheckForDuplicates(ZNamespace& ns) {
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ZDefinition& d = ns.Definitions.GetAdd(f.Name, ZDefinition(ns));
		
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
	static String name = "Z2R 0.0.2";
	return name;
}
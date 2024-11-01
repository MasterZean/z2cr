#include "z2cr.h"

bool ZCompiler::Compile() {
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++) {
		ZScanner scanner(*ass.SourceLookup[i], true);
		scanner.Scan();
	}
	
	if (!CheckForDuplicates())
		return false;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		if (ns.Name == "::")
			continue;
		
		auto names = Split(ns.Name, '.', true);
		
		String backName;
		for (int j = 0; j < names.GetCount(); j++) {
			backName << names[j];
			backName << "::";
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
			ErrorReporter::Duplicate(vf[0]->DefPos, err);
			
			return false;
		}
		
		MainFunction = vf[0];
	}
	else
		return false;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
	
	ZTranspiler cpp(*this, Cout());
	
	cpp.WriteIntro();
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Transpile(cpp, ass.Namespaces[i]);
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

bool ZCompiler::Transpile(ZTranspiler& cpp, ZNamespace& ns){
	for (int i = 0; i < ns.Definitions.GetCount(); i++) {
		ZDefinition& d = ns.Definitions[i];
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			
			Transpile(cpp, f);
		}
	}
	
	return true;
}

bool ZCompiler::Transpile(ZTranspiler& cpp, ZFunction& f) {
	cpp.WriteFunctionDecl(f);
	
	cpp.WriteFunctionBody(f.Nodes);
	
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
		ZDefinition& d = ns.Definitions.GetAdd(f.Name);
		
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
		
		int index = dupes.Find(f.Name);
		if (index != -1) {
			Vector<ZSourcePos>& list = dupes.GetAdd(f.Name);
			list.Add(f.DefPos);
		}
	}
	
	return true;
}

String& ZCompiler::GetName() {
	static String name = "Z2R 0.0.2";
	return name;
}
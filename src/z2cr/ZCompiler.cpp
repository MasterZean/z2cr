#include "z2cr.h"

bool ZCompiler::Compile() {
	dupes.Clear();
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Traverse(ass.Namespaces[i]);
	
	if (dupes.GetCount()) {
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
	}
	else
		return false;
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
	
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
		ZExprParser ep(parser);
		ep.Parse();
		
		parser.ExpectEndStat();
	}
	
	parser.Expect('}');
	
	return true;
}

bool ZCompiler::Traverse(ZNamespace& ns) {
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
	static String name = "Z2R 0.0.1";
	return name;
}
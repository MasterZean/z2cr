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
		
		ZFunction* f = FindMain(*src);
		if (!f)
			return false;
	}
	else
		return false;
}

ZFunction* ZCompiler::FindMain(ZSource& src) {
	for (int i = 0; i < src.Functions.GetCount(); i++) {
		if (src.Functions[i]->Name == "@main")
			return src.Functions[i];
	}
	
	return nullptr;
}

bool ZCompiler::Traverse(ZNamespace& ns) {
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ZDefinition& d = ns.Definitions.GetAdd(f.Name);
		
		f.GenerateSignatures();
		
		bool valid = true;
			
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& g = d.Functions[j];
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
			d.Functions.Add(f);
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
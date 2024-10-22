#include "z2cr.h"

void ZCompiler::Compile() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		Compile(ass.Namespaces[i]);
}

void ZCompiler::Compile(ZNamespace& ns) {
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ZDefinition& d = ns.Definitions.GetAdd(f.Name);
		
		f.GenerateSignatures();
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& g = d.Functions[j];
			if (g.DupSig() == f.DupSig()) {
				Cout() << "Duplicate in ns: " << ns.Name <<	"\n\t" <<
					g.DefPos.Source->Path() << "(" << g.DefPos.P.x << ", " << g.DefPos.P.y << "): " << g.DupSig() << "\n\t" <<
					f.DefPos.Source->Path() << "(" << f.DefPos.P.x << ", " << f.DefPos.P.y << "): " << f.DupSig() << "\n";
			}
		}
		
		d.Functions.Add(f);
	}
}

String& ZCompiler::GetName() {
	static String name = "Z2R 0.0.1";
	return name;
}
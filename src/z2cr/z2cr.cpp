#include "z2cr.h"

class ZCompiler {
public:
	ZCompiler(Assembly& aAss): ass(aAss) {
	}
	
	void Compile();
	
private:
	Assembly& ass;
	
	void Compile(ZNamespace& ns);
};

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

CONSOLE_APP_MAIN {
	String path = "c:\\temp\\test.z2";
	
	Assembly ass;
	
	try {
		ZPackage& mainPak = ass.AddPackage("main", "");
		ZSource& source = mainPak.AddSource(path);
		
		Scanner scanner(source, true);
		scanner.Scan();
		
		ZCompiler compiler(ass);
		compiler.Compile();
	}
	catch (ZException e) {
		Cout() << e.ToString() << "\n";
	}
}

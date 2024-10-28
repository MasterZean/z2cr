#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

class ZTranspiler;

class ZCompiler {
public:
	ZCompiler(Assembly& aAss): ass(aAss), irg(ass) {
	}
	
	bool Compile();
		
	void SetMainFile(const String& aPath) {
		mainPath = aPath;
	}
	
	static String& GetName();
	
private:
	Assembly& ass;
	String mainPath;
	IR irg;
	ArrayMap<String, Vector<ZSourcePos>> dupes;
	
	Vector<ZFunction*> FindMain(ZSource& src);
	
	bool Compile(ZNamespace& ns);
	bool Compile(ZFunction& f);
	
	bool Transpile(ZTranspiler& cpp, ZNamespace& ns);
	bool Transpile(ZTranspiler& cpp, ZFunction& f);
	
	bool Traverse(ZNamespace& ns);
};

#endif

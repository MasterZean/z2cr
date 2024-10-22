#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

class ZCompiler {
public:
	ZCompiler(Assembly& aAss): ass(aAss) {
	}
	
	bool Compile();
		
	void SetMainFile(const String& aPath) {
		mainPath = aPath;
	}
	
	static String& GetName();
	
private:
	Assembly& ass;
	String mainPath;
	ArrayMap<String, Vector<ZSourcePos>> dupes;
	
	ZFunction* FindMain(ZSource& src);
	
	bool Traverse(ZNamespace& ns);
};

#endif

#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

class ZTranspiler;

class ZCompiler {
public:
	String BuildPath;
	String BuildProfile;
	String OutPath;
	
	PlatformType Platform;
	String PlatformString;
	String PlatformSysLib;
	
	ZFunction* MainFunction = nullptr;
	
	ZCompiler(Assembly& aAss);
	
	bool Compile();
		
	void SetMainFile(const String& aPath) {
		mainPath = aPath;
	}
	
	Assembly& GetAssembly() const {
		return ass;
	}
	
	static String& GetName();
	
private:
	Assembly& ass;
	String mainPath;
	IR irg;
	ArrayMap<String, Vector<ZSourcePos>> dupes;
	
	Vector<ZFunction*> FindMain(ZSource& src);
	
	bool PreCompileVars(ZNamespace& ns);
	
	bool Compile(ZNamespace& ns);
	bool Compile(ZFunction& f, Node& target);
	Node* CompileBlock(ZFunction& f, ZParser& parser);
	bool CompileVar(ZVariable& v);
	
	bool Transpile(ZTranspiler& cpp, ZNamespace& ns);
	bool Transpile(ZTranspiler& cpp, ZFunction& f);
	
	bool CheckForDuplicates();
	bool CheckForDuplicates(ZNamespace& ns);
};

#endif

#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

#include <z2crlib/entities.h>
#include <z2crlib/ZScanner.h>

class ZTranspiler;

class ZCompiler {
public:
	String BuildPath;
	String BuildProfile;
	bool   BuildMode = false;
	String OutPath;
	String CppPath;
	
	PlatformType Platform;
	String PlatformString;
	String PlatformSysLib;
	
	ZFunction* MainFunction = nullptr;
	bool MainFound = false;
	
	bool FoldConstants = false;
	
	ZCompiler(Assembly& aAss);
	
	bool Compile();
	bool Transpile();
	bool ScanSources();
	
	void ResolveNamespaces();
	void ResolveNamespace(ZNamespace& ns);
		
	void SetMain(const String& aClass, const String& aFile) {
		mainClass = aClass;
		mainFile = aFile;
	}
	
	Assembly& Ass() const {
		return ass;
	}
	
	static String& GetName();
	
private:
	Assembly& ass;
	String mainClass;
	String mainFile;
	IR irg;
	ArrayMap<String, Vector<ZSourcePos>> dupes;
	
	Vector<ZFunction*> FindMain(ZSource& src);
	
	bool PreCompileVars(ZNamespace& ns);
	
	bool Compile(ZNamespace& ns);
	bool CompileFunc(ZFunction& f, Node& target);
	bool CompileVar(ZVariable& v);
		
	Node* CompileBlock(ZFunction& f, ZParser& parser);
	Node* CompileStatement(ZFunction& f, ZParser& parser);
	Node* CompileIf(ZFunction& f, ZParser& parser);
	Node* CompileWhile(ZFunction& f, ZParser& parser);
	Node* CompileDoWhile(ZFunction& f, ZParser& parser);
	Node* CompileLocalVar(ZFunction& f, ZParser& parser);
	
	bool Transpile(ZTranspiler& cpp, ZNamespace& ns);
	bool Transpile(ZTranspiler& cpp, ZFunction& f);
	
	bool CheckForDuplicates();
	bool CheckForDuplicates(ZNamespace& ns);
	bool FindMain();
	
	void ScanSource(ZSource& src, Array<ZScanner>& scanners);
	void TestVarDup(/*ZClass& cls,*/ ZFunction& over, const String& name, const ZSourcePos& cur);
	
	Node* compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, ZFunction* f);
};

#endif

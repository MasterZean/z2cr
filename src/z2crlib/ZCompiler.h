#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

#include <z2crlib/entities.h>
#include <z2crlib/Assembly.h>
#include <z2crlib/ZScanner.h>
#include <z2crlib/ZIR.h>
#include <z2crlib/ZExprParser.h>

class ZTranspiler;

class ZContext {
public:
	bool Return = false;
	bool InLoop = false;
};

class ZCompiler {
public:
	String BuildPath;
	String BuildProfile;
	bool   BuildMode = false;
	String OutPath;
	String CppPath;
	int CppVersion = 2017;
	Index<String> LibLink;
	
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
	
	void SetMain(const String& aClass, const String& aFile) {
		mainClass = aClass;
		mainFile = aFile;
	}
	
	Assembly& Ass() {
		return ass;
	}
	
	Assembly& Ass() const {
		return ass;
	}
	
	IR& IRG() {
		return irg;
	}
	
	const IR& IRG() const {
		return irg;
	}
	
	static String& GetName();
	
	ZClass& ResolveInstance(ZClass& cc, ZClass& sub, Point p, bool eval);
	
private:
	Assembly& ass;
	String mainClass;
	String mainFile;
	IR irg;
	
	ZClass* Class = nullptr;
	Vector<ZFunction*> CBinds;

	Vector<ZFunction*> FindMain(ZSource& src);
	
	bool PreCompileVars(ZNamespace& ns);
	
	bool Compile(ZNamespace& ns);
	bool CompileFunc(ZFunction& f, Node& target);
	bool CompileVar(ZVariable& v);
		
	Node* CompileBlock(ZFunction& f, ZParser& parser, ZContext& con);
	Node* CompileStatement(ZFunction& f, ZParser& parser, ZContext& con);
	Node *CompileExpression(ZFunction& f, ZParser& parser, ZContext& con);
	
	Node* CompileIf(ZFunction& f, ZParser& parser, ZContext& con);
	Node* CompileWhile(ZFunction& f, ZParser& parser, ZContext& con);
	Node* CompileDoWhile(ZFunction& f, ZParser& parser, ZContext& con);
	Node *CompileFor(ZFunction& f, ZParser& parser, ZContext& con);

	Node* CompileLocalVar(ZFunction& f, ZParser& parser, bool aConst);
	Node *CompileReturn(ZFunction& f, ZParser& parser, ZContext& con);
	
	bool Transpile(ZTranspiler& cpp, ZNamespace& ns);
	bool Transpile(ZTranspiler& cpp, ZFunction& f);
	
	bool FindMain();
	
	void ScanSource(ZSource& src, Array<ZScanner>& scanners);
	void TestVarDup(ZClass* cls, ZFunction& over, const String& name, const ZSourcePos& cur);
	
	Node* compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, ZFunction* f);
};

#endif

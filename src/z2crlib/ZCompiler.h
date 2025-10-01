#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

#include <z2crlib/entities.h>
#include <z2crlib/Assembly.h>
#include <z2crlib/ZScanner.h>
#include <z2crlib/ZIR.h>
#include <z2crlib/ZExprParser.h>

class ZTranspiler;
class ZResolver;

class ZBlockContext {
public:
	bool Return = false;
	bool InLoop = false;
};

class ZCompilerContext: Moveable<ZCompilerContext> {
public:
	ZClass* Class = nullptr;
	ZFunction* Func = nullptr;
	
	ZVariable* TargetVar = nullptr;
	
	ZClass* InstCls = nullptr;
	
	ZSourcePos Pos;
};

class ZCompiler {
public:
	String BuildPath;
	String BuildProfile;
	bool   BuildMode = false;
	String OutPath;
	String CppPath;
	String MCUPath;
	int CppVersion = 2017;
	bool MCU = false;
	
	PlatformType Platform;
	String PlatformString;
	String PlatformSysLib;
	
	Vector<String> MCUPaths;
	Vector<ZFunction*> LLVMInput;
	Vector<ZFunction*> ExtraFunctions;
	
	ZFunction* MainFunction = nullptr;
	bool MainFound = false;
	
	bool FoldConstants = false;
	
	ZCompiler(Assembly& aAss);
	
	bool Compile(bool exc = false);
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
	
	ZClass& ResolveInstance(ZClass& cc, ZClass& sub, const ZSourcePos& p, bool eval);
	
	bool CompileVar(ZVariable& v, const ZCompilerContext& zcon);
	
	bool DoMCU();
	void DoMCU(ZNamespace& ns);
	
	bool CompileFunc(ZFunction& f) {
		return CompileFunc(f, f.Nodes);
	}
			
	bool PreCompileVars(ZNamespace& ns);
	
	void Push(const ZSourcePos& pos, ZClass& cls);
	void Pop();
	
	void SetInUse(ZNamespace& cls);
	void SetInUse(ZClass& cls);
	
private:
	Assembly& ass;
	String mainClass;
	String mainFile;
	IR irg;
	ZResolver* resPtr = nullptr;
	
	int cuCounter = 0;
	Index<ZClass*> cuClasses;
	
	ZClass* Class = nullptr;
	Vector<ZFunction*> CBinds;
	Vector<ZClass*> tempInstances;
	
	Vector<ZCompilerContext> stack;

	Vector<ZFunction*> FindMain(ZSource& src);
	
	ZTrait lastTrait;
	bool useLastTrait = false;
	
	bool compile();
	
	bool Compile(ZNamespace& ns);
	bool CompileFunc(ZFunction& f, Node& target);
		
	Node* CompileBlock(ZFunction& f, ZParser& parser, ZBlockContext& con);
	Node* CompileStatement(ZFunction& f, ZParser& parser, ZBlockContext& con);
	Node *CompileExpression(ZFunction& f, ZParser& parser, ZBlockContext& con);
	
	Node* CompileIf(ZFunction& f, ZParser& parser, ZBlockContext& con);
	Node* CompileWhile(ZFunction& f, ZParser& parser, ZBlockContext& con);
	Node* CompileDoWhile(ZFunction& f, ZParser& parser, ZBlockContext& con);
	Node *CompileFor(ZFunction& f, ZParser& parser, ZBlockContext& con);

	Node* CompileLocalVar(ZFunction& f, ZParser& parser, bool aConst, bool useLastTrait);
	Node *CompileReturn(ZFunction& f, ZParser& parser, ZBlockContext& con);
	
	bool Transpile(ZTranspiler& cpp, ZNamespace& ns);
	bool Transpile(ZTranspiler& cpp, ZFunction& f);
	
	bool FindMain();
	
	void ScanSource(ZSource& src, Array<ZScanner>& scanners);
	void TestVarDup(ZClass* cls, ZFunction& over, const String& name, const ZSourcePos& cur);
	
	Node* compileVarDec(ZVariable& v, ZParser& parser, ZSourcePos& vp, const ZCompilerContext& zcon);
	
	void WriteDeps(ZClass& cls);
	void writeDeps(ZClass& cls);
};

#endif

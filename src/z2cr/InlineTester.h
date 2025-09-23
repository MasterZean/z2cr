#ifndef _z2cr_InlineTester__h_
#define _z2cr_InlineTester__h_

#include <z2crlib/ZCompiler.h>

class ZTest: public Moveable<ZTest> {
public:
	String Name;
	String Error = String::GetVoid();
	VectorMap<int, String> Dumps;
	String DumpNsPubName = String::GetVoid();
	String DumpNsPubCon = String::GetVoid();
	String DumpNsPrivCon = String::GetVoid();
	String DumpNsDef = String::GetVoid();
	String GlobalVarDef = String::GetVoid();
	WithDeepCopy<Vector<String>> Files;
	bool Passed = false;
	bool StdLib = false;
	int Line = 0;
	
	Assembly Ass;
	ZPackage* MainPak = nullptr;
	ZSource* Source = nullptr;
	
	bool Run(ZSourceCache* cache = nullptr);
	
	bool RunDumpNsPub(ZCompiler& compiler);
	
	bool DumpEqual(const String& have, const String& want, const String& desc);
};
	
class InlineTester {
public:
	Array<ZTest> Tests;
	int TestCount = 0;
	int PassCount = 0;
	ZSourceCache Cache;
	
	InlineTester();
	
	void AddTestFolder(const String& path, int parent = 0);
	void AddTestCollection(const String& path);
};

#endif

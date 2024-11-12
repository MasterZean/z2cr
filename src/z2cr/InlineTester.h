#ifndef _z2cr_InlineTester__h_
#define _z2cr_InlineTester__h_

#include <z2crlib/ZCompiler.h>

class ZTest: public Moveable<ZTest> {
public:
	String Name;
	String Error;
	String Con;
	String Dump;
	String GlobalVarDef;
	WithDeepCopy<Vector<String>> Files;
	bool Passed = false;
	int Line = 0;
	
	Assembly Ass;
	ZPackage* MainPak = nullptr;
	ZSource* Source = nullptr;
	
	bool Run();
};
	
class InlineTester {
public:
	Array<ZTest> Tests;
	int TestCount = 0;
	int PassCount = 0;
	
	InlineTester();
	
	void AddTestFolder(const String& path, int parent = 0);
	void AddTestCollection(const String& path);
};

#endif

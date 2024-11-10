#ifndef _z2cr_InlineTester__h_
#define _z2cr_InlineTester__h_

#include <z2crlib/ZCompiler.h>

class ZTest: public Moveable<ZTest> {
public:
	String Name;
	WithDeepCopy<Vector<String>> Files;
	bool Passed = false;
	
	Assembly Ass;
	
	void Run();
};
	
class InlineTester {
public:
	Array<ZTest> Tests;
	int TestCount = 0;
	
	InlineTester();
	
	void AddModule(const String& path, int parent = 0);
	void AddTest(const String& path);
};

#endif

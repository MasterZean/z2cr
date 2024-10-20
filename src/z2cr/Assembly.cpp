#include "z2cr.h"

void ZFunction::GenerateSignatures() {
	dsig = "";
	
	//if (InClass == false)
	//	dsig << "global ";
	
	if (IsFunction)
		dsig << "func ";
	else
		dsig << "def ";
	
	dsig << Name << "()";
}

ZFunction& ZNamespace::PrepareFunction(const String& aName) {
	ZFunction& f = PreFunctions.Add(ZFunction(*this));
	f.Name = aName;
	return f;
}

Assembly::Assembly() {
	Namespaces.FindAdd("");
}

ZNamespace& Assembly::FindAddNamespace(const String& aName) {
	
	int index = Namespaces.FindAdd(aName);
	ASSERT(index != -1);
	Namespaces[index].Name = aName;
	
	return Namespaces[index];
}

ZPackage& Assembly::AddPackage(const String& aName, const String& aPath) {
	int index = packages.Find(aName);
	
	ASSERT(index == -1);
	
	ZPackage& pak = packages.Add(aName, ZPackage(*this, aName, aPath));
	return pak;
}


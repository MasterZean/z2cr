#include "z2cr.h"

String ZSourcePos::ToString() const {
	String str;
	
	if (Source)
		str << Source->Path();
	str << "(" << P.x << ", " << P.y << ")";
	
	return str;
}
	
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
	int index = Packages.Find(aName);
	
	ASSERT(index == -1);
	
	ZPackage& pak = Packages.Add(aName, ZPackage(*this, aName, aPath));
	return pak;
}

ZSource* Assembly::FindSource(const String& aName) {
	for (int i = 0; i < Packages.GetCount(); i++) {
		ZPackage& pak = Packages[i];
		int index = pak.Sources.Find(aName);
		
		if (index != -1)
			return &pak.Sources[index];
	}
	
	return nullptr;
}



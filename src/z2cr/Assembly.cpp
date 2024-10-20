#include "z2cr.h"

ZFunction& ZNamespace::PrepareFunction(const String& aName) {
	ZFunction& f = preFunctions.Add(ZFunction(*this));
	f.Name = aName;
	return f;
}

ZNamespace& Assembly::FindAddNamespace(const String& aName) {
	
	int index = Namespaces.FindAdd(aName);
	ASSERT(index != -1);
	
	return Namespaces[index];
}


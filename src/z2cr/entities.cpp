#include <z2cr/entities.h>

ZNamespaceItem* ZNamespaceItem::Add(const String& aName) {
	ZNamespaceItem& newns = Names.GetAdd(aName);
	
	return &newns;
}

bool ZNamespace::HasMember(const String& aName) {
	int index = Methods.Find(aName);
	if (index != -1)
		return true;
	index = Variables.Find(aName);
	if (index != -1)
		return true;
	
	return false;
}


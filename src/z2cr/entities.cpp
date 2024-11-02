#include <z2cr/entities.h>

ZNamespaceItem* ZNamespaceItem::Add(const String& aName) {
	ZNamespaceItem& newns = Names.GetAdd(aName);
	
	return &newns;
}
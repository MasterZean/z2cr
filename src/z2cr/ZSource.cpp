#include "z2cr.h"

bool ZSource::LoadFile(const String& aPath) {
	String fileContent = Upp::LoadFile(aPath);
	if (fileContent.IsVoid())
		return false;
	
	content = fileContent;
	path = aPath;
	
	return true;
}

bool ZSource::LoadVirtual(const String& aContent, const String& aPath) {
	content = aContent;
	path = aPath;
	
	return true;
}


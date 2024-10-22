#include "z2cr.h"

bool ZSource::LoadFile(const String& aPath) {
	String fileContent = Upp::LoadFile(aPath);
	if (fileContent.IsVoid())
		return false;
	
	content = fileContent;
	path = aPath;
	
	return true;
}

bool ZSource::LoadVirtual(const String& aPath, const String& aContent) {
	content = aContent;
	path = aPath;
	
	return true;
}

ZSource& ZPackage::AddSource(const String& aPath) {
	int index = Sources.Find(aPath);
	
	ASSERT(index == -1);
	
	ZSource& source = Sources.Add(aPath, ZSource(*this));
	if (source.LoadFile(aPath) == false)
		ErrorReporter::CantOpenFile(aPath);
	
	return source;
}

ZSource& ZPackage::AddSource(const String& aPath, const String& aContent) {
	int index = Sources.Find(aPath);
	
	ASSERT(index == -1);
	
	ZSource& source = Sources.Add(aPath, ZSource(*this));
	source.LoadVirtual(aPath, aContent);
	
	return source;
}


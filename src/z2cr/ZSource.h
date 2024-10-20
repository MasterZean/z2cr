#ifndef _z2cr_ZSource_h_
#define _z2cr_ZSource_h_

class ZSource {
public:
	ZSource(ZPackage& aPak): pak(aPak) {
	}
	
	bool LoadFile(const String& aPath);
	bool LoadVirtual(const String& aPath, const String& aContent);
	
	bool IsValid() const {
		return !content.IsVoid();
	}
	
	const String& Content() const {
		return content;
	}
	
	const String& Path() const {
		return path;
	}
	
	ZPackage& Package() {
		return pak;
	}
	
private:
	ZPackage& pak;
	
	String content;
	String path;
};

class ZPackage {
public:
	String Name;
	String Path;
	
	ZPackage(Assembly& aAss, const String& aName, const String& aPath): ass(aAss), Name(aName), Path(aPath) {
	}
	
	ZSource& AddSource(const String& aPath);
	ZSource& AddSource(const String& aPath, const String& aContent);
	
	Assembly& GetAssembly() {
		return ass;
	}
	
private:
	Assembly& ass;
	ArrayMap<String, ZSource> sources;
};

#endif

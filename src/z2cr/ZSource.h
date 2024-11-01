#ifndef _z2cr_ZSource_h_
#define _z2cr_ZSource_h_

class ZSource {
public:
	String Path;
	Time Modified;
	
	Vector<ZFunction*> Functions;
	Vector<String> References;
	Vector<Point> ReferencePos;
	
	bool IsScaned = false;
	bool IsScanError = false;
	bool IsLoaded = false;
	
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
	
	ZPackage& Package() {
		return pak;
	}
	
	const ZPackage& Package() const {
		return pak;
	}
		
	bool LoadFile();
	bool LoadVirtual(const String& aContent);
	
	void AddStdClassRefs();
	
	void AddReference(const String& ns);
	
private:
	ZPackage& pak;
	
	String content;
};

class ZPackage {
public:
	String Name;
	String Path;
	String CachePath;
	
	ArrayMap<String, ZSource> Sources;
	
	ZPackage(Assembly& aAss, const String& aName, const String& aPath): ass(aAss), Name(aName), Path(aPath) {
	}
	
	ZSource& AddSource(const String& aPath, bool aLoadFile);
	
	Assembly& GetAssembly() {
		return ass;
	}
	
private:
	Assembly& ass;
};

#endif

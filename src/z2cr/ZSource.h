#ifndef _z2cr_ZSource_h_
#define _z2cr_ZSource_h_

class ZSource {
public:
	ZSource(Assembly& aAss): ass(aAss) {
	}
	
	bool LoadFile(const String& aPath);
	bool LoadVirtual(const String& aContent, const String& aPath);
	
	bool IsValid() const {
		return !content.IsVoid();
	}
	
	const String& Content() const {
		return content;
	}
	
	const String& Path() const {
		return path;
	}
	
	Assembly& GetAssembly() {
		return ass;
	}
	
	const Assembly& GetAssembly() const {
		return ass;
	}
	
private:
	Assembly& ass;
	
	String content;
	String path;
};

#endif

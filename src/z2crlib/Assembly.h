#ifndef _z2cr_Assembly_h_
#define _z2cr_Assembly_h_

#include <z2crlib/entities.h>

class ZNamespace;

class Assembly {
public:
	ArrayMap<String, ZNamespace> Namespaces;
	ArrayMap<String, ZPackage> Packages;
	ArrayMap<String, ZSource*> SourceLookup;
	ZNamespaceItem NsLookup;
	
	ZClass* CCls = nullptr;
	ZClass* CDef = nullptr;
	ZClass* CVoid = nullptr;
	ZClass* CNull = nullptr;
	ZClass* CInt = nullptr;
	ZClass* CDWord = nullptr;
	ZClass* CBool = nullptr;
	ZClass* CByte = nullptr;
	ZClass* CSmall = nullptr;
	ZClass* CShort = nullptr;
	ZClass* CWord = nullptr;
	ZClass* CFloat = nullptr;
	ZClass* CDouble = nullptr;
	ZClass* CChar = nullptr;
	ZClass* CLong = nullptr;
	ZClass* CQWord = nullptr;
	ZClass* CPtrSize = nullptr;
	ZClass* CPtr = nullptr;
	ZClass* CString = nullptr;
	ZClass* CRaw = nullptr;
	ZClass* CStream = nullptr;
	ZClass* CIntrinsic = nullptr;
	ZClass* CSlice = nullptr;
	ZClass* CVect = nullptr;

	ArrayMap<String, ZClass> Classes;
	
	Assembly();
	
	ZNamespace& FindAddNamespace(const String& aName);
	
	ZNamespace& DefaultNamespace() {
		return Namespaces[0];
	}
	
	ZNamespace& CoreNamespace() {
		return Namespaces[1];
	}
	
	ZNamespace& LangNamespace() {
		return Namespaces[2];
	}
	
	ZPackage& AddPackage(const String& aName, const String& aPath);
	bool LoadPackage(const String& aPath, bool fullBuild = false);
	ZPackage* FindPackage(const String& aName);
	
	ZSource* FindSource(const String& aName);
	
	void AddBuiltInClasses();
	
	bool IsNumeric(const ObjectType& type) const {
		return type.Class->MIsNumeric;
	}
	
	bool IsInteger(const ObjectType& type) const {
		return type.Class->MIsInteger;
	}

	bool IsPtr(const ObjectType& ot) {
		return ot.Class == CPtr;
	}
	
	bool IsSignedInt(ObjectType* type) const;
	bool IsSignedInt(const ObjectType& type) const;
	bool IsFloat(ObjectType* type) const;
	bool IsFloat(const ObjectType& type) const;
	bool IsUnsignedInt(ObjectType* type) const;
	bool IsUnsignedInt(const ObjectType& type) const;
	
	String ClassToString(ObjectInfo* type, bool qual = true);
	
private:
	void AddModule(int parent, const String& path, ZPackage& pak);
	ZSource& AddModuleSource(ZPackage& aPackage, const String& aFile, bool aLoadFile);

	ZClass* AddCoreType(ZNamespace& ns, const String& name, const String& backendName, bool num = false, bool integer = false, bool core = true);
};

#endif

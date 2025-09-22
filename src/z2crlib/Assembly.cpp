#include "z2cr.h"
#include "BuildMethod.h"

String ZSourcePos::ToString() const {
	String str;
	
	if (Source)
		str << Source->Path;
	str << "(" << P.x << ", " << P.y << ")";
	
	return str;
}

Assembly::Assembly() {
	FindAddNamespace("::");
	FindAddNamespace("sys.core.");
	FindAddNamespace("sys.core.lang.");
	
	AddBuiltInClasses();
}

ZNamespace& Assembly::FindAddNamespace(const String& aName) {
	int index = Namespaces.FindAdd(aName, ZNamespace(*this));
	ASSERT(index != -1);
	auto& ns = Namespaces[index];
	ns.Name = aName;
	if (ns.Name != "::")
		ns.ProperName = aName.Mid(0, aName.GetCount() - 1);
	else
		ns.ProperName = ns.Name;
	
	ns.NameElements = Split(ns.Name, '.', true);
	
	return Namespaces[index];
}

ZPackage& Assembly::AddPackage(const String& aName, const String& aPath) {
	int index = Packages.Find(aName);
	
	ASSERT(index == -1);
	
	ZPackage& pak = Packages.Add(aName, ZPackage(*this, aName, aPath));
	return pak;
}

bool Assembly::LoadPackage(const String& aPath, bool fullBuild) {
	String pakPath = NativePath(NormalizePath(aPath));
	String pakName = GetFileName(pakPath);
	if (pakName.GetCount() == 0)
		pakName = GetFileName(GetFileFolder(pakPath));
	
	FindFile ff;
	
	if (!ff.Search(pakPath)) {
		Cout() << "Package '" << pakName << "' could not be found: path '" << pakPath << "' does not exist.\n";
		Cout() << "Exiting!\n";
		return false;
	}
	
	if (!ff.IsFolder()) {
		Cout() << "Could not find package '" << pakName << "' in folder '" << pakPath << "'.\n";
		Cout() << "Exiting!\n";
		return false;
	}
		
	int pakIndex = Packages.Find(pakName);
	if (pakIndex != -1) {
		ZPackage& existingPak = Packages[pakIndex];
		
		if (existingPak.Path != pakPath) {
			Cout() << "When attempting to load package: '" << pakPath << "':\n";
			Cout() << "\ta package with the same name is already referenced at '" << existingPak.Path << "'.\n";
			Cout() << "Exiting!\n";
			return false;
		}
	}
	else
		pakIndex = Packages.FindAdd(pakName, ZPackage(*this, pakName, pakPath));
	
	ZPackage& package = Packages[pakIndex];
	package.Name = pakName;
	package.Path = pakPath;//AppendFileName(pakPath, "");

	/*if (fullBuild) {
		package.CachePath = NativePath(BuildPath + "\\" + package.Name);
		DirectoryCreate(package.CachePath);
		
		ZPackage temp;
		LoadFromFile(temp, NativePath(package.CachePath + "\\cache.dat"));
		
		AddModule(0, package.Path, package, temp);
	}
	else*/
	{
		package.CachePath = NativePath(BuildPath + "\\" + package.Name);
		DirectoryCreate(package.CachePath);
		
		ZPackage temp(*this, "temp", "");
		LoadFromFile(temp, NativePath(package.CachePath + "\\cache.dat"));
		
		AddModule(0, package.Path, package, temp);
	}
	
	return true;
}

void Assembly::AddModule(int parent, const String& path, ZPackage& pak) {
	FindFile ff;
	ff.Search(path + "/*");

	while (ff) {
		if (ff.IsFile()) {
			String name = ff.GetName();
			if (GetFileExt(name) == ".z2") {
				String fp = ff.GetPath();
				ZSource& zs = AddModuleSource(pak, fp, true);
			}
		}
		else if (ff.IsFolder())
			AddModule(parent + 1, ff.GetPath(), pak);

		ff.Next();
	}
}

void Assembly::AddModule(int parent, const String& path, ZPackage& pak, ZPackage& temp) {
	FindFile ff;
	ff.Search(path + "/*");

	while (ff) {
		if (ff.IsFile()) {
			String name = ff.GetName();
			if (GetFileExt(name) == ".z2") {
				String fp = ff.GetPath();
				ZSource& zs = AddModuleSource(pak, fp, temp, true);
			}
		}
		else if (ff.IsFolder())
			AddModule(parent + 1, ff.GetPath(), pak, temp);

		ff.Next();
	}
}

ZSource& Assembly::AddModuleSource(ZPackage& aPackage, const String& aFile, bool aLoadFile) {
	String relPath = aFile.Mid(aPackage.Path.GetLength());

	/*ZSource& source = aPackage.Sources[fileIndex];
	source.Path = relPath;*/
	ZSource& source = aPackage.AddSource(aFile, aLoadFile);
	source.Modified = FileGetTime(aFile);
	
	return source;//LoadSource(source, populate);
}

ZSource& Assembly::AddModuleSource(ZPackage& pak, const String& aFile, ZPackage& temp, bool aLoadFile) {
	//String fileName = aFile.Mid(pak.Path.GetLength());
	int i = temp.Sources.Find(aFile);
	
	if (i != -1) {
		Time t = FileGetTime(aFile);

		if (t.Compare(temp.Sources[i].Modified) > 0) {
			// "Overwriting file
			//ZSource& zs = AddSource(pak, filePath);
			FilesChanged++;
			//Cout() << "Adding changed file: " << aFile << "\n";
			
			ZSource& source = pak.AddSource(aFile, aLoadFile);
			source.Modified = FileGetTime(aFile);
			return source;
		}
		else {
			// Skipping file
			/*ZSource& srcIn = temp.Sources[i];
			ZSource& srcSkip = pak.Sources.Add(fileName);
			srcSkip.Path = fileName;
			srcSkip.Modified = srcIn.Modified;*/
			//srcSkip.Package = &pak;
			//srcSkip.ClassNameList = clone(srcIn.ClassNameList);
			
			/*for (int i = 0; i < srcSkip.ClassNameList.GetCount(); i++) {
				LookUp.Add(srcSkip.ClassNameList[i], filePath);
				
				int dd = srcSkip.ClassNameList[i].ReverseFind('.');
				String name = srcSkip.ClassNameList[i].Mid(dd + 1);
				ass.AddClassCount(name);
			}*/
			FilesUnchanged++;
			//Cout() << "Adding unchanged file: " << aFile << "\n";
			
			ZSource& source = pak.AddSource(aFile, aLoadFile);
			source.Modified = FileGetTime(aFile);
			return source;
		}
	}
	else {
		// "Adding file
		//ZSource& zs = AddSource(pak, filePath);
		FilesNew++;
		//Cout() << "Adding new file: " << aFile << "\n";
		ZSource& source = pak.AddSource(aFile, aLoadFile);
		source.Modified = FileGetTime(aFile);
		return source;
	}
}

ZPackage* Assembly::FindPackage(const String& aName) {
	for (int i = 0; i < Packages.GetCount(); i++) {
		if (Packages[i].Name == aName)
			return &Packages[i];
	}
	
	return nullptr;
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

void Assembly::WriteCache() {
	for (int i = 0; i < Packages.GetCount(); i++) {
		ZPackage& pak = Packages[i];
		
		StoreToFile(pak, NativePath(pak.CachePath + "\\cache.dat"));
	}
}

void Assembly::AddBuiltInClasses() {
	ASSERT(Classes.GetCount() == 0);

	CClass      = AddCoreType(CoreNamespace(), "Class", "Class", false, false, false);
	CDef      = AddCoreType(LangNamespace(), "Def", "Def");
	CVoid     = AddCoreType(CoreNamespace(), "Void", "void");
	CNull     = AddCoreType(CoreNamespace(), "Null", "");
	CBool     = AddCoreType(LangNamespace(), "Bool",     "bool"   ,  true, false);
	CSmall    = AddCoreType(LangNamespace(), "Small",    "int8"   ,  true, true);
	CByte     = AddCoreType(LangNamespace(), "Byte" ,    "uint8"  ,  true, true);
	CShort    = AddCoreType(LangNamespace(), "Short",    "int16"  ,  true, true);
	CWord     = AddCoreType(LangNamespace(), "Word" ,    "uint16" ,  true, true);
	CInt      = AddCoreType(LangNamespace(), "Int"  ,    "int32"  ,  true, true);
	CDWord    = AddCoreType(LangNamespace(), "DWord",    "uint32" ,  true, true);
	CLong     = AddCoreType(LangNamespace(), "Long" ,    "int64"  ,  true, true);
	CQWord    = AddCoreType(LangNamespace(), "QWord",    "uint64" ,  true, true);
	CFloat    = AddCoreType(LangNamespace(), "Float",    "float"  ,  true, false);
	CDouble   = AddCoreType(LangNamespace(), "Double",   "double" ,  true, false);
	CChar     = AddCoreType(LangNamespace(), "Char",     "uint32" ,  true, false);
	CChar->ParamType = CDWord;
	CPtrSize  = AddCoreType(LangNamespace(), "PtrSize",  "size_t",   true, true);
	// TODO: why?
	//CPtrSize->ParamType = CDWord;
	CStream   = AddCoreType(CoreNamespace(),      "Stream",   "Stream",   false, false, false);

	CPtr = AddCoreType(LangNamespace(), "Ptr",      "Ptr");
	CPtr->IsTemplate = true;
	CString  = AddCoreType(LangNamespace(), "String", "String", false, false, false);
	CRaw = AddCoreType(LangNamespace(), "CArray", "", false, false, true);
	CRaw->IsTemplate = true;
	
	CVect     = AddCoreType(LangNamespace(), "Vector", "Vector", false, false, false);

	CSlice    = AddCoreType(LangNamespace(), "Slice", "Slice", false, false, false);
	CSlice->IsTemplate = true;
	
	CIntrinsic = AddCoreType(LangNamespace(),      "Intrinsic",   "Intrinsic",   false, false, false);

	for (int i = 0; i < Classes.GetCount(); i++)
		Classes[i].Pt = GetPtr(&Classes[i].Tt);
}

ZClass* Assembly::AddCoreType(ZNamespace& ns, const String& name, const String& backendName, bool num, bool integer, bool core) {
	ASSERT(ns.Name.EndsWith("."));
	
	int type = Classes.GetCount();
	
	// TODO: option?
	//LOG(String().Cat() << "Adding core class " << ns.Name << name << " Index " << type);
	ZClass& typeCls = Classes.Add(ns.Name + name, ZClass(ns));
		
	//typeCls.Scan.Namespace = ns;
	typeCls.MIsNumeric = num;
	typeCls.MIsInteger = integer;
	typeCls.Name = name;
	typeCls.BackName = name;
	typeCls.StorageName = backendName;
	//typeCls.MContName = name;
	typeCls.CoreSimple = core;
	typeCls.IsDefined = false;
	typeCls.Index = type;
	typeCls.ParamType = &typeCls;
	typeCls.Tt.Class = &typeCls;
	typeCls.IsClass = true;
	
	typeCls.RTTIIndex = type;
	
	ns.Classes.FindAdd(name, &typeCls);

	return &typeCls;
}

ZClass& Assembly::AddClass(ZClass& cls) {
	int type = Classes.GetCount();
	
	String name = cls.Namespace().Name + cls.Name;
	
	int index = Classes.Find(name);

	if (index != -1) {
		ZClass& exCls = Classes[index];
		
		exCls.CopyPreSection(cls);
		exCls.Scan = cls.Scan;
		exCls.SuperPos = cls.SuperPos;
		
		return exCls;
	}
	else {
		// TODO: option?
		//LOG(String().Cat() << "Adding class " << name);
	}

	ZClass& typeCls = (index != -1) ? Classes[index]: Classes.Add(cls.Namespace().Name + cls.Name, cls);
	
	if (typeCls.StorageName.GetCount() == 0)
		typeCls.StorageName = cls.BackName;
	typeCls.Scan = cls.Scan;
	typeCls.ParamType = &typeCls;
	typeCls.Index = type;
	typeCls.ParamType = &typeCls;
	typeCls.Tt.Class = &typeCls;
	typeCls.DefPos = cls.DefPos;
	typeCls.IsClass = cls.IsClass;
	typeCls.Pt = GetPtr(&typeCls.Tt);
	typeCls.SuperPos = cls.SuperPos;
	typeCls.IsStatic = cls.IsStatic;
	typeCls.RTTIIndex = type;
	
	return typeCls;
}

String Assembly::ClassToString(const ObjectType& type) {
	ASSERT(type.Class);
	return type.Class->Name;
}

String Assembly::ClassToString(ObjectInfo* type, bool qual) {
	String s;
	
	/*if (type->Tt.Class->MIsRawVec) {
		s << "CArray<" << type->Tt.Class->T->Scan.Name;
		if (type->Tt.Param != -1)
			s << ", " << type->Tt.Param;
		s << ">";
	}
	else {
		int count = 0;
		if (type->Tt.Class->Scan.Name == "org.z2legacy.ut.container.vector.WithPrivateConstructor03")
			type->Tt.Class->Scan.Name == "";
		int ii = ClassCounts.Find(type->Tt.Class->Scan.Name);
		if (ii != -1)
			count = ClassCounts[ii];
		
		if (count > 1)
			s << type->Tt.Class->Scan.Namespace;
		s << type->Tt.Class->Scan.Name;
		if (qual && type->Tt.Next) {
			s << '<' << type->Tt.Next->Class->Scan.Name;
			if (type->Tt.Param)
				s << ", " << IntStr(type->Tt.Param);
			s << '>';
		}
	}*/
	
	s << type->Tt.Class->Name;
	
	return s;
}

String Assembly::TypeToString(ObjectType& type) {
	String s;
	
	if (type.Next) {
		s << type.Class->Name;
		s << '<';
		s << TypeToString(*type.Next);
		s << '>';
	}
	else {
		s << type.Class->Name;
	}
	
	return s;
}

String Assembly::TypeToColor(ObjectType& type) {
	String s;
	
	if (type.Class->FromTemplate) {
		s << ER::Cyan << type.Class->TBase->Name << ER::White;
		s << '<';
		s << TypeToColor(type.Class->T->Tt);
		
		if (type.Class->TBase == CRaw && type.Param != -1)
			s << ", " << type.Param;
		
		s << '>';
	}
	else if (type.Next) {
		s << ER::Cyan << type.Class->Name << ER::White;
		s << '<';
		s << TypeToColor(*type.Next);
		s << '>';
	}
	else {
		s << ER::Cyan << type.Class->Name << ER::White;
	}
	
	return s;
}

String Assembly::ToQtColor(ObjectType* type) {
	String s = "'";
	s << TypeToColor(*type);
	s << ER::White << "'";
	
	/*s << ER::Green << type->Class->Name;
	if (type->Class == CPtr) {
		s << ER::White << "<";
		s << ER::Green << type->Next->Class->Name;
		s << ER::White << ">";
	}
	s << ER::White << "'";*/
	
	return s;
}

String Assembly::ToQtColor(ZClass* cls) {
	String s = "'";
	s << ER::Green << cls->Name;
	s << ER::White << "'";
	return s;
}
	
bool Assembly::IsSignedInt(const ObjectType& type) const {
	return type.Class == CInt || type.Class == CSmall || type.Class == CShort || type.Class == CLong;
}

bool Assembly::IsUnsignedInt(const ObjectType& type) const {
	return type.Class == CDWord || type.Class == CByte || type.Class == CWord || type.Class == CQWord;
}

bool Assembly::IsFloat(const ObjectType& type) const {
	return type.Class == CFloat || type.Class == CDouble;
}

bool Assembly::AddStdlibPakcages(const String& path) {
#ifdef PLATFORM_WIN32
		String platform = "WIN32";
		String platformLib = "microsoft.windows";
#endif

			
#ifdef PLATFORM_POSIX
		String platform = "POSIX";
		String platformLib = "ieee.posix";
#endif

	String stdLibPath = path + NativePath("source\\stdlib\\");
	
	if (!LoadPackage(stdLibPath + "bind.c")) {
		SetExitCode(BuildMethod::ErrorCode(-1));
		return false;
	}
	
	if (!LoadPackage(stdLibPath + "sys.core")) {
		SetExitCode(BuildMethod::ErrorCode(-1));
		return false;
	}
	
	if (!LoadPackage(stdLibPath + platformLib)) {
		SetExitCode(BuildMethod::ErrorCode(-1));
		return false;
	}
	
	return true;
}


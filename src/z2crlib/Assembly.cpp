#include "z2cr.h"

String ZSourcePos::ToString() const {
	String str;
	
	if (Source)
		str << Source->Path;
	str << "(" << P.x << ", " << P.y << ")";
	
	return str;
}
	
void ZFunction::GenerateSignatures() {
	ZParser parser(ParamPos);
	
	parser.Expect('(');
	
	while (!parser.IsChar(')')) {
		auto pp = parser.GetFullPos();
		String name = parser.ExpectId();
		
		if (name == Name)
			throw ER::Duplicate(name, pp, DefPos);
		parser.Expect(':');
		
		ZClass* cls = ZExprParser::ParseType(Ass(), parser);
		
		if (parser.Char(',')) {
			if (parser.IsChar(')'))
				parser.Error(parser.GetPoint(), "identifier expected, " + parser.Identify() + " found");
		}
		
		ZVariable& var = Params.Add(ZVariable(Namespace()));
		var.Name = name;
		var.I.Tt = cls->Tt;
		var.DefPos = pp;
	}
	
	dsig = "";
	
	if (InClass == false) {
		dsig << "global ";
		dsig << "func ";
	}
	else {
		if (IsFunction)
			dsig << "func ";
		else
			dsig << "def ";
	}
	
	dsig << Name << "(";
	
	for (int i = 0; i < Params.GetCount(); i++) {
		ZVariable& var = Params[i];
		
		if (i > 0)
			dsig << ", ";
		
		dsig << Ass().ClassToString(&var.I);
	}
	
	dsig << ")";
	
	DUMP(dsig);
}

ZFunction& ZNamespace::PrepareFunction(const String& aName) {
	ZFunction& f = PreFunctions.Add(ZFunction(*this));
	f.Name = aName;
	return f;
}

ZVariable& ZNamespace::PrepareVariable(const String& aName) {
	ZVariable& f = PreVariables.Add(ZVariable(*this));
	f.Name = aName;
	return f;
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
	Namespaces[index].Name = aName;
	
	return Namespaces[index];
}

ZPackage& Assembly::AddPackage(const String& aName, const String& aPath) {
	int index = Packages.Find(aName);
	
	ASSERT(index == -1);
	
	ZPackage& pak = Packages.Add(aName, ZPackage(*this, aName, aPath));
	return pak;
}

bool Assembly::LoadPackage(const String& aPath, bool fullBuild) {
	String pakName = GetFileName(NativePath(NormalizePath(aPath)));
	if (pakName.GetCount() == 0)
		pakName = GetFileName(GetFileFolder(aPath));
	
	
	FindFile ff(aPath);
	
	if (!ff.IsFolder()) {
		Cout() << "Could not find package '" << pakName << "' in folder '" << aPath << "'.\n";
		Cout() << "Exiting!\n";
		return false;
	}
		
	int pakIndex = Packages.Find(pakName);
	if (pakIndex != -1) {
		ZPackage& existingPak = Packages[pakIndex];
		
		if (existingPak.Path != aPath) {
			Cout() << "When attempting to load package: '" << aPath << "':\n";
			Cout() << "\ta package with the same name is already referenced at '" << existingPak.Path << "'.\n";
			Cout() << "Exiting!\n";
			return false;
		}
	}
	else
		pakIndex = Packages.FindAdd(pakName, ZPackage(*this, pakName, aPath));
	
	ZPackage& package = Packages[pakIndex];
	package.Name = pakName;
	package.Path = AppendFileName(aPath, "");

	/*if (fullBuild) {
		package.CachePath = NativePath(BuildPath + "\\" + package.Name);
		DirectoryCreate(package.CachePath);
		
		ZPackage temp;
		LoadFromFile(temp, NativePath(package.CachePath + "\\cache.dat"));
		
		AddModule(0, package.Path, package, temp);
	}
	else*/
		AddModule(0, package.Path, package);
	
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

ZSource& Assembly::AddModuleSource(ZPackage& aPackage, const String& aFile, bool aLoadFile) {
	String relPath = aFile.Mid(aPackage.Path.GetLength());

	/*ZSource& source = aPackage.Sources[fileIndex];
	source.Path = relPath;*/
	ZSource& source = aPackage.AddSource(aFile, aLoadFile);
	source.Modified = FileGetTime(aFile);
	
	return source;//LoadSource(source, populate);
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

void Assembly::AddBuiltInClasses() {
	ASSERT(Classes.GetCount() == 0);

	CCls      = AddCoreType(CoreNamespace(), "Class", "Class", false, false, false);
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
	CPtrSize->ParamType = CDWord;
	CStream   = AddCoreType(CoreNamespace(),      "Stream",   "Stream",   false, false, false);

	CPtr = AddCoreType(LangNamespace(), "Ptr",      "Ptr");
	CPtr->Scan.IsTemplate = true;
	CString  = AddCoreType(LangNamespace(), "String", "String", false, false, false);
	CRaw = AddCoreType(LangNamespace(), "CArray", "", false, false, true);
	CRaw->Scan.IsTemplate = true;
	CRaw->MIsRawVec = true;
	
	CVect     = AddCoreType(LangNamespace(), "Vector", "Vector", false, false, false);

	CSlice    = AddCoreType(LangNamespace(), "Slice", "Slice", false, false, false);
	CSlice->Scan.IsTemplate = true;
	
	CIntrinsic = AddCoreType(LangNamespace(),      "Intrinsic",   "Intrinsic",   false, false, false);

	//for (int i = 0; i < Classes.GetCount(); i++)
	//	Classes[i].Pt = GetPtr(&Classes[i].Tt);
}

ZClass* Assembly::AddCoreType(ZNamespace& ns, const String& name, const String& backendName, bool num, bool integer, bool core) {
	ASSERT(ns.Name.EndsWith("."));
	
	int type = Classes.GetCount();
	ZClass& typeCls = Classes.Add(ns.Name + name, ZClass(ns));
		
	//typeCls.Scan.Namespace = ns;
	typeCls.MIsNumeric = num;
	typeCls.MIsInteger = integer;
	typeCls.Name = name;
	typeCls.BackName = backendName;
	//typeCls.MContName = name;
	typeCls.CoreSimple = core;
	typeCls.IsDefined = false;
	typeCls.Index = type;
	typeCls.ParamType = &typeCls;
	typeCls.Tt.Class = &typeCls;
	
	typeCls.RTTIIndex = type;
	
	ns.Classes.FindAdd(name, &typeCls);

	return &typeCls;
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

bool Assembly::IsSignedInt(ObjectType* type) const {
	return type->Class == CInt || type->Class == CSmall || type->Class == CShort || type->Class == CLong;
}

bool Assembly::IsSignedInt(const ObjectType& type) const {
	return type.Class == CInt || type.Class == CSmall || type.Class == CShort || type.Class == CLong;
}

bool Assembly::IsUnsignedInt(ObjectType* type) const {
	return type->Class == CDWord || type->Class == CByte || type->Class == CWord || type->Class == CQWord;
}

bool Assembly::IsUnsignedInt(const ObjectType& type) const {
	return type.Class == CDWord || type.Class == CByte || type.Class == CWord || type.Class == CQWord;
}

bool Assembly::IsFloat(ObjectType* type) const {
	return type->Class == CFloat || type->Class == CDouble;
}

bool Assembly::IsFloat(const ObjectType& type) const {
	return type.Class == CFloat || type.Class == CDouble;
}
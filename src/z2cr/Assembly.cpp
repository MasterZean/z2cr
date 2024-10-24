#include "z2cr.h"

String ZSourcePos::ToString() const {
	String str;
	
	if (Source)
		str << Source->Path();
	str << "(" << P.x << ", " << P.y << ")";
	
	return str;
}
	
void ZFunction::GenerateSignatures() {
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
	
	dsig << Name << "()";
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
	
	int index = Namespaces.FindAdd(aName);
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
	//typeCls.Tt.Class = &typeCls;

	return &typeCls;
}



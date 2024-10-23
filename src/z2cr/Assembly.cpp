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
	Namespaces.FindAdd("::");
	
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

	CCls      = AddCoreType("sys.core.", "Class", "Class", false, false, false);
	CDef      = AddCoreType("sys.core.lang.", "Def", "Def");
	CVoid     = AddCoreType("sys.core.", "Void", "void");
	CNull     = AddCoreType("sys.core.", "Null", "");
	CBool     = AddCoreType("sys.core.lang.", "Bool",     "bool"   ,  true, false);
	CSmall    = AddCoreType("sys.core.lang.", "Small",    "int8"   ,  true, true);
	CByte     = AddCoreType("sys.core.lang.", "Byte" ,    "uint8"  ,  true, true);
	CShort    = AddCoreType("sys.core.lang.", "Short",    "int16"  ,  true, true);
	CWord     = AddCoreType("sys.core.lang.", "Word" ,    "uint16" ,  true, true);
	CInt      = AddCoreType("sys.core.lang.", "Int"  ,    "int32"  ,  true, true);
	CDWord    = AddCoreType("sys.core.lang.", "DWord",    "uint32" ,  true, true);
	CLong     = AddCoreType("sys.core.lang.", "Long" ,    "int64"  ,  true, true);
	CQWord    = AddCoreType("sys.core.lang.", "QWord",    "uint64" ,  true, true);
	CFloat    = AddCoreType("sys.core.lang.", "Float",    "float"  ,  true, false);
	CDouble   = AddCoreType("sys.core.lang.", "Double",   "double" ,  true, false);
	CChar     = AddCoreType("sys.core.lang.", "Char",     "uint32" ,  true, false);
	CChar->ParamType = CDWord;
	CPtrSize  = AddCoreType("sys.core.lang.", "PtrSize",  "size_t",   true, true);
	CPtrSize->ParamType = CDWord;
	CStream   = AddCoreType("sys.core.",      "Stream",   "Stream",   false, false, false);

	CPtr = AddCoreType("sys.core.lang.", "Ptr",      "Ptr");
	CPtr->Scan.IsTemplate = true;
	CString  = AddCoreType("sys.core.lang.", "String", "String", false, false, false);
	CRaw = AddCoreType("sys.core.lang.", "CArray", "", false, false, true);
	CRaw->Scan.IsTemplate = true;
	CRaw->MIsRawVec = true;
	
	CVect     = AddCoreType("sys.core.lang.", "Vector", "Vector", false, false, false);

	CSlice    = AddCoreType("sys.core.lang.", "Slice", "Slice", false, false, false);
	CSlice->Scan.IsTemplate = true;
	
	CIntrinsic = AddCoreType("sys.core.lang.",      "Intrinsic",   "Intrinsic",   false, false, false);

	//for (int i = 0; i < Classes.GetCount(); i++)
	//	Classes[i].Pt = GetPtr(&Classes[i].Tt);
}

ZClass* Assembly::AddCoreType(const String& ns, const String& name, const String& backendName, bool num, bool integer, bool core) {
	ASSERT(ns.EndsWith("."));
	int type = Classes.GetCount();
	ZClass& typeCls = Classes.Add(ns + name);
	typeCls.Scan.Namespace = ns;
	typeCls.MIsNumeric = num;
	typeCls.MIsInteger = integer;
	typeCls.Scan.Name = name;
	typeCls.BackendName = backendName;
	//typeCls.MContName = name;
	typeCls.CoreSimple = core;
	typeCls.IsDefined = false;
	typeCls.Index = type;
	typeCls.ParamType = &typeCls;
	//typeCls.Tt.Class = &typeCls;

	return &typeCls;
}



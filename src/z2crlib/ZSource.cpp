#include "z2cr.h"

String s[] = {
	"sys.core.lang.Def",
	"sys.core.lang.Ptr",
	"sys.core.lang.System",
	
	"sys.core.lang.Bool",
	
	"sys.core.lang.Small",
	"sys.core.lang.Byte",
	"sys.core.lang.Short",
	"sys.core.lang.Word",
	"sys.core.lang.Int",
	"sys.core.lang.DWord",
	"sys.core.lang.Long",
	"sys.core.lang.QWord",
	
	"sys.core.lang.Float",
	"sys.core.lang.Double",
	
	"sys.core.lang.PtrSize",
	"sys.core.lang.Char",
	
	"sys.core.lang.String",
	
	"sys.core.lang.CArray",
	"sys.core.lang.Vector",
	"sys.core.lang.Index",

	"sys.core.lang.Intrinsic",
	"sys.core.lang.Slice",
	"sys.core.lang.ReadSlice",
};

void ZSource::AddStdClassRefs() {
	for (int i = 0; i < __countof(s); i++)
		AddReference(s[i], Point(1, 1));
}

void ZSource::AddReference(const String& ns, Point pt) {
	int dotPos = ns.ReverseFind('.');
	ASSERT(dotPos != -1);
	
	int i = FindIndex(References, ns);
	if (i == -1) {
		//i = pak.Ass().Classes.Find(ns);
		//if (i != -1) {
			References.Add(ns);
			ReferencePos.Add();
			
			//auto shortName = ns.Mid(dotPos + 1);
			
			//ShortNameLookup.FindAdd(shortName, &pak.Ass().Classes[i]);
		//}
	}
}

void ZSource::AlignReferences() {
	for (int k = 0; k < References.GetCount(); k++) {
		const String& ns = References[k];
		int dotPos = ns.ReverseFind('.');
		
		int i = pak.Ass().Classes.Find(ns);
		if (i != -1) {
			auto shortName = ns.Mid(dotPos + 1);
			
			ShortNameLookup.FindAdd(shortName, &pak.Ass().Classes[i]);
		}
	}
	
	for (int k = 0; k < AliasMap.GetCount(); k++) {
		const String& key = AliasMap.GetKey(k);
		
		int i = pak.Ass().Classes.Find(AliasMap[k]);
		if (i != -1) {
			ShortNameLookup.FindAdd(key, &pak.Ass().Classes[i]);
		}
	}
}

void ZSource::AddAlias(const String& alias, const String& ref) {
	AliasMap.FindAdd(alias, ref);
}

bool ZSource::LoadFile() {
	String fileContent = Upp::LoadFile(Path);
	if (fileContent.IsVoid())
		return false;
	
	content = fileContent;
	IsLoaded = true;
	
	return true;
}

bool ZSource::LoadVirtual(const String& aContent) {
	content = aContent;
	IsLoaded = true;
	
	return true;
}

ZSource& ZPackage::AddSource(const String& aPath, bool aLoadFile) {
	int index = Sources.Find(aPath);
	
	ASSERT(index == -1);
	
	ZSource& source = Sources.Add(aPath, ZSource(*this));
	source.Path = aPath;
	
	ass.SourceLookup.FindAdd(aPath, &source);
	
	if (aLoadFile) {
		//Cout() << "Loaded source file: " << source.Path << "\n";
		source.LoadFile();
	}
	
	return source;
}


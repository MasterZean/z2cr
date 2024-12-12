#ifndef _z2crlib_ZResolver_h_
#define _z2crlib_ZResolver_h_

#include <z2crlib/ZCompiler.h>

class ZResolver {
public:
	Index<String> LibLink;
	
	ZResolver(ZCompiler& aComp): comp(aComp), ass(comp.Ass()) {
	}
	
	bool Resolve();
	void ResolveNamespaces();
	void ResolveNamespace(ZNamespace& ns);
	void ResolveClasses();
	void ResolveNamespaceMembers(ZNamespace& ns);
	void ResolveFunctions();
	void ResolveFunction(ZNamespace& ns, ZFunction& f);
	void ResolveVariables();
	
	bool CheckForDuplicates();
	bool CheckForDuplicates(ZNamespace& ns);
	void DuplicateLoop(ZNamespace& ns, bool aPrivate);
	
private:
	ZCompiler& comp;
	Assembly& ass;
	
	Vector<String> dupeFile;
	Vector<String> dupeDecl;
	Vector<String> dupeOwner;
	
	String DupStr(const String& allErrors, const ZSourcePos& dp, const String& colorSig, const String& owner);
	
	void AssignClassRoles(ZClass& cls, ZFunction& f);
};

#endif

#include <z2crlib/ZResolver.h>
#include <z2crlib/ZSource.h>
#include <z2crlib/ZScanner.h>

bool ZResolver::Resolve() {
	ResolveNamespaces();
	ResolveClasses();
	ResolveFunctions();
	ResolveVariables();
	
	return CheckForDuplicates();
}

void ZResolver::ResolveNamespaces() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		ResolveNamespace(ns);
	}
}

void ZResolver::ResolveNamespace(ZNamespace& ns) {
	auto names = Split(ns.Name, '.', true);
	
	// build namespace item hierachy
	ZNamespaceItem* nsp = &ass.NsLookup;
	String backName;
	for (int i = 0; i < names.GetCount(); i++) {
		backName << names[i];
		nsp = nsp->Add(names[i]);
		if (i < names.GetCount() - 1)
			backName << "::";
		else {
			ASSERT(nsp->Namespace == nullptr);
			nsp->Namespace = &ns;
			
			ns.NamespaceItem = nsp;
		}
	}
	
	ns.BackName = backName;
	
	// TODO: prevent other clashes with main_
	if (ns.BackName == "main")
		ns.BackName = "main_";
	
	// setup sections for using clause
	for (int i = 0; i < ns.Sections.GetCount(); i++) {
		ZNamespaceSection& sec = ns.Sections[i];
		
		for (int k = 0; k < sec.UsingNames.GetCount(); k++) {
			ZNamespace* usn = ass.Namespaces.FindPtr(sec.UsingNames[k]);
			if (usn)
				sec.Using.Add(usn);
		}
	}
	
	ns.IsResolved = true;
}

void ZResolver::ResolveClasses() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		
		for (int j = 0; j < ns.PreClasses.GetCount(); j++) {
			ZClass& c = ns.PreClasses[j];
			c.GenerateSignatures();
		}
	}
}

void ZResolver::ResolveFunctions() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		ResolveNamespaceMembers(ns);
				
		for (int j = 0; j < ns.PreClasses.GetCount(); j++) {
			ZClass& c = ns.PreClasses[j];
			ResolveNamespaceMembers(c);
		}
	}
}

void ZResolver::ResolveNamespaceMembers(ZNamespace& ns) {
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ResolveFunction(f);
	}
	
	for (int i = 0; i < ns.PreVariables.GetCount(); i++) {
		ZVariable& f = ns.PreVariables[i];
		f.GenerateSignatures();
	}
}

void ZResolver::ResolveFunction(ZFunction& f) {
	f.GenerateSignatures();
	f.DefPos.Source->Functions.Add(&f);

	ZMethodBundle& d = f.Namespace().Methods.GetAdd(f.Name, ZMethodBundle(f.Namespace()));
	if (d.Name.GetCount() == 0)
		d.Name = f.Name;
	d.Functions.Add(&f);
}

void ZResolver::ResolveVariables() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
			ZVariable& f = ns.PreVariables[j];
			ns.Variables.Add(f.Name, &f);
		}
	}
}

bool ZResolver::CheckForDuplicates() {
	dupeListing = "";
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		CheckForDuplicates(ass.Namespaces[i]);
	
	if (dupeListing.GetCount() == 0)
		return true;
		
	throw ZException("", dupeListing);

	return false;
}


bool ZResolver::CheckForDuplicates(ZNamespace& ns) {
	// verify class duplication
	VectorMap<String, int> dupes;
	for (int i = 0; i < ns.PreClasses.GetCount(); i++) {
		ZClass& c = ns.PreClasses[i];
		int& count = dupes.GetAdd(c.Name, 0);
		count++;
	}
	
	for (int k = 0; k < dupes.GetCount(); k++) {
		if (dupes[k] > 1) {
			String allErrors;
			
			for (int i = 0; i < ns.PreClasses.GetCount(); i++) {
				ZClass& c = ns.PreClasses[i];
				
				if (c.Name == dupes.GetKey(k))
					allErrors << DupStr(allErrors, c.DefPos, c.ColorSig());
			}
			
			dupeListing << allErrors;
		}
	}
	
	// verify members
	DuplicateLoop(ns, true);
	
	// verify class members
	for (int j = 0; j < ns.PreClasses.GetCount(); j++) {
		ZClass& c = ns.PreClasses[j];
		DuplicateLoop(c, true);
	}
	
	return true;
}

void ZResolver::DuplicateLoop(ZNamespace& ns, bool aPrivate) {
	for (int i = 0; i < ns.Methods.GetCount(); i++) {
		ZMethodBundle& d = ns.Methods[i];
		
		VectorMap<String, int> dupes;
		
		for (int j = 0; j < d.Functions.GetCount(); j++) {
			ZFunction& f = *d.Functions[j];
			int& count = dupes.GetAdd(f.DupSig(), 0);
			count++;
		}
		
		for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
			ZVariable& var = ns.PreVariables[j];
			if (var.Name == d.Name) {
				for (int k = 0; k < dupes.GetCount(); k++)
					dupes[k]++;
				var.IsDuplicate = true;
			}
		}
		
		for (int k = 0; k < dupes.GetCount(); k++) {
			if (dupes[k] > 1) {
				String allErrors;
				
				ZFunction* lastFunc = nullptr;
				
				for (int j = 0; j < d.Functions.GetCount(); j++) {
					ZFunction& f = *d.Functions[j];
					lastFunc = &f;
					
					if (f.DupSig() == dupes.GetKey(k))
						allErrors << DupStr(allErrors, f.DefPos, f.ColorSig());
					
				}
				
				if (lastFunc) {
					for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
						ZVariable& v = ns.PreVariables[j];
						if (v.Name == lastFunc->Name)
							allErrors << DupStr(allErrors, v.DefPos, v.ColorSig());
					}
				}
				
				dupeListing << allErrors;
			}
		}
	}
	
	VectorMap<String, int> dupes;
	
	for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
		ZVariable& var = ns.PreVariables[j];
		if (var.IsDuplicate == false) {
			int& count = dupes.GetAdd(var.Name, 0);
			count++;
		}
	}
	
	for (int k = 0; k < dupes.GetCount(); k++) {
		if (dupes[k] > 1) {
			String allErrors;
			
			for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
				ZVariable& v = ns.PreVariables[j];
				if (v.IsDuplicate == false && v.Name == dupes.GetKey(k))
					allErrors << DupStr(allErrors, v.DefPos, v.ColorSig());
			}
			
			dupeListing << allErrors;
		}
	}
}

String ZResolver::DupStr(const String& allErrors, const ZSourcePos& dp, const String& colorSig) {
	String err;
	
	// append to error
	if (allErrors.GetCount() != 0) {
		err << "\t" << ER::Gray << dp.ToString() << ": " << colorSig << "\n";
		return err;
	}
	
	// first message
	err << ER::Gray << dp.ToString() << ": ";
	err << ER::Red << "error: ";
	err << ER::White << "duplicate symbol: " << colorSig;
	err << "\n" << ER::Gray << "other occurrences at:\n";
	
	return err;
}
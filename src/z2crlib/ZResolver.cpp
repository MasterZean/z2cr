#include <z2crlib/ZResolver.h>
#include <z2crlib/ZSource.h>
#include <z2crlib/ZScanner.h>

bool ZResolver::Resolve() {
	ResolveNamespaces();
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
	
	ZNamespaceItem* nsp = &ass.NsLookup;
	String backName;
	for (int j = 0; j < names.GetCount(); j++) {
		backName << names[j];
		nsp = nsp->Add(names[j]);
		if (j < names.GetCount() - 1)
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
	
	for (int j = 0; j < ns.Sections.GetCount(); j++) {
		ZNamespaceSection& sec = ns.Sections[j];
		
		for (int k = 0; k < sec.UsingNames.GetCount(); k++) {
			ZNamespace* usn = ass.Namespaces.FindPtr(sec.UsingNames[k]);
			if (usn)
				sec.Using.Add(usn);
		}
	}
	
	ns.IsResolved = true;
}

void ZResolver::ResolveFunctions() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		//LOG("Resolving " + ns.Name);
		
		for (int j = 0; j < ns.PreFunctions.GetCount(); j++) {
			ZFunction& f = ns.PreFunctions[j];
			
			ResolveFunction(f);
		}
	}
}

void ZResolver::ResolveFunction(ZFunction& f) {
	f.GenerateSignatures();
	f.DefPos.Source->Functions.Add(&f);
	//LOG("Resolved " + f.FuncSig());
	
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
	DuplicateLoop(ns, true);
	
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
				ZVariable& var = ns.PreVariables[j];
				if (var.IsDuplicate == false && var.Name == dupes.GetKey(k)) {
					if (allErrors.GetCount() == 0) {
						String err;
						err << ER::Gray << var.DefPos.ToString() << ": ";
						err << ER::Red << "error: ";
						err << ER::White << "duplicate symbol: " << var.Namespace().ProperName << "::" << "val " << ER::Green << var.Name;
						err << "\n" << ER::Gray << "other occurrences at:\n";
						
						allErrors << err;
					}
					else
						allErrors << ER::Gray << "\t" << var.DefPos.ToString() << ": " << var.Namespace().ProperName << "::" << "val " << ER::Green << var.Name << "\n";
				}
			}
			
			dupeListing << allErrors;
		}
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
					
					if (f.DupSig() == dupes.GetKey(k)) {
						if (allErrors.GetCount() == 0) {
							String err;
							err << ER::Gray << f.DefPos.ToString() << ": ";
							err << ER::Red << "error: ";
							err << ER::White << "duplicate symbol: " << f.ColorSig();
							err << "\n" << ER::Gray << "other occurrences at:\n";
							
							allErrors << err;
						}
						else
							allErrors << "\t" << ER::Gray << f.DefPos.ToString() << ": " << f.ColorSig() << "\n";
		
					}
				}
				
				if (lastFunc) {
					for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
						ZVariable& var = ns.PreVariables[j];
						if (var.Name == lastFunc->Name) {
							allErrors << "\t" << ER::Gray << var.DefPos.ToString() << ": " << var.Namespace().ProperName << "::" << "val " << ER::Green << var.Name << "\n";
						}
					}
				}
				
				dupeListing << allErrors;
			}
		}
	}
}
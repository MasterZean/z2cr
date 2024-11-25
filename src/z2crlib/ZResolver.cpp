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

	DUMP(f.InClass);
	DUMP(f.Namespace().Name);
	DUMP(f.Owner().Name);
	ZMethodBundle& d = f.Owner().Methods.GetAdd(f.Name, ZMethodBundle(f.Owner()));
	if (d.Name.GetCount() == 0)
		d.Name = f.Name;
	d.Functions.Add(&f);
}

void ZResolver::ResolveVariables() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
			ZVariable& f = ns.PreVariables[j];
			f.Owner().Variables.Add(f.Name, &f);
		}
	}
}

String Pad(const String& s, int max) {
	StringBuffer ss;
	ss.Cat(s);
	while (ss.GetLength() < max)
		ss.Cat(' ');
	return ss;
}

String PadSp(int max) {
	StringBuffer ss;
	while (ss.GetLength() < max)
		ss.Cat(' ');
	return ss;
}

bool ZResolver::CheckForDuplicates() {
	dupeListing = "";
	
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		CheckForDuplicates(ass.Namespaces[i]);
	
	ASSERT(dupeDecl.GetCount() == dupeFile.GetCount());
	ASSERT(dupeOwner.GetCount() == dupeFile.GetCount());
	
	if (dupeFile.GetCount() == 0)
		return true;
	
	int maxFile = -1;
	for (int i = 0; i < dupeFile.GetCount(); i++) {
		String strip = dupeFile[i];
		DUMP(strip);
		if (dupeFile[i].GetCount() > maxFile)
			maxFile = strip.GetCount();
	}
		
	int maxDec = -1;
	for (int i = 0; i < dupeDecl.GetCount(); i++) {
		String strip = ER::StripColor(dupeDecl[i]);
		DUMP(strip);
		if (strip.GetCount() > maxDec)
			maxDec = strip.GetCount();
	}

	String err;
	
	for (int i = 0; i < dupeFile.GetCount(); i++) {
		if (i == 0 || dupeFile[i].GetCount() == 0) {
			if (dupeFile[i].GetCount() == 0) {
				//if (i != 0)
				//	err << "\n";
				i++;
			}
			
			err << ER::Gray << Pad(dupeFile[i], maxFile) << ER::White << ": ";
			err << ER::Red << "error: ";
			err << ER::White << "declaration in: " << dupeOwner[i] << "\n";
			
			err << "        " << PadSp(maxFile) << "  ";
			err << dupeDecl[i] << PadSp(maxDec - ER::StripColor(dupeDecl[i]).GetCount());
			err << "\n";
			//err << "        " << dupeOwner[i] << "\n";
			
			err << ER::White << "    clashes with the following declarations:\n";
			i++;
		}
		
		err << ER::White << "        " << ER::DkGray << Pad(dupeFile[i], maxFile) << ": " << dupeDecl[i] << PadSp(maxDec - ER::StripColor(dupeDecl[i]).GetCount());
		err << "\n";
		//err << "        " << dupeOwner[i] << "\n";
	}
	
	throw ZException("", err);

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
					allErrors << DupStr(allErrors, c.DefPos, c.ColorSig(), c.OwnerSig());
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
						allErrors << DupStr(allErrors, f.DefPos, f.ColorSig(), f.OwnerSig());
					
				}
				
				if (lastFunc) {
					for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
						ZVariable& v = ns.PreVariables[j];
						if (v.Name == lastFunc->Name)
							allErrors << DupStr(allErrors, v.DefPos, v.ColorSig(), v.OwnerSig());
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
					allErrors << DupStr(allErrors, v.DefPos, v.ColorSig(), v.OwnerSig());
			}
			
			dupeListing << allErrors;
		}
	}
}

String ZResolver::DupStr(const String& allErrors, const ZSourcePos& dp, const String& colorSig, const String& owner) {
	String err = "dummy";
	
	if (allErrors.GetCount() == 0) {
		dupeFile << "";
		dupeDecl << "";
		dupeOwner << "";
	}
	
	dupeFile << dp.ToString();
	dupeDecl << colorSig;
	dupeOwner << owner;
	
	/*// append to error
	if (allErrors.GetCount() != 0) {
		err << ER::White << "        " << ER::Gray << dp.ToString() << ": " << colorSig << "\n";
		return err;
	}
	
	// first message
	err << ER::Gray << dp.ToString() << ": ";
	err << ER::Red << "error: ";
	err << ER::White << "duplicate symbol: " << "the following declarations clash:\n";
	err << ER::White << "        " << ER::Gray << dp.ToString() << ": " << colorSig << "\n";
	//err << "\n" <<  ER::Gray << "    other occurrences at:\n";*/
	
	return err;
}
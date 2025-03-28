#include <z2crlib/ZResolver.h>
#include <z2crlib/ZSource.h>
#include <z2crlib/ZScanner.h>
#include <z2crlib/ErrorReporter.h>

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
	// build namespace item hierarchy
	ZNamespaceItem* nsp = &ass.NsLookup;
	
	String backName;
	String backNameLegacy;
	String legacySufix;
	
	for (int i = 0; i < ns.NameElements.GetCount(); i++) {
		backName << ns.NameElements[i];
		
		if (i == 0)
			backNameLegacy << ns.NameElements[i] << " {";
		else
			backNameLegacy << " namespace "  << ns.NameElements[i] << " {";
		legacySufix << "}";
			
		nsp = nsp->Add(ns.NameElements[i]);
		if (i < ns.NameElements.GetCount() - 1) {
			backName << "::";
		}
		else {
			ASSERT(nsp->Namespace == nullptr);
			nsp->Namespace = &ns;
			
			ns.NamespaceItem = nsp;
		}
	}
	
	ns.BackName = backName;
	ns.BackNameLegacy = backNameLegacy;
	ns.LegacySufix = legacySufix;
	
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
			
			ZClass& cls = ass.AddClass(c);
			ns.Classes.Add(cls.Name, &cls);
			
			for (int k = 0; k < ns.Sources.GetCount(); k++) {
				ns.Sources[k]->AddReference(cls.Namespace().Name + cls.Name, Point(1, 1));
			}
		}
	}
	
	for (int i = 0; i < ass.SourceLookup.GetCount(); i++)
		ass.SourceLookup[i]->AlignReferences();
}

void ZResolver::ResolveFunctions() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		ResolveNamespaceMembers(ns);
				
		for (int j = 0; j < ns.Classes.GetCount(); j++) {
			ZClass& c = *ns.Classes[j];
			ResolveNamespaceMembers(c);
		}
	}
}

void ZResolver::ResolveNamespaceMembers(ZNamespace& ns) {
	if (ns.IsClass) {
		for (int i = 0; i < ns.PreConstructors.GetCount(); i++) {
			ZFunction& f = ns.PreConstructors[i];
			ResolveFunction(ns, f);
		}
	}
	
	for (int i = 0; i < ns.PreFunctions.GetCount(); i++) {
		ZFunction& f = ns.PreFunctions[i];
		ResolveFunction(ns, f);
	}
	
	for (int i = 0; i < ns.PreVariables.GetCount(); i++) {
		ZVariable& f = ns.PreVariables[i];
		f.GenerateSignatures();
	}
}

void ZResolver::ResolveFunction(ZNamespace& ns, ZFunction& f) {
	f.GenerateSignatures(comp);
	f.DefPos.Source->Functions.Add(&f);
	
	f.SetOwner(ns);

	ZMethodBundle& d = f.Owner().Methods.GetAdd(f.Name, ZMethodBundle(f.Owner()));
	if (d.Name.GetCount() == 0)
		d.Name = f.Name;
	d.Functions.Add(&f);
	
	if (f.InClass)
		AssignClassRoles((ZClass&)f.Owner(), f);
}

void ZResolver::AssignClassRoles(ZClass& cls, ZFunction& f) {
	if (f.IsConstructor == 1) {
		if (f.Params.GetCount() == 0)
			cls.Meth.Default = &f;
	}
}

void ZResolver::ResolveVariables() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++) {
		ZNamespace& ns = ass.Namespaces[i];
		for (int j = 0; j < ns.PreVariables.GetCount(); j++) {
			ZVariable& f = ns.PreVariables[j];
			f.Owner().Variables.Add(f.Name, &f);
		}
		
		for (int j = 0; j < ns.Classes.GetCount(); j++) {
			ZClass& c = *ns.Classes[j];
			
			for (int j = 0; j < c.PreVariables.GetCount(); j++) {
				ZVariable& f = c.PreVariables[j];
				f.SetOwner(c);
				f.Owner().Variables.Add(f.Name, &f);
			}
		}
	}
}

bool ZResolver::CheckForDuplicates() {
	for (int i = 0; i < ass.Namespaces.GetCount(); i++)
		CheckForDuplicates(ass.Namespaces[i]);
	
	ASSERT(dupeDecl.GetCount() == dupeFile.GetCount());
	ASSERT(dupeOwner.GetCount() == dupeFile.GetCount());
	
	if (dupeFile.GetCount() == 0)
		return true;
	
	int maxFile = -1;
	for (int i = 0; i < dupeFile.GetCount(); i++) {
		String strip = dupeFile[i];
		if (dupeFile[i].GetCount() > maxFile)
			maxFile = strip.GetCount();
	}
		
	int maxDec = -1;
	for (int i = 0; i < dupeDecl.GetCount(); i++) {
		String strip = ER::StripColor(dupeDecl[i]);
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
			
			err << ER::White << "    clashes with the following declarations:\n";
			i++;
		}
		
		err << ER::White << "        " << ER::DkGray << Pad(dupeFile[i], maxFile) << ": " << dupeDecl[i] << PadSp(maxDec - ER::StripColor(dupeDecl[i]).GetCount());
		err << "\n";
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
			
			for (int i = 0; i < ns.Classes.GetCount(); i++) {
				ZClass& c = *ns.Classes[i];
				
				if (c.Name == dupes.GetKey(k))
					allErrors << DupStr(allErrors, c.DefPos, c.ColorSig(), c.OwnerSig());
			}
		}
	}
	
	// verify members
	DuplicateLoop(ns, true);
	
	// verify class members
	for (int j = 0; j < ns.Classes.GetCount(); j++) {
		ZClass& c = *ns.Classes[j];
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
	
	return err;
}
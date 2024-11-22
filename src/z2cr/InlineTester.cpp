#include <z2cr/InlineTester.h>
#include <z2crlib/ZTranspiler.h>

String anTest = "// @test";
String anFile = "// @file";
String anError = "// @error";
String anErrors = "/* @errors";
String anDumpBody = "/* @dumpBody";
String anDumpEnd = "// @dumpEnd";
String anDumpGlobalVarDef = "/* @dumpGlobalVarDef";

bool ZTest::Run() {
	bool result = true;
	
	if (Source) {
		//DUMP(Con);
		//Source->LoadVirtual(Con);
		Source = nullptr;
	}
		
	/*LOG("-----------------------------------------------------------------------------------------------------------------");
	LOG("NEW TEST");
	for (int i = 0; i < Ass.SourceLookup.GetCount(); i++) {
		LOG("=================================================================================================================");
		LOG(Ass.SourceLookup[i]->Content());
		LOG("=================================================================================================================");
	}*/
			
	try {
		ZCompiler compiler(Ass);
		compiler.SetMain("", "test.z2");
		compiler.FoldConstants = true;
		
		ER::NoColor = true;
		
		bool compResult = compiler.Compile();
		if (compResult == false && compiler.MainFound == false)
			compResult = true;
		
		if (compResult == false && Error.GetCount() == 0)
			result = false;
		
		if (!Error.IsVoid()) {
			LOG(String().Cat() << Name << "(" << Line << ")" << " test failled\n");
			
			for (int i = 0; i < Ass.SourceLookup.GetCount(); i++) {
				LOG("=================================================================================================================");
				LOG(Ass.SourceLookup[i]->Content());
				LOG("=================================================================================================================");
			}
			
			DUMP("No error");
			DUMP(Error);
			result = false;
		}
		
		for (int td = 0; td < Dumps.GetCount(); td++) {
			StringStream ss;
			ZTranspiler cpp(compiler, ss);
			
			for (int n = 0; n < Ass.Namespaces.GetCount(); n++) {
				ZNamespace& ns = Ass.Namespaces[n];
				for (int i = 0; i < ns.Methods.GetCount(); i++) {
					ZMethodBundle& d = ns.Methods[i];
					
					for (int j = 0; j < d.Functions.GetCount(); j++) {
						ZFunction& f = *d.Functions[j];
						//DUMP(f.DefPos.P.x);
						//DUMP(Dumps.GetKey(td));
						if (f.DefPos.P.x == Dumps.GetKey(td))
							cpp.WriteFunctionBody(f, false);
					}
				}
			}
			
			String dump = ss;
			if (Dumps[td] != ss) {
				LOG(String().Cat() << Name << "(" << Line << ")" << " test failled\n");
				
				for (int i = 0; i < Ass.SourceLookup.GetCount(); i++) {
					LOG("=================================================================================================================");
					LOG(Ass.SourceLookup[i]->Content());
					LOG("=================================================================================================================");
				}
			
				DUMP(dump);
				DUMP(Dumps[td]);
				result = false;
				Cout() << Name << "(" << Line << ")" << " test failled because dumps\n";
			}
		}
		
		if (!GlobalVarDef.IsVoid()) {
			StringStream ss;
			ZTranspiler cpp(compiler, ss);
			
			int index = Ass.Namespaces.Find("test");
			for (int i = 0; i < Ass.Namespaces.GetCount(); i++)
				cpp.TranspileValDefintons(Ass.Namespaces[i], false);
			
			String dump = ss;
			if (GlobalVarDef != ss) {
				LOG(String().Cat() << Name << "(" << Line << ")" << " test failled\n");
				
				for (int i = 0; i < Ass.SourceLookup.GetCount(); i++) {
					LOG("=================================================================================================================");
					LOG(Ass.SourceLookup[i]->Content());
					LOG("=================================================================================================================");
				}
			
				DUMP(dump);
				DUMP(GlobalVarDef);
				result = false;
				Cout() << Name << "(" << Line << ")" << " test failled because GlobalVarDef\n";
			}
		}
			
		if (result == false) {
			Cout() << Name << "(" << Line << ")" << " test failled\n";
		}
	}
	catch (ZException& e) {
		StringStream ss;
		e.PrettyPrint(ss);
		String aError = ss;
		aError = TrimRight(aError);
		Error = TrimRight(Error);
		if (Error != aError) {
			LOG(String().Cat() << Name << "(" << Line << ")" << " test failled because found exception\n");
			
			for (int i = 0; i < Ass.SourceLookup.GetCount(); i++) {
				LOG("=================================================================================================================");
				LOG(Ass.SourceLookup[i]->Content());
				LOG("=================================================================================================================");
			}
			
			DUMP(aError);
			DUMP(Error);
			Cout() << Name << "(" << Line << ")" << " test failled because found exception\n";
			result = false;
		}
		else
			result = true;
	}
	catch (CParser::Error& e) {
		// should not reach, but...
		Cout() << e << "\n";
	}
	
	return result;
}

InlineTester::InlineTester() {
}

void InlineTester::AddTestFolder(const String& path, int parent) {
	FindFile ff;
	ff.Search(NativePath(path + "/*"));

	while (ff) {
		if (ff.IsFile()) {
			String name = ff.GetName();
			if (GetFileExt(name) == ".z2test") {
				AddTestCollection(ff.GetPath());
			}
		}
		else if (ff.IsFolder())
			AddTestFolder(ff.GetPath(), parent + 1);

		ff.Next();
	}
}

void InlineTester::AddTestCollection(const String& path) {
	//if (!path.EndsWith("02-decl-02-const.z2test"))
	//	return;
	
	FileIn file(path);
	
	if (file.IsError())
		return;
	
	int lineNo = 0;
	int localLineNo = 0;
	String con;
	
	ZTest* test = nullptr;
	String error;
	
	while (!file.IsEof()) {
		lineNo++;
		localLineNo++;
		String line = file.GetLine();
		
		if (line.StartsWith(anTest)) {
			if (test) {
				if (test->Source)
					test->Source->LoadVirtual(con);
				
				if (test->Run())
					PassCount++;
				
				test = nullptr;
				error = "";
			}
			
			TestCount++;
			con = "";
			con << line << "\n";
			localLineNo = 1;
			
			String rest = line.Mid(anTest.GetCount());
			rest = TrimBoth(rest);
			
			test = &Tests.Add();
			test->Name = GetFileName(path);
			test->Line = lineNo;
			test->MainPak = &test->Ass.AddPackage("main", "");
		}
		else if (line.StartsWith(anFile)) {
			if (test && test->MainPak) {
				if (test->Source) {
					test->Source->LoadVirtual(con);
					con = "";
					localLineNo = 1;
				}
			}
			
			con << line << "\n";
			
			String rest = line.Mid(anFile.GetCount());
			rest = TrimBoth(rest);
			
			if (rest == "" || rest.IsVoid())
				rest = "test";
			
			if (test && test->MainPak) {
				test->Source = &test->MainPak->AddSource(rest + ".z2", false);
			}
		}
		else if (line.StartsWith(anError)) {
			con << line << "\n";
			
			String rest = line.Mid(anError.GetCount());
			rest = TrimBoth(rest);
			
			if (test)
				test->Error = rest;
		}
		else if (line.StartsWith(anDumpBody)) {
			con << line << "\n";
			
			String dump;
			
			while (!file.IsEof()) {
				String sub = file.GetLine();
				lineNo++;
				localLineNo++;
				con << sub << "\n";
				
				if (sub.StartsWith("*/"))
					break;
				else
					dump << sub << "\n";
			}
			
			if (test) {
				test->Dumps.Add(localLineNo + 1, dump);
			}
		}
		else if (line.StartsWith(anDumpGlobalVarDef)) {
			con << line << "\n";
			
			String dump;
			
			while (!file.IsEof()) {
				String sub = file.GetLine();
				lineNo++;
				localLineNo++;
				con << sub << "\n";
				
				if (sub.StartsWith("*/"))
					break;
				else
					dump << sub << "\n";
			}
			
			if (test)
				test->GlobalVarDef = dump;
		}
		else if (line.StartsWith(anErrors)) {
			con << line << "\n";
			
			String err;
			
			while (!file.IsEof()) {
				String sub = file.GetLine();
				lineNo++;
				localLineNo++;
				con << sub << "\n";
				
				if (sub.StartsWith("*/"))
					break;
				else
					err << sub << "\n";
			}
			
			if (test)
				test->Error = err;
		}
		else {
			con << line << "\n";
		}
	}
	
	if (test) {
		if (test->Source)
			test->Source->LoadVirtual(con);
		
		if (test->Run())
			PassCount++;
		
		test = nullptr;
		error = "";
	}
}
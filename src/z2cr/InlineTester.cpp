#include <z2cr/InlineTester.h>
#include <z2crlib/ZTranspiler.h>

String anTest = "// @test";
String anFile = "// @file";
String anError = "// @error";
String anDumpBody = "// @dumpBody";
String anDumpEnd = "// @dumpEnd";
String andumpGlobalVarDef = "// @dumpGlobalVarDef";

bool ZTest::Run() {
	bool result = true;
	
	if (Source) {
		Source->LoadVirtual(Con);
		Source = nullptr;
	}
	
	try {
		ZCompiler compiler(Ass);
		compiler.SetMainFile("test.z2");
		compiler.FoldConstants = true;
		
		bool compResult = compiler.Compile();
		if (compResult == false && compiler.MainFound == false)
			compResult = true;
		
		if (compResult == false && Error.GetCount() == 0)
			result = false;
		
		if (Error.GetCount() != 0) {
			DUMP("No error");
			DUMP(Error);
			result = false;
		}
		
		if (Dump.GetCount()) {
			StringStream ss;
			ZTranspiler cpp(compiler, ss);
			
			for (int i = 0; i < Ass.Namespaces.GetCount(); i++)
				cpp.TranspileDefinitions(Ass.Namespaces[i], false, false, false);
			
			String dump = ss;
			if (Dump != ss) {
				DUMP(dump);
				DUMP(Dump);
				result = false;
			}
		}
		
		if (GlobalVarDef.GetCount()) {
			StringStream ss;
			ZTranspiler cpp(compiler, ss);
			
			int index = Ass.Namespaces.Find("test");
			for (int i = 0; i < Ass.Namespaces.GetCount(); i++)
				cpp.TranspileValDefintons(Ass.Namespaces[i], false);
			
			String dump = ss;
			if (GlobalVarDef != ss) {
				DUMP(dump);
				DUMP(GlobalVarDef);
				result = false;
			}
		}
			
		if (result == false) {
			Cout() << Name << "(" << Line << ")" << " test failled\n";
		}
	}
	catch (ZException& e) {
		if (Error != e.ToString()) {
			DUMP(e.ToString());
			DUMP(Error);
			Cout() << Name << "(" << Line << ")" << " test failled\n";
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
				test->Con = con;
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
			con << line << "\n";
			
			String rest = line.Mid(anFile.GetCount());
			rest = TrimBoth(rest);
			
			if (rest == "" || rest.IsVoid())
				rest = "test";
			
			if (test && test->MainPak)
				test->Source = &test->MainPak->AddSource(rest + ".z2", false);
		}
		else if (line.StartsWith(anError)) {
			con << line << "\n";
			
			String rest = line.Mid(anError.GetCount());
			rest = TrimBoth(rest);
			
			if (test)
				test->Error = rest;
		}
		else if (line.StartsWith(anDumpBody)) {
			String dump;
			
			while (!file.IsEof()) {
				String sub = file.GetLine();
				
				if (sub.StartsWith(anDumpEnd))
					break;
				else
					dump << sub << "\n";
			}
			
			if (test)
				test->Dump = dump;
		}
		else if (line.StartsWith(andumpGlobalVarDef)) {
			String dump;
			
			while (!file.IsEof()) {
				String sub = file.GetLine();
				
				if (sub.StartsWith(anDumpEnd))
					break;
				else
					dump << sub << "\n";
			}
			
			if (test)
				test->GlobalVarDef = dump;
		}
		else {
			con << line << "\n";
		}
	}
	
	if (test) {
		test->Con = con;
		if (test->Run())
			PassCount++;
		
		test = nullptr;
		error = "";
	}
}
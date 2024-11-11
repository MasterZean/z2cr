#include <z2cr/InlineTester.h>

String anTest = "// @test";
String anFile = "// @file";
String anError = "// @error";

bool ZTest::Run() {
	bool result = false;
	
	if (Source) {
		Source->LoadVirtual(Con);
		Source = nullptr;
	}
	
	try {
		ZCompiler compiler(Ass);
		compiler.ScanSources();
		
		if (Error.GetCount() == 0)
			result = true;
		else {
			DUMP("No error");
			DUMP(Error);
		}
	}
	catch (ZException& e) {
		if (Error != e.ToString()) {
			DUMP(e.ToString());
			DUMP(Error);
			Cout() << " test failled\n";
		}
		else
			result = true;
		//Cout() << e.ToString() << "\n";
	}
	catch (CParser::Error& e) {
		// should not reach, but...
		Cout() << e << "\n";
	}
	
	return result;
}

InlineTester::InlineTester() {
}

void InlineTester::AddModule(const String& path, int parent) {
	FindFile ff;
	ff.Search(NativePath(path + "/*"));

	while (ff) {
		if (ff.IsFile()) {
			String name = ff.GetName();
			if (GetFileExt(name) == ".z2test") {
				AddTest(ff.GetPath());
			}
		}
		else if (ff.IsFolder())
			AddModule(ff.GetPath(), parent + 1);

		ff.Next();
	}
}

void InlineTester::AddTest(const String& path) {
	FileIn file(path);
	
	if (file.IsError())
		return;
	
	int lineNo = 0;
	String con;
	
	ZTest* test = nullptr;
	String error;
	
	while (!file.IsEof()) {
		lineNo++;
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
			lineNo = 1;
			
			String rest = line.Mid(anTest.GetCount());
			rest = TrimBoth(rest);
			
			test = &Tests.Add();
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
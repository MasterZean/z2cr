#include <z2cr/InlineTester.h>

String anTest = "// @test";
String anFile = "// @file";
String anError = "// @error";

void ZTest::Run() {
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
	Assembly* ass = nullptr;
	ZPackage* mainPak = nullptr;
	ZSource* source = nullptr;
	
	String error;
	
	while (!file.IsEof()) {
		lineNo++;
		String line = file.GetLine();
		
		if (line.StartsWith(anTest)) {
			if (test) {
				if (source) {
					source->LoadVirtual(con);
					source = nullptr;
				}
				
				try {
					ZCompiler compiler(*ass);
					compiler.ScanSources();
				}
				catch (ZException& e) {
					if (error != e.ToString()) {
						DUMP(e.ToString());
						DUMP(error);
						Cout() << " test failled\n";
					}
					//Cout() << e.ToString() << "\n";
				}
				catch (CParser::Error& e) {
					// should not reach, but...
					Cout() << e << "\n";
				}
				
				test = nullptr;
				error = "";
			}
			
			TestCount++;
			con = "";
			con << line << "\n";
			
			String rest = line.Mid(anTest.GetCount());
			rest = TrimBoth(rest);
			
			test = &Tests.Add();
			ass = &test->Ass;
			mainPak = &ass->AddPackage("main", "");
		}
		else if (line.StartsWith(anFile)) {
			con << line << "\n";
			
			String rest = line.Mid(anFile.GetCount());
			rest = TrimBoth(rest);
			
			if (rest == "" || rest.IsVoid())
				rest = "test";
			
			if (mainPak)
				source = &mainPak->AddSource(rest + ".z2", false);
		}
		else if (line.StartsWith(anError)) {
			con << line << "\n";
			
			String rest = line.Mid(anError.GetCount());
			rest = TrimBoth(rest);
			
			error = rest;
		}
		else {
			con << line << "\n";
		}
	}
	
	if (test) {
		if (source) {
			source->LoadVirtual(con);
			source = nullptr;
		}
		
		try {
			ZCompiler compiler(*ass);
			compiler.ScanSources();
		}
		catch (ZException& e) {
			if (error != e.ToString()) {
				DUMP(e.ToString());
				DUMP(error);
				Cout() << " test failled\n";
			}
			//Cout() << e.ToString() << "\n";
		}
		catch (CParser::Error& e) {
			// should not reach, but...
			Cout() << e << "\n";
		}
		
		test = nullptr;
	}
}
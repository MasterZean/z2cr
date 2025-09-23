#include <z2crlib/StopWatch.h>
#include <z2crlib/ZCompiler.h>
#include <z2cr/CommandLine.h>
#include <z2crlib/BuildMethod.h>
#include <z2cr/Builder.h>
#include <z2cr/InlineTester.h>
#include <z2crlib/ErrorReporter.h>
#include <z2crlib/LLVMIR.h>

bool GetBuildMethod(const String& exeDir, const ::CommandLine& K, BuildMethod& bm) {
	Vector<BuildMethod> methods;
	
	// load existing BMs
	LoadFromXMLFile(methods, exeDir + "buildMethods.xml");
	if (methods.GetCount() == 0) {
		Cout() << "No cached build method found! Trying to auto-detect...\n";
		Cout().Flush();
		BuildMethod::Get(methods);
		if (methods.GetCount() == 0) {
			Cout() << ZCompiler::GetName() << " couldn't find a backend compiler. Skipping compilation step..." << '\n';
			Cout().Flush();
			//K.SCU = false;
		}
		else
			StoreAsXMLFile(methods, "methods", exeDir + "buildMethods.xml");
	}
	
	if (K.BM == false && K.ARCH == false && methods.GetCount() > 0) {
		bm = methods[0];
	
		return true;
	}
	
	int bmi = -1;
	
	// find input BM
	for (int i = 0; i < methods.GetCount(); i++) {
		bool found = true;
	
		if (K.BM && ToUpper(K.BMName) != ToUpper(methods[i].Name)) {
			found = false;
		}
		
		if (K.ARCH && ToUpper(K.ARCHName) != ToUpper(methods[i].Arch)) {
			found = false;
		}
		
		if (found) {
			bmi = i;
			break;
		}
	}
	
	if (bmi == -1) {
		String bms = K.BMName;
		if (bms.GetCount())
			bms << " ";
		bms << K.ARCHName;
		
		if (bms.GetCount())
			Cout() << "Build method '" << ToUpper(bms) << "' can't be found. Exiting!" << '\n';
		else
			Cout() << "No build method can be found. Exiting!" << '\n';
		Cout().Flush();
		
		return false;
	}
	
	bm = methods[bmi];
	
	return true;
}

void RunInlineTests(const String& path) {
	Cout() << "Searching for tests...\n";
	
	StopWatch sw;
	
	InlineTester tester;
	tester.AddTestFolder(path);
	
	if (tester.TestCount == 0) {
		Cout() << "Could not find any tests. Aborting testing!\n\n";
		return;
	}
	
	Cout() << "Testing status: " << tester.PassCount << "/" << tester.TestCount << " passed (" << sw.ToString() <<"s).\n\n";
}

#ifdef PLATFORM_WIN32

int separate_console() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE), &csbi))
    {
        //printf( "GetConsoleScreenBufferInfo failed: %lu\n", GetLastError());
        return 0;
    }

    // if cursor position is (0,0) then we were launched in a separate console
    return ((!csbi.dwCursorPosition.X) && (!csbi.dwCursorPosition.Y));
}

bool EnableVTMode() {
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return false;
    }
    return true;
}

#else

int separate_console() {
	return 0;
}

bool EnableVTMode() {
	return true;
}

#endif

CONSOLE_APP_MAIN {
	bool noConsole = separate_console();
	
	String curDir = NativePath(GetCurrentDirectory() + "/");
	String exeDir = GetFileDirectory(GetExeFilePath());
	
	::CommandLine K;
	if (!K.Read()) {
		SetExitCode(-1);
		return;
	}
	
	if (K.UTEXE) {
		String cmd = GetExeFilePath() + " -ut";
		LocalProcess exe(cmd);
		
		String t, tt;
		while (exe.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		
		Cout() << tt;
		
		if (K.Files.GetCount() == 0) {
			return;
		}
	}
	
	if (K.UT) {
		RunInlineTests(AppendFileName(curDir, "tests"));
		
		if (K.Files.GetCount() == 0) {
			return;
		}
	}
		
	if (!K.SCU && !K.CPP) {
		Cout() << ZCompiler::GetName() << " requires the '-scu' or '-c++' parameters. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	if (K.PP_NOPATH)
		ER::PrintPath = false;
	else
		ER::PrintPath = true;
	
	ER::ErrorColor = K.ErrorColor;
	if (ER::ErrorColor == ErrorColorType::Ansi) {
		bool result = EnableVTMode();
		LOG(result);
	}
	
	if (K.NoConsoleTest)
		noConsole = false;
	
	if (K.Files.GetCount() == 0 && K.UT == false) {
		Cout() << ZCompiler::GetName() << " requires a list of input packages or files. Exiting!" << '\n';
		SetExitCode(-1);
		
		if (noConsole) {
			Cout() << "Press any key to continue...";
			ReadStdIn();
		}
		
		return;
	}
	
	BuildMethod bm;
	if (!GetBuildMethod(exeDir, K, bm)) {
		SetExitCode(-1);
		return;
	}
	
	StopWatch tm;
	
	Assembly ass;
	ass.SetBM(bm);
	
	try {
		ZCompiler compiler(ass);
		compiler.BuildMode = true;
		compiler.CppVersion = bm.CppVersion;
		compiler.BuildProfile = compiler.PlatformString + ToUpper(bm.Arch) + "." + ToUpper(bm.Name) + K.O;
		compiler.BuildPath = exeDir + NativePath("builds/") + compiler.PlatformString + "." + ToUpper(bm.Arch) + "." + ToUpper(bm.Name);
		RealizeDirectory(compiler.BuildPath);
		
		ass.BuildPath = compiler.BuildPath;
		
		String prjPath;
		if (IsFullPath(K.EntryFile))
			prjPath = GetFileFolder(K.EntryFile);
		else
			prjPath = GetFileFolder(curDir + K.EntryFile);
		
		String prjPackage = GetFileName(prjPath);
		String prjCP = GetFileDirectory(prjPath);
		
		for (int i = 0; i < K.Packages.GetCount(); i++) {
			if (!ass.LoadPackage(K.Packages[i])) {
				SetExitCode(BuildMethod::ErrorCode(-1));
				return;
			}
		}
		
		if (!ass.AddStdlibPakcages(exeDir)) {
			SetExitCode(BuildMethod::ErrorCode(-1));
			return;
		}
		
		if (K.Files.GetCount()) {
			ZPackage& mainPak = ass.AddPackage("main", "");
			for (int i = 0; i < K.Files.GetCount(); i++)
				ZSource& source = mainPak.AddSource(K.Files[i], true);
		}
		
		ass.WriteCache();
				
		Cout() << "Done loading packages, " << ass.FilesNew << " new files, " << ass.FilesChanged << " changed files, " << ass.FilesUnchanged << " unchanged files (" << tm.ToString() << "s)\n\n";
		
		if (IsFullPath(K.OutPath))
			compiler.OutPath = K.OutPath;
		else
			compiler.OutPath = curDir + K.OutPath;
			
		compiler.SetMain(K.EntryClass, K.EntryFile);
		
		StopWatch tmComp;
		
		if (!compiler.Compile(true)) {
			if (compiler.MainFound == false) {
				if (K.EntryFile.GetCount())
					Cout() << "No main entry point found. Exiting.\n";
				else
					Cout() << "No main entry point found. Please provide one using the '-main' or '-mainfile' parameters. Exiting.\n";
			}
			else
				Cout() << "Compilation failed. Exiting.\n";
			SetExitCode(-1);
			return;
		}

		if (!compiler.Transpile()) {
			Cout() << "CPP code generation failed. Exiting.\n";
			SetExitCode(-1);
			return;
		}
		
		Cout() << "Compilation finished in " << tmComp.ToString() << " seconds.\n\n";
		
		Builder builder(bm);
		builder.ZCompilerPath(exeDir);
		builder.TargetRoot(compiler.BuildPath);
		builder.Arch(bm.Arch);
		builder.Optimize(K.O);
		builder.Subsystem(K.SUBSYSTEM);
		
		if (compiler.MainFound == false) {
			if (compiler.MainFound == false) {
				if (K.EntryFile.GetCount())
					Cout() << "No main entry point found. Exiting.\n";
				else
					Cout() << "No main entry point found. Please provide one using the '-main' or '-mainfile' parameters. Exiting.\n";
			}
			SetExitCode(-1);
			return;
		}
		
		Cout() << "Using '" << ToUpper(bm.Name) << "." << ToUpper(bm.Arch) << "' back end compiler.\n\n";
		
		bool buildOk = builder.Build(compiler.CppPath, compiler.OutPath, ass.LibLink);
		if (buildOk) {
			Cout() << "\n";
			Cout() << bm.Name << " code generation finished, total build time " << tm.ToString() << " seconds.\n";
		}
		else {
			SetExitCode(-1);
			Cout() << "\n";
			Cout() << bm.Name << " code generation failed.\n";
		}
		
		for (int i = 0; i < compiler.MCUPaths.GetCount(); i++) {
			String p = compiler.MCUPaths[i];
			Cout() << "Building: " << p << ": ";
			
			buildOk = builder.Compile(p, GetFileFolder(p) + "/" + GetFileTitle(p) + ".o", compiler.BuildPath);
			//Cout() << "\n";
		}
		
		Cout() << "\n";
		
#if defined(_RELEASE) && defined(flagLLVM) && defined(_MSC_VER)
		int main2();
		
		//main2();
		Cout() << "\n";
			
		LLVMIR llvm;
		
		for (int i = 0; i < compiler.LLVMInput.GetCount(); i++) {
			llvm.AddFunc(*compiler.LLVMInput[i]);
		}
		
		llvm.Print();
#endif
	}
	catch (ZException& e) {
		e.PrettyPrint(Cout());
		Cout() << "\n";
		SetExitCode(-1);
	}
	catch (CParser::Error& e) {
		// should not reach, but...
		Cout() << e << "\n";
		SetExitCode(-1);
	}
}

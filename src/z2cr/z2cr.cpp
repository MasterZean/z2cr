#include "z2cr.h"

#include <Core/Core.h>

using namespace Upp;

#include <z2cr/StopWatch.h>
#include <z2cr/ZCompiler.h>

CONSOLE_APP_MAIN {
	String path = "c:\\temp\\test.z2";
	
	String curDir = GetCurrentDirectory() + "/";
	String exeDir = GetFileDirectory(GetExeFilePath());

	::CommandLine K;
	if (!K.Read()) {
		SetExitCode(-1);
		return;
	}
	
	Vector<BuildMethod> methods;
	
	// load existing BMs
	LoadFromXMLFile(methods, exeDir + "buildMethods.xml");
	if (methods.GetCount() == 0) {
		Cout() << "No cached build method found! Trying to auto-detect...\n";
		BuildMethod::Get(methods);
		if (methods.GetCount() == 0) {
			Cout() << ZCompiler::GetName() << " couldn't find a backend compiler. skiping compilation step..." << '\n';
			//K.SCU = false;
		}
		else
			StoreAsXMLFile(methods, "methods", exeDir + "buildMethods.xml");
	}
	
	// check params
	if (!K.SCU && !K.CPP && !K.INT && !K.VASM) {
		Cout() << ZCompiler::GetName() << " requires the '-scu', '-c++', '-vasm' or '-i' parameters. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	if (!K.BM) {
		Cout() << ZCompiler::GetName() << " requires a build method specified (-bm). Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	int bmi = -1;
	
	// find input BM
	for (int i = 0; i < methods.GetCount(); i++)
		if (ToUpper(K.BMName) == ToUpper(methods[i].Name)) {
			bmi = i;
			break;
		}
	
	if (bmi == -1) {
		Cout() << "Build method '" << ToUpper(K.BMName) << "' can't be found. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	bmi = -1;
	// find input BM with given arch
	for (int i = 0; i < methods.GetCount(); i++)
		if (ToUpper(K.BMName) == ToUpper(methods[i].Name) && ToUpper(K.ARCH) == ToUpper(methods[i].Arch)) {
			bmi = i;
			break;
		}
	
	if (bmi == -1) {
		Cout() << "Build method '" << ToUpper(K.BMName) << "' doesn't support architecture 'x86'. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	if (K.PP_NOPATH)
		ErrorReporter::PrintPath = false;

	
	if (K.Path.GetCount() == 0) {
		return;
	}
	
	BuildMethod& bm = methods[bmi];
	
	StopWatch tm;
	// compile
	Assembly ass;
	
	try {
		String prjPath;
		if (IsFullPath(K.Path))
			prjPath = GetFileFolder(K.Path);
		else
			prjPath = GetFileFolder(curDir + K.Path);
		
		String prjPackage = GetFileName(prjPath);
		String prjCP = GetFileDirectory(prjPath);
		
		if (!ass.LoadPackage("c:\\temp\\test.pak")) {
			SetExitCode(BuildMethod::ErrorCode(-1));
			return;
		}
		
		//ass.LoadPackage("c:\\temp\\a\\test.pak");
		
		ZPackage& stdPakPak = *ass.FindPackage("test");
		
		ZPackage& mainPak = ass.AddPackage("main", "");
		ZSource& source = mainPak.AddSource(K.Path, true);
		
		ZExprParser::Initialize();
			
		ZCompiler compiler(ass);
		
#ifdef PLATFORM_WIN32
		String platform = "WIN32";
		String platformLib = "microsoft.windows";
#endif
		
#ifdef PLATFORM_POSIX
		String platform = "POSIX";
		String platformLib = "ieee.posix";
#endif

		compiler.BuildProfile = platform + ToUpper(K.ARCH) + "." + ToUpper(bm.Name) + K.O;
		compiler.BuildPath = exeDir + NativePath("build\\") + platform + "." + ToUpper(K.ARCH) + "." + ToUpper(bm.Name);
		RealizeDirectory(compiler.BuildPath);
		
		Cout() << "\nCompiling...\n\n";
			
		compiler.SetMainFile(K.Path);
		compiler.Compile();
		
		Cout() << "\n";
		
		Cout() << bm.Name << " code generation finished in " << tm.ToString() << " seconds.\n";
	}
	catch (ZException e) {
		Cout() << e.ToString() << "\n";
		SetExitCode(-1);
		
		Cout() << bm.Name << " code generation failed.\n";
	}
}

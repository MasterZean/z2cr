#include "z2cr.h"

#include <Core/Core.h>

using namespace Upp;

#include <z2cr/StopWatch.h>
#include <z2cr/ZCompiler.h>
#include <z2cr/CommandLine.h>
#include <z2cr/BuildMethod.h>
#include <z2cr/Builder.h>

bool GetBuildMethod(const String& exeDir, const ::CommandLine& K, BuildMethod& bm) {
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
	
	if (!K.BM && K.ARCH.GetCount() == 0 && methods.GetCount() > 0) {
		bm = methods[0];
	
		return true;
	}
		
	if (!K.BM) {
		Cout() << ZCompiler::GetName() << " requires a build method specified (-bm). Exiting!" << '\n';
		return false;
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
		return false;
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
		return false;
	}
	
	bm = methods[bmi];
	
	return true;
}

CONSOLE_APP_MAIN {
	String curDir = GetCurrentDirectory() + "/";
	String exeDir = GetFileDirectory(GetExeFilePath());

	::CommandLine K;
	if (!K.Read()) {
		SetExitCode(-1);
		return;
	}
	
	// check params
	if (!K.SCU && !K.CPP && !K.INT && !K.VASM) {
		Cout() << ZCompiler::GetName() << " requires the '-scu', '-c++', '-vasm' or '-i' parameters. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	if (K.PP_NOPATH)
		ErrorReporter::PrintPath = false;
	
	if (K.Path.GetCount() == 0) {
		Cout() << ZCompiler::GetName() << " requires an execution entry point. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	BuildMethod bm;
	if (!GetBuildMethod(exeDir, K, bm)) {
		SetExitCode(-1);
		return;
	}
	
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
		
		for (int i = 0; i < K.Packages.GetCount(); i++) {
			if (!ass.LoadPackage(K.Packages[i])) {
				SetExitCode(BuildMethod::ErrorCode(-1));
				return;
			}
		}
		
		//ZPackage& stdPakPak = *ass.FindPackage("test");
		
		//ZPackage& mainPak = ass.AddPackage("main", "");
		//ZSource& source = mainPak.AddSource(K.Path, true);
			
		ZCompiler compiler(ass);
		
		if (IsFullPath(K.OutPath))
			compiler.OutPath = K.OutPath;
		else
			compiler.OutPath = curDir + K.OutPath;
		
		compiler.BuildProfile = compiler.PlatformString + ToUpper(K.ARCH) + "." + ToUpper(bm.Name) + K.O;
		compiler.BuildPath = exeDir + NativePath("build_r\\") + compiler.PlatformString + "." + ToUpper(K.ARCH) + "." + ToUpper(bm.Name);
		RealizeDirectory(compiler.BuildPath);
		
		Cout() << "\n";
			
		compiler.SetMainFile(K.Path);
		
		if (!compiler.Compile()) {
			Cout() << "Compilation failed. Exiting.\n";
			SetExitCode(-1);
			return;
		}

		Builder builder(bm);
		builder.ZCompilerPath(exeDir);
		builder.TargetRoot(compiler.BuildPath);
		builder.Arch(K.ARCH);
		builder.Optimize(K.O);
		
		bool buildOk = builder.Build(compiler.OutPath, compiler.OutPath);
		if (buildOk) {
			Cout() << "\n";
			Cout() << bm.Name << " code generation finished in " << tm.ToString() << " seconds.\n";
		}
		else {
			SetExitCode(-1);
			Cout() << "\n";
			Cout() << bm.Name << " code generation failed.\n";
		}
	}
	catch (ZException e) {
		Cout() << e.ToString() << "\n";
		SetExitCode(-1);
	}
	catch (CParser::Error e) {
		// should not reach, but...
		Cout() << e << "\n";
		SetExitCode(-1);
	}
}

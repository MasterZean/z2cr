#include <z2cr/Builder.h>
#include <z2crlib/StopWatch.h>

#ifdef PLATFORM_POSIX

bool Builder::CompileMSC(const String& src, const String& out, const String& inc) {
	return false;
}

bool Builder::CompileGCC(const String& src, const String& out, const String& inc) {
	StopWatch sw;
	
	Cout() << "building " << GetFileTitle(src) << "... ";
	String cmd;
	String t, tt;
	
	/*cmd << "SET PATH=%PATH%;";
	cmd << bm.Compiler << "\\bin" << ";";
	cmd << " & ";*/
	
	cmd << QT << cppPath << QT << " ";
	cmd << "-w " << optimize << " ";
	cmd << "-c -x c++ -fpermissive -fexceptions -msse2 -std=c++11";
	cmd << " -m";
	if (arch == "x64")
		cmd << "64";
	else
		cmd << "32";
	cmd << " ";
	cmd << QT << src << QT << " ";
	cmd << "-o " << QT << out << QT << " ";
	
	if (inc.GetCount())
		cmd << "-I\"" << inc << "\"";
	
	DUMP(cmd);
	
	//String pp = "cmd.exe /C \"" + cmd + "\"";
	
	LocalProcess lp(cmd);
	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	
	//DUMP(lp.GetExitMessage());
	if (!tt.IsEmpty()) {
		Cout() << "FAILED!\n";
		Cout() << tt << "\n";
		return false;
	}
	else {
		Cout() << "done in " << sw.ToString() << " seconds.\n";
		return true;
	}
}

bool Builder::BuildMSC(const String& path, const String& origPath, const Index<String>& libs) {
	return false;
}

bool Builder::BuildGCC(const String& path, const String& origPath, const Index<String>& libs) {
	String inPath = GetFileDirectory(path);
	String inTitle = GetFileTitle(path);
	String outPath = GetFileDirectory(origPath);
	String outTitle = GetFileTitle(origPath);
	String c;

	bool result = true;
	
	cppPath = bm.Compiler;
	linkPath = cppPath;

	if (optimize == " -Od")
		optimize = " -Og";
	
	String leakObj = AppendFileName(target, "leakdetect.o");

	if (!FileExists(leakObj))
		if (!CompileGCC(zPath + "codegen/leakdetect.cpp", leakObj))
			result = false;
		
	String bridgeObj = AppendFileName(target, "zbridge.o");

	if (!FileExists(bridgeObj))
		if (!CompileGCC(zPath + "codegen/zbridge.cpp", bridgeObj))
			result = false;
		
	if (!CompileGCC(inPath + inTitle + ".cpp", inPath + inTitle + ".o"))
		result = false;
	
	StopWatch sw;
	String d, t, tt;
	
	Cout() << "linking... ";
	
	d << QT + linkPath + QT + " " + inPath + inTitle + ".o \"" + leakObj + "\" \"" + bridgeObj + "\" -o \"" + outPath + outTitle + "\" -m";
	if (arch == "x64")
		d << "64";
	else
		d << "32";
	
	//d << " -static";
	for (int i = 0; i < libs.GetCount(); i++)
		d << " -l" << libs[i];
	
	DUMP(d);
	//Cout() << d;
	
	{
		LocalProcess lp(d);
		while (lp.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		if (tt.GetLength()) {
			Cout() << "FAILED!\n";
			Cout() << tt << "\n";
			result = false;
		}
		else {
			Cout() << "done in " << sw.ToString() << " seconds.\n";
		}
	}
	
	return result;
}

#endif
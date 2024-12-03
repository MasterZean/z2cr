#include <z2cr/Builder.h>
#include <z2crlib/StopWatch.h>

String QT = "\"";

bool Builder::Build(const String& path, const String& origPath, const Index<String>& libs) {
	if (bm.Type == BuildMethod::btMSC)
		return BuildMSC(path, origPath, libs);
	
	if (bm.Type == BuildMethod::btGCC)
		return BuildGCC(path, origPath, libs);

	return false;
}

#ifdef PLATFORM_WIN32

String GetWinPath() {
	WCHAR h[MAX_PATH];
	GetWindowsDirectoryW(h, MAX_PATH);
	return FromSystemCharsetW(h);
}

void Builder::DoPathsMSC() {
	cppPath = bm.Compiler + "\\";
	if (arch == "x64") {
		if (FindFile(cppPath + "x86_amd64\\cl.exe"))
			cppPath << "x86_amd64\\cl.exe";
		else
			cppPath << "cl.exe";
	}
	else
		cppPath << "cl.exe";
	cppPath = NormalizePath(cppPath);
	
	linkPath = bm.Compiler + "\\";
	if (arch == "x64") {
		if (FindFile(linkPath + "x86_amd64\\link.exe"))
			linkPath << "x86_amd64\\link.exe";
		else
			linkPath << "link.exe";
	}
	else
		linkPath << "link.exe";
	linkPath = NormalizePath(linkPath);
}

void Builder::DoEnvMSC() {
	StringStream ss;
	
	ss << "PATH=";
	ss << GetEnv("PATH") << ";";
	ss << bm.Tools << "\\" << ";";
	ss << bm.Compiler << "\\" << ";";
	ss.Put(0);

	ss << "LIB=";
	ss << GetEnv("LIB");
	
	for (int i = 0; i < bm.Lib.GetCount(); i++) {
		ss << bm.Lib[i] << ";";
	}
	ss.Put(0);
	
	ss << "INCLUDE=";
	ss << GetEnv("INCLUDE");
	
	for (int i = 0; i < bm.Lib.GetCount(); i++) {
		ss << bm.Include[i] << ";";
	}
	ss.Put(0);
	
	String tempPath = GetEnv("TMP");
	ss << "TMP=";
	ss << tempPath;
	ss.Put(0);
	ss << "TEMP=";
	ss << tempPath;
	ss.Put(0);
	
	ss << "SystemRoot=" << GetWinPath();
	ss.Put(0);
	
	ss.Put(0);
	
	env = ss;
}

bool Builder::CompileMSC(const String& src, const String& out) {
	StopWatch sw;
	
	Cout() << "building " << GetFileTitle(src) << "... ";
	String cmd;
	String t, tt;
	
	cmd << QT << cppPath << QT << " ";
	cmd << QT << src << QT << " ";
	cmd << "/Fo" << QT << out << QT << " ";
	cmd << optimize << " ";
	cmd << "/c /nologo /MT /EHsc /std:c++17";
	
	DUMP(cmd);
	
	LocalProcess lp(cmd, env);
	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	
	if (tt != GetFileTitle(src) + ".cpp\r\n") {
		Cout() << "FAILLED!\n";
		Cout() << tt << "\n";
		return false;
	}
	else {
		Cout() << "done in " << sw.ToString() << " seconds.\n";
		return true;
	}
}


bool Builder::BuildMSC(const String& path, const String& origPath, const Index<String>& libs) {
	bool result = true;

	DoPathsMSC();
	
	String inPath = GetFileDirectory(path);
	String inTitle = GetFileTitle(path);
	String outPath = GetFileDirectory(origPath);
	String outTitle = GetFileTitle(origPath);
		
	DoEnvMSC();
	DUMP(env);
	
	StopWatch sw;
	
	String d;
	String t, tt;
	
	//SetCurrentDirectory(inPath);
	
	String leakObj = AppendFileName(target, "leakdetect.obj");

	if (!FileExists(leakObj))
		if (!CompileMSC(zPath + "codegen\\leakdetect.cpp", leakObj))
			result = false;
	
	if (!CompileMSC(inPath + inTitle + ".cpp", inPath + inTitle + ".obj"))
		result = false;
	
	sw.Reset();
	Cout() << "linking... ";
	d = "";
	t = "";
	tt = "";
	
	d << QT + linkPath + QT + " " + inPath + inTitle + ".obj \"" + leakObj + "\" user32.lib /nologo " +"/out:\"" + outPath + outTitle + ".exe\"";
	if (arch == "x64")
		d << " /MACHINE:x64 ";
	{
		LocalProcess lp(d, env);
		while (lp.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		if (tt.GetLength()) {
			Cout() << "FAILLED!\n";
			Cout() << tt << "\n";
			result = false;
		}
		else {
			Cout() << "done in " << sw.ToString() << " seconds.\n";
		}
	}
		
	//DeleteFile(inPath + inTitle + ".obj");
	//DeleteFile(inPath + inTitle + ".cpp");
	
	return result;
}

void Builder::DoEnvGCC() {
	StringStream ss;
	
	ss << "PATH=";
	//ss << bm.Tools << "\\" << ";";
	ss << GetFileFolder(bm.Compiler) << ";";
	ss << GetEnv("PATH") << ";";
	ss.Put(0);

	/*ss << "LIB=";
	ss << GetEnv("LIB");
	
	for (int i = 0; i < bm.Lib.GetCount(); i++) {
		ss << bm.Lib[i] << ";";
	}
	ss.Put(0);*/
	
	String tempPath = GetEnv("TMP");
	ss << "TMP=";
	ss << tempPath;
	ss.Put(0);
	ss << "TEMP=";
	ss << tempPath;
	ss.Put(0);
	
	/*ss << "SystemRoot=" << GetWinPath();
	ss.Put(0);*/
	
	ss.Put(0);
	
	env = ss;
}

bool Builder::CompileGCC(const String& src, const String& out) {
	StopWatch sw;
	
	Cout() << "building " << GetFileTitle(src) << "... ";
	String cmd;
	String t, tt;
	
	/*cmd << "SET PATH=%PATH%;";
	cmd << bm.Compiler << "\\bin" << ";";
	cmd << " & ";*/
	
	cmd << QT << cppPath << QT << " ";
	cmd << "-w " << optimize << " ";
	cmd << "-c -x c++ -fpermissive -fexceptions -msse2 -std=c++17";
	cmd << " -m";
	if (arch == "x64")
		cmd << "64";
	else
		cmd << "32";
	cmd << " ";
	cmd << QT << src << QT << " ";
	cmd << "-o " << QT << out << QT << " ";
	
	DUMP(cmd);
	
	//String pp = "cmd.exe /C \"" + cmd + "\"";
	
	LocalProcess lp;
	if (!lp.Start(cmd, env)) {
		Cout() << "FAILLED!\n";
		Cout() << "Could not execute: " << cppPath << "\n";
		return false;
	}
	
	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	
	//DUMP(lp.GetExitMessage());
	if (!tt.IsEmpty()) {
		Cout() << "FAILLED!\n";
		Cout() << tt << "\n";
		return false;
	}
	else {
		Cout() << "done in " << sw.ToString() << " seconds.\n";
		return true;
	}
}

bool Builder::BuildGCC(const String& path, const String& origPath, const Index<String>& libs) {
	DUMP(path);
	String inPath = GetFileDirectory(path);
	String inTitle = GetFileTitle(path);
	String outPath = GetFileDirectory(origPath);
	String outTitle = GetFileTitle(origPath);
	String c;
	//String O = optimize;
	bool result = true;

	cppPath = bm.Compiler;
	linkPath = cppPath;
	
	DoEnvGCC();
	DUMP(env);
	
	if (optimize == " -Od")
		optimize = " -Og";
	
	String leakObj = AppendFileName(target, "leakdetect.o");

	if (!FileExists(leakObj))
		if (!CompileGCC(zPath + "codegen\\leakdetect.cpp", leakObj))
			result = false;
	
	if (!CompileGCC(inPath + inTitle + ".cpp", inPath + inTitle + ".o"))
		result = false;
	
	if (result == false) {
		Cout() << "BUILDING FAILLED!\n";
		return result;
	}
	
	StopWatch sw;
	String d, t, tt;
	
	Cout() << "linking... ";
	if (arch == "x64")
		d << QT + linkPath + QT + " " + inPath + inTitle + ".o \"" + leakObj + "\" -lwinmm -luser32 --static -o \"" + outPath + outTitle + ".exe\" -m";
	else
		d << QT + linkPath + QT + " " + inPath + inTitle + ".o \"" + leakObj + "\" -lwinmm -luser32 --static -o \"" + outPath + outTitle + ".exe\" -m";
	
	if (arch == "x64")
		d << "64";
	else
		d << "32";
	
	for (int i = 0; i < libs.GetCount(); i++)
		d << " -l" << libs[i];
	//d << "";
	//d << " -lmingw32";
	//d << " -Wl,--subsystem,windows";
	//d << "-mthreads -mconsole";
	//d << " /entry:mainCRTStartup";
	/*if (arch == "x64")
		d << " /MACHINE:x64 ";*/
	DUMP(d);
	{
		LocalProcess lp(d, env);
		while (lp.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		if (tt.GetLength()) {
			Cout() << "FAILLED!\n";
			Cout() << tt << "\n";
			result = false;
		}
		else {
			Cout() << "done in " << sw.ToString() << " seconds.\n";
		}
	}

	//DeleteFile(inPath + inTitle + ".o");
	//DeleteFile(inPath + inTitle + ".cpp");
	
	return result;
}

#endif

#ifdef PLATFORM_POSIX

bool Builder::CompileMSC(const String& src, const String& out) {
	return false;
}

bool Builder::CompileGCC(const String& src, const String& out) {
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
	
	DUMP(cmd);
	
	//String pp = "cmd.exe /C \"" + cmd + "\"";
	
	LocalProcess lp(cmd);
	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	
	//DUMP(lp.GetExitMessage());
	if (!tt.IsEmpty()) {
		Cout() << "FAILLED!\n";
		Cout() << tt << "\n";
		return false;
	}
	else {
		Cout() << "done in " << sw.ToString() << " seconds.\n";
		return true;
	}
}

bool Builder::BuildMSC(const String& path, const String& origPath) {
	return false;
}

bool Builder::BuildGCC(const String& path, const String& origPath) {
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
	
	if (!CompileGCC(inPath + inTitle + ".cpp", inPath + inTitle + ".o"))
		result = false;
	
	StopWatch sw;
	String d, t, tt;
	
	Cout() << "linking... ";
	
	d << QT + linkPath + QT + " " + inPath + inTitle + ".o \"" + leakObj + "\" -o \"" + outPath + outTitle + "\" -m";
	if (arch == "x64")
		d << "64";
	else
		d << "32";
	
	{
		LocalProcess lp(d);
		while (lp.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		if (tt.GetLength()) {
			Cout() << "FAILLED!\n";
			Cout() << tt << "\n";
			result = false;
		}
		else {
			Cout() << "done in " << sw.ToString() << " seconds.\n";
		}
	}

	DeleteFile(inPath + inTitle + ".o");
	DeleteFile(inPath + inTitle + ".cpp");
	
	return result;
}

#endif
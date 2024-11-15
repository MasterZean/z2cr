#include <z2cr/BuildMethod.h>

String NormalizePathNN(const String& path) {
	return IsNull(path) ? path : NormalizePath(path);
}

#ifdef PLATFORM_WIN32

struct DirFinder {
	Vector<String> dir;

	String Get(const String& substring, const char *files);
	void   GatherDirs(Index<String>& path, const String& dir);
	
	DirFinder();
};

void DirFinder::GatherDirs(Index<String>& path, const String& dir) {
	path.FindAdd(dir);
	FindFile ff(dir + "/*");
	while(ff) {
		if(ff.IsFolder())
			GatherDirs(path, ff.GetPath());
		ff.Next();
	}
}

DirFinder::DirFinder() {
	Index<String> path;

	for(int i = 0; i < 3; i++) {
		HKEY key = 0;
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders", 0,
		                KEY_READ|decode(i, 0, KEY_WOW64_32KEY, 1, KEY_WOW64_64KEY, 0),
		                &key) == ERROR_SUCCESS) {
			int i = 0;
			for(;;) {
				char  value[255];
				unsigned long valueSize = 250;
				*value = 0;
		
				if(RegEnumValue(key, i++, value, &valueSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
					break;
				
				path.FindAdd(value);
				DUMP(value);
			}
			RegCloseKey(key);
		}
	}
	
	Array<FileSystemInfo::FileInfo> root = StdFileSystemInfo().Find(Null);
	for(int i = 0; i < root.GetCount(); i++) {
		if(root[i].root_style == FileSystemInfo::ROOT_FIXED) {
			int drive = *root[i].filename;
			String pf = GetProgramsFolderX86();
			pf.Set(0, drive);
			pf = AppendFileName(pf, "Microsoft Visual Studio");
			if(DirectoryExists(pf))
				GatherDirs(path, pf);
		}
	}

	for(String s : path) {
		s = ToLower(s);
		s.Replace("\\", "/");
		while(s.TrimEnd("/"));
		dir.Add(s);
	}
}

String DirFinder::Get(const String& substring, const char *files) {
	String path;
	Vector<int> versions;
	Vector<String> fn = Split(files, ';');
	for(const auto& d : dir) {
		int p = d.Find(substring);
		if(p >= 0) {
			String cp = d;
			for(const auto& f : fn)
				if(!FileExists(AppendFileName(d, f))) {
					cp.Clear();
					break;
				}
			if(cp.GetCount()) {
				Vector<String> h = Split(cp.Mid(p), [] (int c) { return IsDigit(c) ? 0 : c; });
				Vector<int> vers;
				for(const auto& hh : h)
					vers.Add(atoi(hh));
				if(CompareRanges(vers, versions) > 0) {
					path = cp;
					versions = clone(vers);
				}
			}
		}
	}
	return path;
}

void DeepVSSearch(Vector<BuildMethod>& methods) {
	DirFinder df;
	
	enum { VS_2022, VSP_2022, BT_2022, VS_2019, VSP_2019, BT_2019, VS_2017, BT_2017, VS_2015 };

	for(int version = VS_2022; version <= VS_2015; version++) {
		for(int x64 = 1; x64 >= 0; x64--) {
			BuildMethod bm;
			
			String x86method = decode(version, VS_2015, "MSVS15",
			                                   VS_2017, "MSVS17", BT_2017, "MSBT17",
			                                   VS_2019, "MSVS19", VSP_2019, "MSVSP19", BT_2019, "MSBT19",
			                                   VS_2022, "MSVS22", VSP_2022, "MSVSP22", BT_2022, "MSBT22",
			                                   "MSBT");
			String x64s = x64 ? "x64" : "";
			String method = x86method + x64s;
			String builder = decode(version, VS_2015, "MSC15",
			                                 VS_2017, "MSC17", BT_2017, "MSC17",
			                                 VS_2019, "MSC19", VSP_2019, "MSC19", BT_2019, "MSC19",
			                                 VS_2022, "MSC22", VSP_2022, "MSC22", BT_2022, "MSC22",
			                                 "MSC22"
			                 ) + ToUpper(x64s);
		
			String vc, bin, inc, lib, kit81;
		
			Vector<String> bins;
			Vector<String> incs;
			Vector<String> libs;

			if(version == VS_2015)
				vc = df.Get("/microsoft visual studio 14.0/vc", "bin/cl.exe;bin/lib.exe;bin/link.exe;bin/mspdb140.dll");
			else
				vc = df.Get(decode(version, BT_2017, "/microsoft visual studio/2017/buildtools/vc/tools/msvc",
				                            VS_2017, "/microsoft visual studio/2017/community/vc/tools/msvc",
				                            BT_2019, "/microsoft visual studio/2019/buildtools/vc/tools/msvc",
				                            VS_2019, "/microsoft visual studio/2019/community/vc/tools/msvc",
				                            VSP_2019, "/microsoft visual studio/2019/professional/vc/tools/msvc",
				                            BT_2022, "/microsoft visual studio/2022/buildtools/vc/tools/msvc",
				                            VS_2022, "/microsoft visual studio/2022/community/vc/tools/msvc",
				                            VSP_2022, "/microsoft visual studio/2022/professional/vc/tools/msvc",
				                            ""),
				            x64 ? "bin/hostx64/x64/cl.exe;bin/hostx64/x64/mspdb140.dll"
				                : "bin/hostx86/x86/cl.exe;bin/hostx86/x86/mspdb140.dll");

			bin = df.Get("/windows kits/10/bin", "x86/makecat.exe;x86/accevent.exe");
			inc = df.Get("/windows kits/10", "um/adhoc.h");
			lib = df.Get("/windows kits/10", "um/x86/kernel32.lib");
			
			bool ver17 = version != VS_2015;
	
			if(inc.GetCount() == 0 || lib.GetCount() == 0) // workaround for situation when 8.1 is present, but 10 just partially
				kit81 = df.Get("/windows kits/8.1", "include");
			
			LOG("=============");
			DUMP(method);
			DUMP(vc);
			DUMP(bin);
			DUMP(inc);
			DUMP(kit81);
			DUMP(lib);
			
			if(vc.GetCount() && bin.GetCount() && (inc.GetCount() && lib.GetCount() || kit81.GetCount())) {
				bins.At(0) = vc + (ver17 ? (x64 ? "/bin/hostx64/x64" : "/bin/hostx86/x86") : (x64 ? "/bin/amd64" : "/bin"));
				bins.At(1) = bin + (x64 ? "/x64" : "/x86");

				incs.At(0) = vc + "/include";
				int ii = 1;
				if(inc.GetCount()) {
					incs.At(ii++) = inc + "/um";
					incs.At(ii++) = inc + "/ucrt";
					incs.At(ii++) = inc + "/shared";
				}
				if(kit81.GetCount()) {
					incs.At(ii++) = kit81 + "/include/um";
					incs.At(ii++) = kit81 + "/include/ucrt";
					incs.At(ii++) = kit81 + "/include/shared";
				}
				
				libs.At(0) = vc + (ver17 ? (x64 ? "/lib/x64" : "/lib/x86") : (x64 ? "/lib/amd64" : "/lib"));
				ii = 1;
				if(lib.GetCount()) {
					libs.At(ii++) = lib + (x64 ? "/ucrt/x64" : "/ucrt/x86");
					libs.At(ii++) = lib + (x64 ? "/um/x64" : "/um/x86");
				}
				if(kit81.GetCount()) {
					libs.At(ii++) = kit81 + (x64 ? "/lib/winv6.3/um/x64" : "/lib/winv6.3/um/x86");
				}
							
				bm.Name = method;
				bm.Compiler = bins[0];
				bm.Sdk = bin;

				if (x64 == 0)
					bm.Arch = "x86";
				else if (x64 == 1)
					bm.Arch = "x64";
				
				for (int i = 0; i < incs.GetCount(); i++)
					bm.Include << NormalizePath(incs[i]);
				
				for (int i = 0; i < libs.GetCount(); i++)
					bm.Lib << NormalizePath(libs[i]);
				
				if (bm.Lib.GetCount()) {
					String t = bm.Compiler;
					
					while (true) {
						FindFile ff(AppendFileName(t, "\\Common7\\IDE\\mspdb*.dll"));
						if (ff) {
							bm.Tools = NormalizePath(AppendFileName(t, "\\Common7\\IDE"));
							break;
						}
						
						t = GetFileFolder(t);
						if (t.GetLength() <= 0)
							break;
					}
			
					bm.Compiler = NormalizePath(bm.Compiler);
					bm.Type = BuildMethod::btMSC;
					bm.Sdk = NormalizePath(bm.Sdk);
					
					methods.Add(bm);
					
					Cout() << "Found BM: " << bm.Name << "\n";
				}
			}
		}
	}
}

bool ExistProgram(String& bin, const char *dir, const char *file) {
	String win = NormalizePath(GetWindowsDirectory());
	if(FileExists(AppendFileName(win.Mid(0, 3) + dir, file))) {
		bin = win.Mid(0, 3) + dir;
		return true;
	}
	return false;
}

bool BuildMethod::DetectMSC7_1(Vector<BuildMethod>& methods) {
	String vs = GetWinRegString("ProductDir", "SOFTWARE\\Microsoft\\VisualStudio\\7.1\\Setup\\VC");
	Compiler = NormalizePathNN(
			Nvl(
				GetWinRegString("InstallLocation",
	                            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
	                            "{362882AE-E40A-4435-B214-6420634C401F}"),
	            vs)
	    );
	Name = "MSC7.1";
	Type = BuildMethod::btMSC;
	
	TestLib(methods, "x86");
	return true;
}

bool BuildMethod::DetectMSC8(Vector<BuildMethod>& methods) {
	String sdk8 = GetWinRegString("InstallationFolder",
                             "Software\\Microsoft\\Microsoft SDKs\\Windows\\v6.0",
                             HKEY_CURRENT_USER);
	String bin8;
	if(!IsNull(sdk8)) {
		sdk8 = NormalizePath(sdk8);
		Sdk = sdk8;
		Compiler = bin8 = sdk8;
	}
	else {
		Compiler = bin8 = NormalizePathNN(
			GetWinRegString("8.0", "SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7")
		);
		Sdk = NormalizePathNN(GetWinRegString("Install Dir", "SOFTWARE\\Microsoft\\MicrosoftSDK\\InstalledSDKs\\8F9E5EF3-A9A5-491B-A889-C58EFFECE8B3"));
	}
	bool create = !IsNull(Compiler);
	bool create64 = false;
	if(bin8.GetLength() && FileExists(AppendFileName(bin8, "VC\\Bin\\x64\\cl.exe"))) {
		create64 = true;
	}
	
	Name = "MSC8";
	Type = BuildMethod::btMSC;
		
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectMSC9(Vector<BuildMethod>& methods) {
	String sdk9 = NormalizePathNN(GetWinRegString("InstallationFolder",
	                                       "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.0",
	                                       HKEY_LOCAL_MACHINE));
	if(IsNull(sdk9))
		sdk9 = NormalizePathNN(GetWinRegString("InstallationFolder",
	                                           "Software\\Microsoft\\Microsoft SDKs\\Windows\\v6.1",
	                                           HKEY_CURRENT_USER));
	if(IsNull(sdk9))
		sdk9 = NormalizePathNN(GetWinRegString("InstallationFolder",
		                                       "Software\\Microsoft\\Microsoft SDKs\\Windows\\v6.0A",
		                                       HKEY_CURRENT_USER));
	String bin9;
	if(!IsNull(sdk9) && FileExists(AppendFileName(sdk9, "VC\\Bin\\cl.exe")))
		bin9 = sdk9;
	else
		ExistProgram(bin9, "Program Files (x86)\\Microsoft Visual Studio 9.0", "VC\\Bin\\cl.exe")
		|| ExistProgram(bin9, "Program Files\\Microsoft Visual Studio 9.0", "VC\\Bin\\cl.exe");
	Sdk = sdk9;
	Compiler = bin9;
	bool create = !IsNull(Compiler);
	bool create64 = false;
	String vc9_64 = AppendFileName(bin9, "VC\\Bin\\x64");
	if(!FileExists(AppendFileName(vc9_64, "cl.exe")))
		vc9_64 = AppendFileName(bin9, "VC\\Bin\\amd64");
	if(bin9.GetLength() && FileExists(AppendFileName(vc9_64, "cl.exe")))
		create64 = true;
	
	Name = "MSC9";
	Type = BuildMethod::btMSC;
		
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectMSC10(Vector<BuildMethod>& methods) {
	String sdk10 = NormalizePathNN(GetWinRegString("InstallationFolder",
	                                       "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.1",
	                                       HKEY_LOCAL_MACHINE));
	if (IsNull(sdk10))
		sdk10 = NormalizePathNN(GetWinRegString("InstallationFolder",
		                                        "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v7.0A",
		                                        HKEY_LOCAL_MACHINE));
	String bin10;
	if(!IsNull(sdk10) && FileExists(AppendFileName(sdk10, "VC\\Bin\\cl.exe")))
		bin10 = sdk10;
	else
		ExistProgram(bin10, "Program Files (x86)\\Microsoft Visual Studio 10.0", "VC\\Bin\\cl.exe")
		|| ExistProgram(bin10, "Program Files\\Microsoft Visual Studio 10.0", "VC\\Bin\\cl.exe");
	Sdk = sdk10;
	Compiler = bin10;
	bool create = !IsNull(Compiler);
	bool create64 = false;
	String vc10_64 = AppendFileName(bin10, "VC\\Bin\\x64");
	if(!FileExists(AppendFileName(vc10_64, "cl.exe")))
		vc10_64 = AppendFileName(bin10, "VC\\Bin\\amd64");
	if(bin10.GetLength() && FileExists(AppendFileName(vc10_64, "cl.exe")))
		create64 = true;
	
	Name = "MSC10";
	Type = BuildMethod::btMSC;
		
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectMSC11(Vector<BuildMethod>& methods) {
	String sdk11 = NormalizePathNN(GetWinRegString("InstallationFolder",
	                                       "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.0",
	                                       HKEY_LOCAL_MACHINE));
	String bin11;
	if(!IsNull(sdk11) && FileExists(AppendFileName(sdk11, "VC\\bin\\cl.exe")))
		bin11 = sdk11;
	else
		ExistProgram(bin11, "Program Files (x86)\\Microsoft Visual Studio 11.0", "VC\\bin\\cl.exe")
		|| ExistProgram(bin11, "Program Files\\Microsoft Visual Studio 11.0", "VC\\bin\\cl.exe");
	Sdk = sdk11;
	Compiler = bin11;
	bool create = !IsNull(Compiler);
	bool create64 = false;
	String vc11_64 = AppendFileName(bin11, "VC\\bin\\x64");
	if (!FileExists(AppendFileName(vc11_64, "cl.exe")))
		vc11_64 = AppendFileName(bin11, "VC\\bin\\x86_amd64");
	if (bin11.GetLength() && FileExists(AppendFileName(vc11_64, "cl.exe")))
		create64 = true;
	
	Name = "MSC11";
	Type = BuildMethod::btMSC;
		
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectMSC12(Vector<BuildMethod>& methods) {
	String sdk12 = NormalizePathNN(GetWinRegString("InstallationFolder",
	                                       "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v8.1",
	                                       HKEY_LOCAL_MACHINE));
	String bin12;
	if(!IsNull(sdk12) && FileExists(AppendFileName(sdk12, "VC\\bin\\cl.exe")))
		bin12 = sdk12;
	else
		ExistProgram(bin12, "Program Files (x86)\\Microsoft Visual Studio 12.0", "VC\\bin\\cl.exe")
		|| ExistProgram(bin12, "Program Files\\Microsoft Visual Studio 12.0", "VC\\bin\\cl.exe");
	Sdk = sdk12;
	Compiler = bin12;
	bool create = !IsNull(Compiler);
	bool create64 = false;
	String vc12_64 = AppendFileName(bin12, "VC\\bin\\x64");
	if(!FileExists(AppendFileName(vc12_64, "cl.exe")))
		vc12_64 = AppendFileName(bin12, "VC\\bin\\x86_amd64");
	if(bin12.GetLength() && FileExists(AppendFileName(vc12_64, "cl.exe")))
		create64 = true;
	
	Name = "MSC12";
	Type = BuildMethod::btMSC;
	
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectMSC14(Vector<BuildMethod>& methods) {
	String sdk12 = NormalizePathNN(GetWinRegString("KitsRoot10",
	                                       "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
	                                       HKEY_LOCAL_MACHINE));
	String bin12;
	if(!IsNull(sdk12) && FileExists(AppendFileName(sdk12, "VC\\bin\\cl.exe")))
		bin12 = sdk12;
	else
		ExistProgram(bin12, "Program Files (x86)\\Microsoft Visual Studio 14.0", "VC\\bin\\cl.exe")
		|| ExistProgram(bin12, "Program Files\\Microsoft Visual Studio 14.0", "VC\\bin\\cl.exe");
	Sdk = sdk12;
	Compiler = bin12;
	bool create = !IsNull(Compiler);
	bool create64 = false;
	String vc12_64 = AppendFileName(bin12, "VC\\bin\\x64");
	if(!FileExists(AppendFileName(vc12_64, "cl.exe")))
		vc12_64 = AppendFileName(bin12, "VC\\bin\\x86_amd64");
	if(bin12.GetLength() && FileExists(AppendFileName(vc12_64, "cl.exe")))
		create64 = true;
	
	Name = "MSC14";
	Type = BuildMethod::btMSC;
		
	if (create)
		TestLib(methods, "x86");
	if (create64)
		TestLib(methods, "x64");
	
	return true;
}

bool BuildMethod::DetectGCC(Vector<BuildMethod>& methods) {
	FindFile ff(GetCurrentDirectory() + "\\gcc\\*");
	
	while (ff) {
		if (ff.IsDirectory()) {
			String s = ff.GetName();
			if (s != ".." && s != ".") {
				String p = ff.GetPath();
				String cppExe = AppendFileName(p, "\\bin\\c++.exe");
				
				if (FileExists(cppExe)) {
					BuildMethod gcc;
					gcc.Name = s;
					gcc.Compiler = p;
					gcc.Sdk = p;
					gcc.Type = btGCC;
					
					String p = GetTempPath();
					String pc = AppendFileName(p, "A42.cpp");
					String po = AppendFileName(p, "A42.exe");
					 
					{
						FileOut f(pc);
						f << "#include <iostream>\n int main() {\n\tstd::cout << \"A42\";\n\treturn 0;\n }\n";
					}
					
					String t, tt;
					bool res = true;
					
					{
						String c;
						c << "SET PATH=%PATH%;";
						c << GetFileFolder(cppExe) << ";";
						c << " & ";
						c << cppExe + " " + pc + " -m32 -o " + po;
						
						String pp = "cmd.exe /C \"" + c + "\"";
						LocalProcess lp(pp);
						tt = "";
						while (lp.Read(t)) {
							if (t.GetCount())
								tt << t;
						}
						DUMP(tt);
						res = lp.GetExitCode() == 0;
						
						if (res) {
							gcc.Arch = "x86";
							methods.Add(gcc);
							Cout() << "Found BM: " << gcc.Name << "\n";
						}
					}
					
					{
						String c;
						c << "SET PATH=%PATH%;";
						c << GetFileFolder(cppExe) << ";";
						c << " & ";
						c << cppExe + " " + pc + " -m64 -o " + po;
						
						String pp = "cmd.exe /C \"" + c + "\"";
						LocalProcess lp(pp);
						tt = "";
						while (lp.Read(t)) {
							if (t.GetCount())
								tt << t;
						}
						DUMP(tt);
						res = lp.GetExitCode() == 0;
						
						if (res) {
							gcc.Arch = "x64";
							methods.Add(gcc);
							Cout() << "Found BM: " << gcc.Name << "\n";
						}
					}
				}
			}
		}
		
		ff.Next();
	}
	
	return true;
}

bool BuildMethod::DetectClang(Vector<BuildMethod>& methods) {
	String bin = GetExeDirFile("externaltools");

	if(!DirectoryExists(bin + "/clang"))
		return false;
	
	for(int x64 = 1; x64 >= 0; x64--) {
		String method = x64 ? "CLANG" : "CLANG";
		String clangFolder = bin + "/clang/";

		BuildMethod clang;
		
		clang.Path << NativePath(clangFolder + "bin/");
		clang.Path << NativePath(clangFolder + (x64 ? "/x86_64-w64-mingw32/bin/" : "/i686-w64-mingw32/bin/"));
		clang.Name = method;
		clang.Compiler = NativePath(clang.Path[0] + (x64 ? "c++" : "i686-w64-mingw32-c++"));
		clang.Sdk = clangFolder;
		clang.Type = btGCC;
		clang.Arch = x64 == 1 ? "x64" : "x86";
		
		methods.Add(clang);
		Cout() << "Found BM: " << clang.Name << "." << clang.Arch << "\n";
	}
	
	return true;
}
#endif

void BuildMethod::Get(Vector<BuildMethod>& methods, bool print) {
	methods.Clear();
	
#ifdef PLATFORM_WIN32
	// search for modern MSC
	DetectClang(methods);
	//DeepVSSearch(methods);
	//DetectGCC(methods);
#endif

#ifdef PLATFORM_POSIX
	String t, tt;
	LocalProcess lp("which c++");
	while (lp.Read(t)) {
		if (t.GetCount())
			tt << t;
	}
	bool res = lp.GetExitCode() == 0;
	
	String cppExe = TrimBoth(tt);
	
	if (res && cppExe.GetLength()) {
		String p = GetTempPath();
		String pc = p + "/A42.cpp";
		String po = p + "/A42";
		 
		{
			FileOut f(pc);
			f << "#include <iostream>\n int main() {\n\tstd::cout << \"A42\";\n\treturn 0;\n }\n";
		}
		
		BuildMethod gcc;
		gcc.Name = "GCC";
		gcc.Type = btGCC;
		gcc.Compiler = cppExe;
		
		{
			LocalProcess lp(cppExe + " " + pc + " -m64 -o " + po);
			tt = "";
			while (lp.Read(t)) {
				if (t.GetCount())
					tt << t;
			}
			res = lp.GetExitCode() == 0;
			
			if (res) {
				gcc.Arch = "x64";
				methods.Add(gcc);
			}
		}
		
		{
			LocalProcess lp(cppExe + " " + pc + " -m32 -o " + po);
			tt = "";
			while (lp.Read(t)) {
				if (t.GetCount())
					tt << t;
			}
			res = lp.GetExitCode() == 0;
			
			if (res) {
				gcc.Arch = "x86";
				methods.Add(gcc);
			}
		}
	}
#endif

	if (print) {
		for (int i = 0; i < methods.GetCount(); i++) {
			BuildMethod& m = methods[i];
			Cout() << m.Name << " " << m.Arch << ": " << m.Compiler << "\n";
		}
	}
}

bool BuildMethod::TestLib(Vector<BuildMethod>& methods, const String& arch) {
	if (Compiler.IsEmpty() || Sdk.IsEmpty())
		return false;
	
	Tools = AppendFileName(Compiler, "\\Common7\\IDE");
	String tt = AppendFileName(Tools, "\\mspdb*.dll");
	FindFile ff(tt);
	if (!ff)
		return false;
	
	Arch = arch;
	
	if (arch == "x86") {
		Vector<String> x86um;
		Vector<String> x86ucrt;
		
		Lib.Clear();
		
		if (FindFile(Sdk + "\\lib\\*.lib"))
			x86um.Add("\\lib\\");
		else if (FindFile(Sdk + "\\lib\\x86\\*.lib"))
			x86um.Add("\\lib\\x86\\");
		else if (FindFile(Sdk + "\\lib\\win8\\um\\x86\\*.lib"))
			x86um.Add("\\lib\\win8\\um\\x86\\");
		else if (FindFile(Sdk + "\\lib\\winv6.3\\um\\x86"))
			x86um.Add("\\lib\\winv6.3\\um\\x86\\");
		else {
			FindFile ff(Sdk + "\\lib\\*");
			
			while (ff) {
				if (ff.IsDirectory()) {
					String s = ff.GetName();
					if (s != ".." && s != ".") {
						String p = ff.GetPath();
						if (FileExists(p + "\\um\\x86\\User32.Lib")) {
							x86um.Add("\\lib\\" + ff.GetName() + "\\um\\x86\\");
						}
						if (FileExists(p + "\\ucrt\\x86\\*.lib"))
							x86ucrt.Add("\\lib\\" + ff.GetName() + "\\ucrt\\x86\\");
					}
				}
				
				ff.Next();
			}
		}
		
		if (x86um.GetCount() == 0)
			return false;
		for (int i = x86um.GetCount() - 1; i >= 0; i--) {
			Lib << Sdk + x86um[i];
		}
		for (int i = x86ucrt.GetCount() - 1; i >= 0; i--) {
			Lib << Sdk + x86ucrt[i];
		}
		if (FileExists(Compiler + "\\VC\\lib\\LIBCMT.lib"))
			Lib << Compiler + "\\VC\\lib\\";
		else
			Lib.Clear();
		
		String bc = Compiler;
		
		String newComp = AppendFileName(Compiler, "VC\\BIN\\");
		if (FileExists(AppendFileName(newComp, "cl.exe")))
			Compiler = newComp;
		
		if (!Lib.IsEmpty())
			methods.Add(*this);
		
		Compiler = bc;
	}
	
	
	if (arch == "x64") {
		Vector<String> x86um;
		Vector<String> x86ucrt;
		
		Lib.Clear();
		
		if (FindFile(Sdk + "\\lib\\*.lib"))
			x86um.Add("\\lib\\");
		else if (FindFile(Sdk + "\\lib\\x64\\*.lib"))
			x86um.Add("\\lib\\x64\\");
		else if (FindFile(Sdk + "\\lib\\win8\\um\\x64\\*.lib"))
			x86um.Add("\\lib\\win8\\um\\x64\\");
		else if (FindFile(Sdk + "\\lib\\winv6.3\\um\\x64"))
			x86um.Add("\\lib\\winv6.3\\um\\x64\\");
		else {
			FindFile ff(Sdk + "\\lib\\*");
			
			while (ff) {
				if (ff.IsDirectory()) {
					String s = ff.GetName();
					if (s != ".." && s != ".") {
						String p = ff.GetPath();
						if (FileExists(p + "\\um\\x64\\User32.Lib"))
							x86um.Add("\\lib\\" + ff.GetName() + "\\um\\x64\\");
						if (FileExists(p + "\\ucrt\\x64\\*.lib"))
							x86ucrt.Add("\\lib\\" + ff.GetName() + "\\ucrt\\x64\\");
					}
				}
				
				ff.Next();
			}
		}
		
		if (x86um.GetCount() == 0)
			return false;
		for (int i = x86um.GetCount() - 1; i >= 0; i--) {
			Lib << Sdk + x86um[i];
		}
		for (int i = x86ucrt.GetCount() - 1; i >= 0; i--) {
			Lib << Sdk + x86ucrt[i];
		}
		
		if (FileExists(Compiler + "\\VC\\lib\\amd64\\LIBCMT.lib"))
			Lib << Compiler + "\\VC\\lib\\amd64\\";
		else if (FileExists(Compiler + "\\VC\\lib\\LIBCMT.lib"))
			Lib << Compiler + "\\VC\\lib\\";
		else
			Lib.Clear();
		
		String bc = Compiler;
		
		String newComp = AppendFileName(Compiler, "VC\\BIN\\");
		if (FileExists(AppendFileName(newComp, "cl.exe")))
			Compiler = newComp;
		
		if (!Lib.IsEmpty())
			methods.Add(*this);
		
		Compiler = bc;
	}
	
	return true;
}
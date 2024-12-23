#include <z2crlib/BuildMethod.h>

String NormalizePathNN(const String& path) {
	return IsNull(path) ? path : NormalizePath(path);
}

bool ExistProgram(String& bin, const char *dir, const char *file) {
	String win = NormalizePath(GetWindowsDirectory());
	if(FileExists(AppendFileName(win.Mid(0, 3) + dir, file))) {
		bin = win.Mid(0, 3) + dir;
		return true;
	}
	return false;
}

#ifdef PLATFORM_WIN32

struct DirFinder {
	Vector<String> dir;
	bool hasSdk = false;

	String Get(const String& substring, const char *files);
	void   GatherDirs(Index<String>& path, const String& dir);
	
	void   AddVs(Index<String>& path, const String& version, const String& vspath);
	
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

	/*for(int i = 0; i < 3; i++) {
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
				//DUMP(value);
			}
			RegCloseKey(key);
		}
	}*/
	
	/*Array<FileSystemInfo::FileInfo> root = StdFileSystemInfo().Find(Null);
	for(int i = 0; i < root.GetCount(); i++) {
		if(root[i].root_style == FileSystemInfo::ROOT_FIXED) {
			int drive = *root[i].filename;
			String pf = GetProgramsFolderX86();
			pf.Set(0, drive);
			pf = AppendFileName(pf, "Microsoft Visual Studio");
			if(DirectoryExists(pf))
				GatherDirs(path, pf);
		}
	}*/
	
	String sInstall = "installationPath:";
	String sName = "displayName:";
	
	String vsname;
	String vspath;
	
	Array<FileSystemInfo::FileInfo> root = StdFileSystemInfo().Find(Null);
	for (int i = 0; i < root.GetCount(); i++) {
		if(root[i].root_style == FileSystemInfo::ROOT_FIXED) {
			int drive = *root[i].filename;
			String pf = GetProgramsFolderX86();
			pf.Set(0, drive);
			pf = AppendFileName(pf, "Microsoft Visual Studio\\Installer");
			pf = AppendFileName(pf, "vswhere.exe");
			if (FileExists(pf)) {
				String t, tt;
				LocalProcess lp(pf);
				while (lp.Read(t)) {
					if (t.GetCount())
						tt << t;
				}
				
				Vector<String> lines = Split(tt, '\n', false);
				
				for (int j = 0; j < lines.GetCount(); j++) {
					String line = TrimBoth(lines[j]);
					//DUMP(line);
					
					if (line.StartsWith(sInstall)) {
						String rest = TrimBoth(line.Mid(sInstall.GetCount()));
						DUMP(rest);
						vspath = rest;
					}
					else if (line.StartsWith(sName)) {
						String rest = TrimBoth(line.Mid(sName.GetCount()));
						DUMP(rest);
						vsname = rest;
					}
					else if (line.GetCount() == 0) {
						if (vsname.GetCount() && vspath.GetCount() && DirectoryExists(vspath)) {
							AddVs(path, vsname, vspath);
							vsname = "";
							vspath = "";
						}
					}
				}
				
				if (vsname.GetCount() && vspath.GetCount() && DirectoryExists(vspath)) {
					AddVs(path, vsname, vspath);
					vsname = "";
					vspath = "";
				}
			}
		}
	}
	
	//DUMP(path);

	for(String s : path) {
		s = ToLower(s);
		s.Replace("\\", "/");
		while(s.TrimEnd("/"));
		dir.Add(s);
	}
}

void DirFinder::AddVs(Index<String>& path, const String& version, const String& vspath) {
	GatherDirs(path, vspath);
	
	if (hasSdk)
		return;
	
	String sdk = AppendFileName(vspath, "Common7\\Tools\\vsdevcmd\\core\\winsdk.bat");
	
	if (FileExists(sdk)) {
		String t, tt;
		String cmd = String("cmd.exe /C \"") + "detect.bat" + "\"";
		DUMP(cmd);
		LocalProcess lp(cmd);
		while (lp.Read(t)) {
			if (t.GetCount())
				tt << t;
		}
		
		Vector<String> lines = Split(tt, '\n', false);
		String sdkString = "WindowsSdkDir=";
		
		for (int j = 0; j < lines.GetCount(); j++) {
			String line = TrimBoth(lines[j]);
			
			if (line.StartsWith(sdkString)) {
				String rest = TrimBoth(line.Mid(sdkString.GetCount()));
				if (DirectoryExists(rest)) {
					GatherDirs(path, rest);
					hasSdk = true;
				}
			}
		}
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
			String method = x86method/* + x64s*/;
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
					
					Cout() << "Found BM: " << bm.Name << "." << bm.Arch << "\n";
				}
			}
		}
	}
}

bool BuildMethod::DetectGCC(Vector<BuildMethod>& methods) {
	String bin = GetExeDirFile("externaltools");
	String gccFolder = NativePath(bin + "/gcc/");

	if(!DirectoryExists(gccFolder))
		return false;
	
	FindFile ff(gccFolder + "*");
	
	while (ff) {
		if (ff.IsDirectory()) {
			String s = ff.GetName();
			if (s != ".." && s != ".") {
				String p = ff.GetPath();
				String cppExe = AppendFileName(p, "\\bin\\c++.exe");
				
				if (FileExists(cppExe)) {
					for(int x64 = 1; x64 >= 0; x64--) {
						BuildMethod gcc;
						gcc.Name = s;
						gcc.Compiler = cppExe;
						gcc.Sdk = p;
						gcc.Type = btGCC;
						gcc.Arch = x64 == 1 ? "x64" : "x86";
						gcc.CppVersion = 2017;
						
						String p = GetTempPath();
						String pc = AppendFileName(p, "A42.cpp");
						String po = AppendFileName(p, "A42.exe");
						 
						{
							FileOut f(pc);
							f << "#include <iostream>\n namespace test::cpp::seventeen {} int main() {\n\tstd::cout << \"A42\";\n\treturn 0;\n }\n";
						}
						
						String t, tt;
						bool res = true;
						
						{
							String c;
							c << "SET PATH=";
							c << GetFileFolder(cppExe) << ";%PATH%;";
							c << " && ";
							c << cppExe + " " + pc + " -m";
							if (gcc.Arch == "x86")
								c << "32";
							else
								c << "64";
							c << " -o " + po;
							
							String pp = "cmd.exe /C \"" + c + "\"";
							DUMP(pp);
							LocalProcess lp(pp);
							tt = "";
							while (lp.Read(t)) {
								if (t.GetCount())
									tt << t;
							}
							DUMP(tt);
							res = lp.GetExitCode() == 0;
							
							if (res) {
								methods.Add(gcc);
								Cout() << "Found BM: " << gcc.Name << "." << gcc.Arch << "\n";
								
								continue;
							}
						}
						
						{
							FileOut f(pc);
							f << "#include <iostream>\n int main() {\n\tstd::cout << \"A42\";\n\treturn 0;\n }\n";
						}
						
						{
							String c;
							c << "SET PATH=%PATH%;";
							c << GetFileFolder(cppExe) << ";";
							c << " & ";
							c << cppExe + " " + pc + " -m";
							if (gcc.Arch == "x86")
								c << "32";
							else
								c << "64";
							c << " -o " + po;
							
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
								gcc.CppVersion = 2011;
								methods.Add(gcc);
								Cout() << "Found BM: " << gcc.Name << "." << gcc.Arch << "\n";
							}
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
	String clangFolder = NativePath(bin + "/clang/");

	if(!DirectoryExists(clangFolder))
		return false;
	
	for(int x64 = 1; x64 >= 0; x64--) {
		String method = x64 ? "CLANG" : "CLANG";
		BuildMethod clang;
		
		clang.Path << NativePath(clangFolder + "bin/");
		clang.Path << NativePath(clangFolder + (x64 ? "/x86_64-w64-mingw32/bin/" : "/i686-w64-mingw32/bin/"));
		clang.Name = method;
		clang.Compiler = NativePath(clang.Path[0] + (x64 ? "c++" : "i686-w64-mingw32-c++"));
		clang.Sdk = clangFolder;
		clang.Type = btGCC;
		clang.Arch = x64 == 1 ? "x64" : "x86";
		
		String p = GetTempPath();
		String pc = p + "/A42.cpp";
		String po = p + "/A42";
		 
		{
			FileOut f(pc);
			f << "#include <iostream>\n int main() {\n\tstd::cout << \"A42\";\n\treturn 0;\n }\n";
		}
		
		String cppExe = clang.Compiler;
		String command = cppExe + " " + pc + " -m";
		if (clang.Arch == "x86")
			command << "32";
		else
			command << "64";
		command << " -o " << po;
		
		{
			LocalProcess lp(command);
			String tt = "";
			String t;
			while (lp.Read(t)) {
				if (t.GetCount())
					tt << t;
			}
			bool res = lp.GetExitCode() == 0;
			
			if (res) {
				methods.Add(clang);
				Cout() << "Found BM: " << clang.Name << "." << clang.Arch << "\n";
			}
		}
	}
	
	return true;
}
#endif

void BuildMethod::NewVs() {
	DirFinder dir;
	
	Vector<BuildMethod> methods;
	
	DeepVSSearch(methods);
}

void BuildMethod::Get(Vector<BuildMethod>& methods, bool print) {
	methods.Clear();
	
#ifdef PLATFORM_WIN32
	// search for modern MSC
	DetectClang(methods);
	DetectGCC(methods);
	DeepVSSearch(methods);
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

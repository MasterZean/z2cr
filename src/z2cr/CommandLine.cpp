#include <z2cr/CommandLine.h>
#include <z2crlib/ZCompiler.h>

CommandLine::CommandLine() {
#ifdef PLATFORM_POSIX
	ErrorColor = ErrorColorType::Ansi;
#elif PLATFORM_WIN32
	ErrorColor = ErrorColorType::Win32;
#else
	ErrorColor = ErrorColorType::None;
#endif
}

bool CommandLine::Read() {
	O = " -O2";
	const Vector<String>& commands = Upp::CommandLine();
	String name = ZCompiler::GetName();
	
	int i = 0;
	
	while (i < commands.GetCount()) {
		if (commands[i] == "-cc")
			CC = true;
		else if (commands[i] == "-ff")
			FF = true;
		else if (commands[i] == "-scu") {
			CPP = true;
			SCU = true;
		}
		else if (commands[i] == "-O1")
			O = " -O1";
		else if (commands[i] == "-O2")
			O = " -O2";
		else if (commands[i] == "-Od")
			O = " -Od";
		else if (commands[i] == "-lib")
			LIB = true;
		else if (commands[i] == "-lazy")
			LAZY = true;
		else if (commands[i] == "-c++") {
			CPP = true;
			SCU = false;
		}
		else if (commands[i] == "-out") {
			i++;
			if (i < commands.GetCount()) {
				OutPath = commands[i];
			}
			else {
				Cout() << name << " requires that '-scu', '-c++', '-vasm', '-out' parameters be followed by a path" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-bm") {
			i++;
			BM = true;
			if (i < commands.GetCount()) {
				BMName = commands[i];
			}
			else {
				Cout() << name << " requires that '-bm' parameter be followed by a valid build method" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-arch") {
			i++;
			ARCH = true;
			if (i < commands.GetCount()) {
				ARCHName = commands[i];
			}
			else {
				Cout() << name << " requires that '-arch' parameter be followed by a valid build architecture" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-pak") {
			i++;
			if (i < commands.GetCount()) {
				Packages.FindAdd(NormalizePath(commands[i]));
			}
			else {
				Cout() << name << " requires that '-pak' parameter be followed by a valid package path" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-main") {
			i++;
			if (i < commands.GetCount())
				EntryClass = commands[i];
			else {
				Cout() << name << " requires that '-main' parameter be followed by a valid symbol identifier" << '\n';
				return false;
			}
				
		}
		else if (commands[i] == "-mainfile") {
			i++;
			if (i < commands.GetCount()) {
				EntryFile = NormalizePath(commands[i]);
				if (FileExists(EntryFile))
					AddInputFile(EntryFile);
				else
					Cout() << "Could not open file: \n\t" << EntryFile << "\n";
			}
			else {
				Cout() << name << " requires that '-mainfile' parameter be followed by a valid package path" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-acp") {
			i++;
			if (i + 1 < commands.GetCount()) {
				ACP.y = StrInt(commands[i]);
				ACP.x = StrInt(commands[i + 1]);
				i++;
			}
			else {
				Cout() << name << " requires that '-acp' be followed by two coordinates" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-pp:no-path") {
			PP_NOPATH = true;
		}
		else if (commands[i] == "-ut:exe") {
			UTEXE = true;
		}
		else if (commands[i] == "-ut") {
			UT = true;
		}
		else if (commands[i] == "-color") {
			i++;
			if (i < commands.GetCount()) {
				String color = ToLower(commands[i]);
				
				if (color == "none")
					ErrorColor = ErrorColorType::None;
				else if (color == "win32") {
				#ifdef PLATFORM_POSIX
					Cout() << "option: -color win32: not supported on POSIX systems: ignoring" << '\n';
				#endif
					ErrorColor = ErrorColorType::Win32;
				}
				else if (color == "ansi")
					ErrorColor = ErrorColorType::Ansi;
				else if (color == "qtf")
					ErrorColor = ErrorColorType::Qtf;
				else
					Cout() << "option: -color: found invalid paramameter: " << color << ": ingoring" << '\n';
			}
			else {
				Cout() << name << " requires that '-color' parameter be followed by one of the following: 'none', 'win32', 'ansi'" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-subsystem") {
			i++;
			if (i < commands.GetCount()) {
				if (i < commands.GetCount()) {
					String ss = ToLower(commands[i]);
					
					if (ss == "console")
						SUBSYSTEM = 0;
					else if (ss == "windows")
						SUBSYSTEM = 1;
					else {
						Cout() << "option: -color: found invalid paramameter: " << ss << ": exiting" << '\n';
						return false;
					}
				}
			}
			else {
				Cout() << name << " requires that '-subsystem' parameter be followed by one of the following: 'console', 'windows'" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-noconsoletest") {
			NoConsoleTest = true;
		}
		else {
			String path = NormalizePath(commands[i]);
			if (FileExists(path))
				AddInputFile(path);
			else {
				Cout() << "Unknown parameter '" << commands[i] << "' or invalid file path. Exiting!\n";
				return false;
			}
		}
		i++;
	}
	
	return true;
}

void CommandLine::AddInputFile(const String& path) {
	Files.Add(path);
	if (OutPath.GetCount() == 0) {
		String out = GetFileDirectory(path) + GetFileTitle(path);
	#ifdef PLATFORM_WIN32
		out << ".exe";
	#endif
		OutPath = out;
	}
}


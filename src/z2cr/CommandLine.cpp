#include "z2cr.h"

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
			/*i++;
			if (i < commands.GetCount()) {
				Path = commands[i];
				OutPath = GetFileDirectory(Path) + GetFileTitle(Path) + ".cpp";
			}
			else {
				Cout() << Compiler::GetName() << " requires that '-scu', '-c++', '-vasm' parameters be followed by a path" << '\n';
				return false;
			}*/
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
			/*i++;
			if (i < commands.GetCount()) {
				Path = commands[i];
				OutPath = GetFileDirectory(Path) + GetFileTitle(Path) + ".cpp";
			}
			else {
				Cout() << Compiler::GetName() << " requires that '-scu', '-c++', '-vasm', '-out' parameters be followed by a path" << '\n';
				return false;
			}*/
		}
		else if (commands[i] == "-i")
			INT = true;
		else if (commands[i] == "-vasm") {
			VASM = true;
			i++;
			if (i < commands.GetCount()) {
				Path = commands[i];
				OutPath = GetFileDirectory(Path) + GetFileTitle(Path) + ".vasm";
			}
			else {
				Cout() << name << " requires that '-scu', '-c++', '-vasm', '-out' parameters be followed by a path" << '\n';
				return false;
			}
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
			if (i < commands.GetCount()) {
				ARCH = commands[i];
			}
			else {
				Cout() << name << " requires that '-arch' parameter be followed by a valid build architecture" << '\n';
				return false;
			}
		}
		else if (commands[i] == "-class") {
			i++;
			if (i < commands.GetCount()) {
				Class = commands[i];
			}
			else {
				Cout() << name << " requires that '-class' parameter be followed by a valid class name" << '\n';
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
		else if (commands[i] == "-file") {
			i++;
			if (i < commands.GetCount()) {
				Path = commands[i];
				OutPath = GetFileDirectory(Path) + GetFileTitle(Path) + ".cpp";
			}
			else {
				Cout() << name << " requires that '-file' parameter be followed by a valid file path" << '\n';
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
		else {
			Cout() << "Unknown parameter '" << commands[i] << "'. Exiting!\n";
			return false;
		}
		i++;
	}
	
	return true;
}
#ifndef __COMMAND_LINE_HPP__
#define __COMMAND_LINE_HPP__

#include <Core/Core.h>

using namespace Upp;

class CommandLine {
public:
	Index<String> Packages;
	Index<String> Files;
	String OutPath;
	String EntryFile;
	
	String O = " -O2";
	String BMName;
	String ARCHName;
	
	bool CC = false;
	bool FF = false;
	bool SCU = true;
	bool LIB = false;
	bool CPP = true;
	bool BM = false;
	bool ARCH = false;
	bool VASM = false;
	bool INT = false;
	bool LAZY = false;
	bool UT = false;
	
	bool PP_NOPATH = false;
	
	Point ACP = Point(-1, -1);
	
	String Class;
	
	bool Read();
};

#endif
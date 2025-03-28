#ifndef __COMMAND_LINE_HPP__
#define __COMMAND_LINE_HPP__

#include <z2crlib/common.h>

class CommandLine {
public:
	Index<String> Packages;
	Index<String> Files;
	String OutPath;
	String EntryFile;
	String EntryClass;
	
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
	bool LAZY = false;
	bool UT = false;
	int SUBSYSTEM = 0;
	ErrorColorType ErrorColor;
	bool NoConsoleTest = false;
	
	bool PP_NOPATH = false;
	
	Point ACP = Point(-1, -1);
	
	CommandLine();
	
	bool Read();
	
private:
	void AddInputFile(const String& path);
};

#endif
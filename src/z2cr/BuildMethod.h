#ifndef __BUILD_METHOD_HPP__
#define __BUILD_METHOD_HPP__

#include <Core/Core.h>

using namespace Upp;

struct BuildMethod: public Moveable<BuildMethod> {
	enum Type {
		btMSC,
		btGCC,
		btCLANG,
		btUnknown,
	};
	
	BuildMethod::Type Type;
	String Name;
	String Arch;
	String Compiler;
	String Tools;
	String Sdk;
	WithDeepCopy<Vector<String>> Lib;
	WithDeepCopy<Vector<String>> Include;
	
	void Xmlize(XmlIO& xml) {
		xml("name", Name);
		xml("arch", Arch);
		if (xml.IsLoading()) {
			String s;
			xml("type", s);
			if (ToUpper(s) == "GCC")
				Type = BuildMethod::btGCC;
			else if (ToUpper(s) == "MSC")
				Type = BuildMethod::btMSC;
			else
				Type = BuildMethod::btUnknown;
		}
		else {
			if (Type == BuildMethod::btGCC) {
				String s = "GCC";
				xml("type", s);
			}
			else if (Type == BuildMethod::btMSC) {
				String s = "MSC";
				xml("type", s);
			}
			else {
				String s = "unknown";
				xml("type", s);
			}
		}
		xml("compiler", Compiler);
		xml("tools", Tools);
		xml("sdk", Sdk);
		xml.List("include", "path", Include);
		xml.List("lib", "path", Lib);
	}
	
	static void Get(Vector<BuildMethod>& methods, bool print = false);
	
	static String Exe(const String& exe) {
	#ifdef PLATFORM_WIN32
		return exe + ".exe";
	#endif
	#ifdef PLATFORM_POSIX
		return exe;
	#endif
	}
	
	static int ErrorCode(int code) {
	#ifdef PLATFORM_WIN32
		return code;
	#endif
	#ifdef PLATFORM_POSIX
		byte c = (byte)code;
		if (c == 0)
			c = 1;
		
		return c;
	#endif
	}
	
	static int SuccessCode(int code) {
	#ifdef PLATFORM_WIN32
		return code;
	#endif
	#ifdef PLATFORM_POSIX
		return 0;
	#endif
	}
	
	static bool IsSuccessCode(int code) {
	#ifdef PLATFORM_WIN32
		return code >= 0;
	#endif
	#ifdef PLATFORM_POSIX
		return code == 0;
	#endif
	}
	
	bool TestLib(Vector<BuildMethod>& methods, const String& arch);
	
private:
#ifdef PLATFORM_WIN32
	static bool DetectGCC(Vector<BuildMethod>& methods);
	static bool DetectClang(Vector<BuildMethod>& methods);
	bool DetectMSC7_1(Vector<BuildMethod>& methods);
	bool DetectMSC8(Vector<BuildMethod>& methods);
	bool DetectMSC9(Vector<BuildMethod>& methods);
	bool DetectMSC10(Vector<BuildMethod>& methods);
	bool DetectMSC11(Vector<BuildMethod>& methods);
	bool DetectMSC12(Vector<BuildMethod>& methods);
	bool DetectMSC14(Vector<BuildMethod>& methods);
#endif
};


#endif
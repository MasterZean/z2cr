#ifndef __BUILDER_HPP__
#define __BUILDER_HPP__

#include <Core/Core.h>

using namespace Upp;

#include <z2crlib/BuildMethod.h>

class Builder {
public:
	Builder(const BuildMethod& abm): bm(abm) {
	}
	
	void ZCompilerPath(const String& azPath) {
		zPath = azPath;
	}
	
	void TargetRoot(const String& aroot) {
		target = aroot;
	}
	
	void Arch(const String& aarch) {
		arch = aarch;
	}
	
	void Optimize(const String& ao) {
		optimize = ao;
	}
	
	void Subsystem(int ss) {
		subsystem = ss;
	}
			
	bool Build(const String& path, const String& origPath, const Index<String>& libs);
	bool Compile(const String& src, const String& out, const String& inc = "");
	
private:
	BuildMethod bm;
	String zPath;
	String target;
	String cppPath;
	String linkPath;
	String arch;
	String env;
	String optimize;
	int subsystem;
	
	bool BuildMSC(const String& path, const String& origPath, const Index<String>& libs);
	bool BuildGCC(const String& path, const String& origPath, const Index<String>& libs);
	
	void DoPathsMSC();
	void DoEnvMSC();
	void DoEnvGCC();
	
	bool CompileMSC(const String& src, const String& out, const String& inc = "");
	bool CompileGCC(const String& src, const String& out, const String& inc = "");
};

#endif
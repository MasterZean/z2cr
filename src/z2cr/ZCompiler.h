#ifndef _z2cr_ZCompiler_h_
#define _z2cr_ZCompiler_h_

class ZCompiler {
public:
	ZCompiler(Assembly& aAss): ass(aAss) {
	}
	
	void Compile();
	
	static String& GetName();
	
private:
	Assembly& ass;
	
	void Compile(ZNamespace& ns);
};

#endif

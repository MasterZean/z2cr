#ifndef _z2cr_Transpiler_h_
#define _z2cr_Transpiler_h_

class ZTranspiler {
public:
	ZTranspiler(Assembly& aAss, Stream& aStream): ass(aAss), stream(aStream) {
	}
	
private:
	Assembly& ass;
	Stream& stream;
};

#endif

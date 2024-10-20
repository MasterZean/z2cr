#ifndef _z2cr_ZException_h_
#define _z2cr_ZException_h_

class ZException {
public:
	String Path;
	String Error;
	
	ZException(const String& path, const String& error): Path(path), Error(error) {
	}
	
	//void PrettyPrint(Context* con, Stream& stream);
};

#endif

#ifndef _z2cr_ZException_h_
#define _z2cr_ZException_h_

class ZException {
public:
	String Path;
	String Error;
	
	ZException(const String& error): Error(error) {
	}
	
	ZException(const String& path, const String& error): Path(path), Error(error) {
	}
	
	String ToString() {
		String msg;
		
		if (Path.GetCount())
			msg << Path << ": ";
		
		msg << Error;
		
		return msg;
	}
};

#endif

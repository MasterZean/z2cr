#ifndef _z2cr_ZParser_h_
#define _z2cr_ZParser_h_

class ZParser: public CParser {
public:
	enum NumberType {
		ntInvalid,
		ntInt,
		ntDWord,
		ntLong,
		ntQWord,
		ntFloat,
		ntDouble,
		ntPtrSize,
	};
	
	String Mode;
	
	ZParser(ZSource& aSource): source(aSource) {
		NestComments();
		Set(aSource.Content());
	}
			
	Point GetPoint() {
		return Point(GetLine(), GetPos().GetColumn());
	}
	
	String Identify();
	
	const ZSource& Source() const {
		return source;
	}
	
	bool IsZId();
	
	String ExpectId();
	String ExpectZId();
	String ExpectId(const String& id);

	void Expect(char ch);
	void Expect(char ch, char ch2);
	int ExpectNum();
	
	void ExpectEndStat();
	
	void Error(const Point& p, const String& text);
	
private:
	ZSource& source;
};
	
#endif

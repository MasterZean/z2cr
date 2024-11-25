#ifndef _z2cr_ZParser_h_
#define _z2cr_ZParser_h_

#include <z2crlib/entities.h>
#include <z2crlib/ZSource.h>

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
	
	ZParser(ZSourcePos& aPos): source(*aPos.Source) {
		NestComments();
		SetPos(aPos.Pos);
	}
			
	Point GetPoint() {
		return Point(GetLine(), GetPos().GetColumn());
	}
	
	String Identify();
	
	const ZSource& Source() const {
		return source;
	}
	
	bool IsZId();
	
	bool IsCharConst() {
		return term[0] == '\'';
	}
	
	String ExpectId();
	String ExpectZId();
	String ExpectId(const String& id);

	void Expect(char ch);
	void Expect(char ch, char ch2);
	int ExpectNum();
	
	void EatNewlines() {
	}
	
	void ExpectEndStat();
	
	void Error(const Point& p, const String& text);
	
	uint32 ReadChar();
	ZParser::NumberType ReadInt64(int64& oInt, double& oDub, int& base);
	
	ZSourcePos GetFullPos() {
		return ZSourcePos(source, GetPoint(), GetPos());
	}
	
	ZSource& Source() {
		return source;
	}
	
private:
	ZSource& source;
	
	uint64 ReadNumber64Core(Point& p, int base);
	ZParser::NumberType ReadI(Point& p, int sign, int64& oInt);
	ZParser::NumberType ReadF(Point& p, int sign, double& oDub);
};
	
#endif

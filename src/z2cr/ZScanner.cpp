#include "z2cr.h"

extern char SingleOp[24];
extern char DoubleOpCh1[9];
extern char DoubleOpCh2[9];

void ZScanner::Scan() {
	nmspace = &ass.DefaultNamespace();
	
	while (!parser.IsEof()) {
		Point p = parser.GetPoint();
		
		if (parser.Id("namespace"))
			ScanNamespace();
		else if (parser.Id("private")) {
			parser.Expect('{');
			
			while (!parser.IsChar('}')) {
				if (ScanDeclaration(AccessType::Private)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
			
			parser.Expect('}');
		}
		else if (parser.Id("protected")) {
			parser.Expect('{');
			
			while (!parser.IsChar('}')) {
				if (ScanDeclaration(AccessType::Protected)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
			
			parser.Expect('}');
		}
		else if (ScanDeclaration(AccessType::Public)) {
			// ALL GOOD
		}
		else {
			Point p = parser.GetPoint();
			parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
		}
	}
}

bool ZScanner::ScanDeclaration(AccessType accessType) {
	if (parser.IsChar2('@', '[')) {
		CParser::Pos tp = parser.GetPos();
		TraitLoop();
		Point p2 = parser.GetPoint();
		return ScanDeclarationLine(accessType, &tp);
	}
	
	return ScanDeclarationLine(accessType);

}

bool ZScanner::ScanDeclarationLine(AccessType accessType, CParser::Pos* tp) {
	if (parser.Id("def")) {
		Vector<ZFunction*> funcs;
		do {
			ZFunction& f = ScanFunc(accessType, false);
			if (tp)
				f.TraitPos = *tp;
			
			funcs.Add(&f);
		} while (parser.Char(','));
		
		CParser::Pos bp = parser.GetPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
		
		return true;
	}
	else if (parser.Id("func")) {
		Vector<ZFunction*> funcs;
		
		do {
			ZFunction& f = ScanFunc(accessType, true);
			if (tp)
				f.TraitPos = *tp;
			
			funcs.Add(&f);
		} while (parser.Char(','));
		
		CParser::Pos bp = parser.GetPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
		
		return true;
	}
	else if (parser.Id("val")) {
		ZSourcePos dp = parser.GetFullPos();
		
		String name = parser.ExpectId();
		
		ZVariable& f = nmspace->PrepareVariable(name);
		f.DefPos = dp;
		f.BackName = name;
		
		parser.Expect(':');
				
		parser.ExpectId();
		
		parser.ExpectEndStat();
		
		return true;
	}
	
	return false;
}

void ZScanner::ScanNamespace() {
	String name = parser.ExpectId();
	String fullName = name;
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	parser.ExpectEndStat();
	
	int index = ass.Namespaces.Find(fullName);
	
	nmspace = &ass.FindAddNamespace(fullName);
	
	/*if (index == -1)
		Cout() << "Found namespace: " << fullName << "\n";
	else
		Cout() << "Updated namespace: " << fullName << "\n";*/
}

ZFunction& ZScanner::ScanFunc(AccessType accessType, bool aFunc) {
	ASSERT(nmspace);
	
	ZSourcePos dp = parser.GetFullPos();
	
	String name;
	String bname;
	
	if (parser.Char('@')) {
		String s = parser.ExpectId();
		if (s == "size")
			parser.Error(dp.P, "'@size' can't be overlaoded");
		name = "@" + s;
		bname = "_" + s;
	}
	else {
		name = parser.ExpectId();
		bname = name;
	}
	
	CParser::Pos pp = parser.GetPos();
	parser.Expect('(');
	parser.Expect(')');
	
	ZFunction& f = nmspace->PrepareFunction(name);
	f.IsFunction = aFunc;
	f.DefPos = dp;
	f.ParamPos = pp;
	f.BackName = bname;
	
	/*Cout() << "\tFound: ";
	
	if (accessType == AccessType::Public)
		Cout() << "public ";
	else if (accessType == AccessType::Protected)
		Cout() << "protected ";
	else if (accessType == AccessType::Private)
		Cout() << "private ";
	
	if (aFunc)
		Cout() << "func ";
	else
		Cout() << "def ";
	
	Cout() << name << "()\n";*/
	
	return f;
}

void ZScanner::ScanBlock() {
	parser.Expect('{');
	parser.EatNewlines();
	
	while (!parser.IsChar('}')) {
		if (parser.Char('{')) {
			parser.EatNewlines();
		    ScanBlock();
		}
		else
			ScanToken();
	}
	
	parser.Expect('}');
	parser.EatNewlines();
}

void ZScanner::ScanToken() {
	if (parser.IsInt()) {
		int64 oInt;
		double oDub;
		int base;
		parser.ReadInt64(oInt, oDub, base);
	}
	else if (parser.IsString())
		parser.ReadString();
	else if (parser.IsId())
		parser.ReadId();
	else if (parser.IsCharConst())
		parser.ReadChar();
	else {
		for (int i = 0; i < 9; i++)
			if (parser.Char2(DoubleOpCh1[i], DoubleOpCh1[i]))
			    return;
		for (int i = 0; i < 24; i++)
			if (parser.Char(SingleOp[i]))
			    return;
		if (parser.Char('{') || parser.Char('}'))
			return;

		Point p = parser.GetPoint();
		parser.Error(p, "syntax error: " + parser.Identify() + " found");
	}
}

void ZScanner::InterpretTrait(const String& trait) {
	if (trait == "bindc")
		bindName = trait;
	else if (trait == "intrinsic")
		isIntrinsic = true;
	else if (trait == "dllimport")
		isDllImport = true;
	else if (trait == "stdcall")
		isStdCall = true;
	else if (trait == "cdecl")
		isCDecl = true;
	else if (trait == "nodoc")
		isNoDoc = true;
	else if (trait == "force")
		isForce = true;
}

void ZScanner::TraitLoop() {
	bindName = "";
	isIntrinsic = false;
	isDllImport = false;
	isStdCall = false;
	isCDecl = false;
	isNoDoc = false;
	isForce = false;
	
	if (parser.Char2('@', '[')) {
		
		String trait = parser.ExpectId();
		InterpretTrait(trait);
		
		while (!parser.IsChar(']')) {
			parser.Expect(',');
			
			trait = parser.ExpectId();
			InterpretTrait(trait);
		}
		
		parser.Expect(']');
	}
}
#include "z2cr.h"

void Scanner::Scan() {
	nmspace = &ass.DefaultNamespace();
	
	while (!parser.IsEof()) {
		Point p = parser.GetPoint();
		
		if (parser.Id("namespace"))
			ScanNamespace();
		else if (parser.IsChar2('@', '[')) {
			CParser::Pos tp = parser.GetPos();
			TraitLoop();
			Point p2 = parser.GetPoint();
			if (parser.Id("def")) {
				ZFunction& f = ScanFunc(false);
				f.TraitPos = tp;
			}
			else if (parser.Id("func")) {
				ZFunction& f = ScanFunc(true);
				f.TraitPos = tp;
			}
			else
				parser.Error(p2, "syntax error: declaration expected: "  + parser.Identify() + " found");
		}
		else if (parser.Id("def"))
			ScanFunc(false);
		else if (parser.Id("func"))
			ScanFunc(true);
		else {
			Point p = parser.GetPoint();
			parser.Error(p, "syntax error: " + parser.Identify() + " found");
		}
	}
}

void Scanner::ScanNamespace() {
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
	
	if (index == -1)
		Cout() << "Found namespace: " << fullName << "\n";
	else
		Cout() << "Updated namespace: " << fullName << "\n";
}

ZFunction& Scanner::ScanFunc(bool aFunc) {
	ASSERT(nmspace);
	
	Point p = parser.GetPoint();
	CParser::Pos dp = parser.GetPos();
	
	String name;
	String bname;
	
	if (parser.Char('@')) {
		String s = parser.ExpectId();
		if (s == "size")
			parser.Error(p, "'@size' can't be overlaoded");
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
	parser.ExpectEndStat();
	
	ZFunction& f = nmspace->PrepareFunction(name);
	f.IsFunction = aFunc;
	f.DefPos = dp;
	f.ParamPos = pp;
	f.BackName = bname;
	
	if (aFunc)
		Cout() << "\tFound func: " << name << "\n";
	else
		Cout() << "\tFound def: " << name << "\n";
	
	return f;
}

void Scanner::InterpretTrait(const String& trait) {
	Cout() << "\t\tFound trait: " << trait << "\n";
	
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

void Scanner::TraitLoop() {
	bindName = "";
	isIntrinsic = false;
	isDllImport = false;
	isStdCall = false;
	isCDecl = false;
	isNoDoc = false;
	isForce = false;
	
	if (parser.Char2('@', '[')) {
		Cout() << "\tBegin trait loop\n";
		
		String trait = parser.ExpectId();
		InterpretTrait(trait);
		
		while (!parser.IsChar(']')) {
			parser.Expect(',');
			
			trait = parser.ExpectId();
			InterpretTrait(trait);
		}
		
		parser.Expect(']');
	}
	
	Cout() << "\tEnd trait loop\n";
}
#include "z2cr.h"

extern char SingleOp[24];
extern char DoubleOpCh1[9];
extern char DoubleOpCh2[9];

void ZScanner::Scan() {
	nmspace = &ass.DefaultNamespace();
	
	while (!parser.IsEof()) {
		Point p = parser.GetPoint();
		
		if (parser.Id("namespace")) {
			ScanNamespace();
			
			if (parser.Char('{')) {
				bool back = inNamespaceBlock;
				inNamespaceBlock = true;
				
				while (!parser.IsChar('}'))
					ScanSingle();
				
				parser.Expect('}');
				
				inNamespaceBlock = back;
			}
			else
				parser.ExpectEndStat();
		}
		else
			ScanSingle();
	}
}

void ZScanner::ScanSingle() {
	if (parser.Id("using"))
		ScanUsing();
	else if (parser.Id("private")) {
		if (parser.Char('{')) {
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
		else if (ScanDeclaration(AccessType::Public)) {
			// ALL GOOD
		}
		else {
			Point p = parser.GetPoint();
			parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
		}
	}
	else if (parser.Id("protected")) {
		if (parser.Char('{')) {
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
	else if (parser.Id("namespace")) {
		ScanNamespace();
		
		if (parser.Char('{')) {
			bool back = inNamespaceBlock;
			inNamespaceBlock = true;
			
			while (!parser.IsChar('}'))
				ScanSingle();
			
			parser.Expect('}');
			
			inNamespaceBlock = back;
		}
		else
			parser.ExpectEndStat();
	}
	else if (ScanDeclaration(AccessType::Public)) {
		// ALL GOOD
	}
	else {
		Point p = parser.GetPoint();
		parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
	}
}

bool ZScanner::ScanDeclaration(AccessType accessType) {
	if (parser.IsChar2('@', '[')) {
		ZSourcePos tp = parser.GetFullPos();
		TraitLoop();
		return ScanDeclarationItem(accessType, &tp);
	}
	
	return ScanDeclarationItem(accessType);

}

bool ZScanner::ScanDeclarationItem(AccessType accessType, ZSourcePos* tp) {
	if (parser.Id("def")) {
		Vector<ZFunction*> funcs;
		do {
			ZFunction& f = ScanFunc(accessType, false);
			if (tp)
				f.TraitPos = *tp;
			
			funcs.Add(&f);
		} while (parser.Char(','));
		
		ZSourcePos bp = parser.GetFullPos();
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
		
		ZSourcePos bp = parser.GetFullPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
		
		return true;
	}
	else if (parser.Id("val")) {
		ZSourcePos dp = parser.GetFullPos();
		
		String name = parser.ExpectId();
		
		if (nmspace == &ass.DefaultNamespace())
			throw ER::ErrDeclOutsideNamespace(dp);
		
		ZVariable& f = nmspace->PrepareVariable(name);
		f.DefPos = dp;
		f.BackName = name;
		f.Section = section;
		
		if (parser.Char(':')) {
			ScanType();
			
			if (parser.Char('='))
				while (!(parser.IsChar(';')/* || (parser.GetSkipNewLines() == false && (parser.PeekChar() == '\n' || parser.PeekChar() == '\r'))*/)) {
					if (parser.IsChar('{'))
						ScanBlock();
					else
						ScanToken();
				}
						
		}
		else {
			if (!parser.IsChar('='))
				parser.Error(dp.P, "variable must have either an explicit type or be initialized");
			
			parser.Expect('=');
			
			while (!(parser.IsChar(';'))) {
				if (parser.IsChar('{'))
						ScanBlock();
					else
						ScanToken();
			}
		}
		
		parser.ExpectEndStat();
		
		return true;
	}
	
	return false;
}

void ZScanner::ScanType() {
	parser.ExpectId();
	while (parser.Char('.'))
		parser.ExpectId();
}

void ZScanner::ScanUsing() {
	parser.ExpectId("def");
	
	String name = parser.ExpectId();
	String fullName = name;
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	if (nmspace == nullptr) {
		usingNames << fullName;
	}
	else {
		ASSERT(section);
		section->UsingNames.FindAdd(fullName);
	}
	
	parser.ExpectEndStat();
}

void ZScanner::ScanNamespace() {
	ZSourcePos np = parser.GetFullPos();
	
	String name = parser.ExpectId();
	String fullName = name;
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	// TODO: delay throw Errors <<
	if (inNamespaceBlock)
		throw ER::ErrNestedNamespace(np);
	
	int index = ass.Namespaces.Find(fullName);
	
	nmspace = &ass.FindAddNamespace(fullName);
	
	section = &nmspace->Sections.Add();
	section->UsingNames = usingNames;
	
	namespaceCount++;
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
	
	ZSourcePos pp = parser.GetFullPos();
	parser.Expect('(');
	
	while (!parser.IsChar(')')) {
		parser.ExpectId();
		parser.Expect(':');
		
		ScanType();
		
		if (parser.Char(',')) {
			if (parser.IsChar(')'))
				parser.Error(parser.GetPoint(), "identifier expected, " + parser.Identify() + " found");
		}
	}
	
	parser.Expect(')');
	
	if (parser.Char(':'))
		ScanType();
	
	ZFunction& f = nmspace->PrepareFunction(name);
	f.IsFunction = aFunc;
	f.DefPos = dp;
	f.ParamPos = pp;
	f.BackName = bname;
	f.Section = section;
	
	return f;
}

void ZScanner::ScanBlock() {
	parser.Expect('{');
	parser.EatNewlines();
	
	while (!parser.IsChar('}')) {
		if (parser.IsChar('{')) {
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
